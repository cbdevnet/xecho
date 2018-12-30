/*
This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar and 
reproduced below.

DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 
Version 2, December 2004 

Copyright (C) 2004 Sam Hocevar <sam@hocevar.net> 

	Everyone is permitted to copy and distribute verbatim or modified 
	copies of this license document, and changing it is allowed as long 
	as the name is changed. 

DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 
TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION 

	0. You just DO WHAT THE FUCK YOU WANT TO.
*/

#include <stdio.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <ctype.h>

//#include "xft_debug.c"

volatile bool xecho_shutdown=false;

/**
 *TODO: more debug stuff
 *
 */

struct {
	char* font_name;
	char* cs_back;
	char* cs_text;
} OPTIONS;

struct {
	XftDraw* xft_drawable;
	XftColor xft_text_color;
	XftColor xft_background_color;
	Display* display;
	Window main_window;
} RESOURCES;

void draw_string(char* text, char* font_name, int width, int height, int border, Display* display, XftDraw* drawable, XftColor c){
	XGlyphInfo extents={0,0,0,0,0,0};
	XftFont* font=NULL;
	double search_max=1, search_min=1, search_test;

	printf("Doing size calculation for window parameters %dx%d\n", width, height);

	do{
		search_min=search_max;
		search_max*=2;

		printf("Trying with size %f\n",search_max);
		//open font at size
		font=XftFontOpen(display, DefaultScreen(display), XFT_FAMILY, XftTypeString, font_name, XFT_SIZE, XftTypeDouble, search_max, NULL);

		if(!font){
			printf("Could not open font %s at size %f\n", font_name, search_max);
		}

		XftTextExtentsUtf8(display, font, text, strlen(text), &extents);
		XftFontClose(display, font);

		//reset size
		printf("Size %f spans %dx%d\n", search_max, extents.width, extents.height);
	}while(extents.width!=0&&extents.height!=0&&extents.width<width-border&&extents.height<height-border);

	if(extents.height==0||extents.width==0){
		printf("Failed to get extents\n");
		return;
	}

	printf("Doing binary search between sizes %f and %f\n", search_max, search_min);

	font=NULL;

	while(search_max>search_min){
		search_test=search_max-((search_max-search_min)/2);
		printf("Testing size %f\n", search_test);
		
		if(search_test-search_min<1){
			printf("Size found\n");
			break;
		}

		if(font){
			XftFontClose(display, font);
		}
		
		//open font at size
		font=XftFontOpen(display, DefaultScreen(display), XFT_FAMILY, XftTypeString, font_name, XFT_SIZE, XftTypeDouble, search_test, NULL);
		if(!font){
			printf("Could not open font %s at size %f\n", font_name, search_max);
		}

		XftTextExtents8(display, font, text, strlen(text), &extents);
		
		if(extents.width==0||extents.height==0){
			printf("Failed to get extents in binary search\n");
			return;
		}
		
		printf("Size %f spans %dx%d\n", search_test, extents.width, extents.height);

		if(extents.width<width-border&&extents.height<height-border){
			printf("Smaller than window\n");
			search_min=search_test;
		}
		else{
			printf("Bigger than window\n");
			search_max=search_test;
		}

	}

	printf("Window width: %d, Text width: %d, Space: %d, Align Delta: %d\n", width, extents.width, width-extents.width, (width-extents.width)/2);
	printf("Window height: %d, Text height: %d, Space: %d, Align Delta: %d\n", height, extents.height, width-extents.height, (width-extents.height)/2);
	printf("Font params: ascent %d, descent %d, height %d, max_advance_width %d\n", font->ascent, font->descent, font->height, font->max_advance_width);
	printf("Extents: width: %d, height: %d, x: %d, y: %d, yOff: %d, xOff: %d\n", extents.width, extents.height, extents.x, extents.y, extents.yOff, extents.xOff);
	printf("Drawing at (%d|%d)",0, ((height-extents.height)/2)+extents.height);

	XftColor color=c;
	XftDrawRect(drawable, &color, 0, /*((height-extents.height)/2)+extents.height*/0, extents.xOff, extents.height); 
	XftDrawString8(drawable, /*&color*/&(RESOURCES.xft_background_color), font, /*((width-extents.width)/2)*/extents.x, /*((height-extents.height)/2)+extents.height*//*extents.height*/extents.y/*height*/, text, strlen(text)); 
	XftFontClose(display, font);
}

unsigned short colorspec_read_byte(char* cs){
	char* hexmap="0123456789abcdef";
	unsigned short rv=0;
	int i;
	if(*cs!=0&&cs[1]!=0){
		for(i=0;hexmap[i]!=0&&hexmap[i]!=cs[0];i++){
		}
		rv|=(i<<12);
		for(i=0;hexmap[i]!=0&&hexmap[i]!=cs[1];i++){
		}
		rv|=(i<<8);
	}
	return rv;
}

XftColor colorspec_parse(char* cs, Display* display, int screen){
	XftColor rv;
	XRenderColor xrender_color={0,0,0,0xffff};
	int i;

	if(*cs=='#'){
		if(strlen(cs)!=7){
			printf("Invalid colorspec length\n");
		}

		for(i=1;i<strlen(cs);i++){
			if(!isxdigit(cs[i])){
				printf("Invalid digit in colorspec: %c\n", cs[i]);
				return rv;
			}
		}

		xrender_color.red=colorspec_read_byte(cs+1);
		xrender_color.green=colorspec_read_byte(cs+3);
		xrender_color.blue=colorspec_read_byte(cs+5);

		printf("Read colorspec %s as r:%04x g:%04x b:%04x\n", cs, xrender_color.red, xrender_color.green, xrender_color.blue);

		if(!XftColorAllocValue(display, DefaultVisual(display, screen), DefaultColormap(display, screen), &xrender_color, &rv)){
			printf("Failed to allocate color\n");
		}
	}
	else{
		if(!XftColorAllocName(display, DefaultVisual(display, screen), DefaultColormap(display, screen), cs, &rv)){
			printf("Failed to get color by name\n");
		}
	}
	return rv;
}


int main(int argc, char** argv){
	int i, text_size, width, height;
	bool read_stdin=false;
	int text_argument=1;
	char* current_text=NULL;

	int screen_no;
	Window root_window;
	XSetWindowAttributes window_attributes;
	XEvent event;

	OPTIONS.font_name="verdana";
	OPTIONS.cs_back="black";
	OPTIONS.cs_text="white";

	Atom wm_state_fullscreen;

	for(i=1;i<argc;i++){
		if(!strcmp(argv[i], "--stdin")){
			read_stdin=true;
		}
		else if(!strcmp(argv[i], "-font")){
			if(i+1<argc){
				OPTIONS.font_name=argv[i+1];
				i++;
				text_argument=i+1;
			}
			else{
				printf("No font specified\n");
			}
		}
		else if(!strcmp(argv[i], "-bc")){
			if(i+1<argc){
				OPTIONS.cs_back=argv[i+1];
				i++;
				text_argument=i+1;
			}
			else{
				printf("No background color argument\n");
			}
		}
		else if(!strcmp(argv[i], "-tc")){
			if(i+1<argc){
				OPTIONS.cs_text=argv[i+1];
				i++;
				text_argument=i+1;
			}
			else{
				printf("No text color argument\n");
			}
		}
		else if(!strcmp(argv[i], "--")){
			text_argument=i+1;
			break;
		}
		/*
		 * 	-align text-align
		 */
	}

	if(read_stdin){
		printf("Reading text from stdin\n");
	}
	else{
		text_size=0;
		for(i=text_argument;i<argc;i++){
			text_size+=strlen(argv[i])+1;
		}
		
		current_text=realloc(current_text, text_size*sizeof(char));
		
		text_size=0;
		for(i=text_argument;i<argc;i++){
			strncpy(current_text+text_size, argv[i], strlen(argv[i]));
			text_size+=strlen(argv[i])+1;
			current_text[text_size-1]=' ';
		}
		current_text[text_size-1]=0;

		printf("Text to print: \"%s\"\n", current_text);
	}

	RESOURCES.display=XOpenDisplay(NULL);

	if(!RESOURCES.display){
		printf("Failed to open display\n");
		return -1;
	}

	screen_no=DefaultScreen(RESOURCES.display);
	root_window=RootWindow(RESOURCES.display, screen_no);
	
	printf("X Server connected\n");
	
	//set up xft
	XftInit(NULL);

	printf("Parsing colorspecs\n");	
	RESOURCES.xft_text_color=colorspec_parse(OPTIONS.cs_text, RESOURCES.display, screen_no);
	RESOURCES.xft_background_color=colorspec_parse(OPTIONS.cs_back, RESOURCES.display, screen_no);
	printf("Done parsing\n");

	//create window
	window_attributes.background_pixel=RESOURCES.xft_background_color.pixel;
	//window_attributes.background_pixel=XBlackPixel(display, screen_no);
	window_attributes.cursor=None;
	window_attributes.event_mask=ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask;

	width=DisplayWidth(RESOURCES.display, screen_no);
	height=DisplayHeight(RESOURCES.display, screen_no);
	RESOURCES.main_window=XCreateWindow(RESOURCES.display, root_window, 0, 0, width, height, 0, CopyFromParent, InputOutput, CopyFromParent, CWBackPixel | CWCursor | CWEventMask, &window_attributes);	

	//set window properties
	//XSetWMProperties
	XSizeHints* size_hints=XAllocSizeHints();
	XWMHints* wm_hints=XAllocWMHints();
	XClassHint* class_hints=XAllocClassHint();

	if(!size_hints||!wm_hints||!class_hints){
		//FIXME should probably exit here
		printf("Failed to allocate hinting structures\n");
	}

	class_hints->res_name=argv[0];
	class_hints->res_class="xecho";

	//XSetWMProperties(RESOURCES.display, RESOURCES.main_window, "xecho", NULL, argv, argc, size_hints, wm_hints, class_hints);
	
	XFree(size_hints);
	XFree(wm_hints);
	XFree(class_hints);

	//set fullscreen mode
	wm_state_fullscreen=XInternAtom(RESOURCES.display, "_NET_WM_STATE_FULLSCREEN", True);
	XChangeProperty(RESOURCES.display, RESOURCES.main_window, XInternAtom(RESOURCES.display, "_NET_WM_STATE", True), XA_ATOM, 32, PropModeReplace, (unsigned char*) &wm_state_fullscreen, 1);

	//make xft drawable from window
	RESOURCES.xft_drawable=XftDrawCreate(RESOURCES.display, RESOURCES.main_window, DefaultVisual(RESOURCES.display, screen_no), DefaultColormap(RESOURCES.display, screen_no)); //FIXME visuals & colormaps?
	if(!RESOURCES.xft_drawable){
		printf("Xft: failed to allocate drawable\n");
	}

	//map window
	XMapRaised(RESOURCES.display, RESOURCES.main_window);

	while(!xecho_shutdown){
		//TODO listen to x events as well as stdin
		XNextEvent(RESOURCES.display,&event);
		switch(event.type){
			case ConfigureNotify:
				//TODO redraw
				width=event.xconfigure.width;
				height=event.xconfigure.height;
				XClearWindow(RESOURCES.display, RESOURCES.main_window);
				draw_string(current_text, OPTIONS.font_name, width, height, 20, RESOURCES.display, RESOURCES.xft_drawable, RESOURCES.xft_text_color);
				printf("Configure! (%dx%d)\n", width, height);
				break;

			case Expose:
				printf("Expose!\n");
				draw_string(current_text, OPTIONS.font_name, width, height, 20, RESOURCES.display, RESOURCES.xft_drawable, RESOURCES.xft_text_color);
				break;

			case KeyPress:
				printf("KeyPress!\n");
				break;

			case ButtonPress:
				free(current_text);
				XftColorFree(RESOURCES.display, DefaultVisual(RESOURCES.display, screen_no), DefaultColormap(RESOURCES.display, screen_no), &(RESOURCES.xft_text_color));
				XftDrawDestroy(RESOURCES.xft_drawable);
				XCloseDisplay(RESOURCES.display);
				exit(0);
				break;
		}
	}
	return 0;
}
