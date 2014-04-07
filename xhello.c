/*
Pretty minimal XWindow test program


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
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

int main(int argc, char** argc){
	printf("Xlib Hello World");

	Display* d;
	XEvent e;
	Window w,r;
	int screen;
	char* dpyName=getenv("DISPLAY");
	if(!dpyName){
		dpyName=":0";
	}

	d=XOpenDisplay(dpyName);
	if(!d){
		printf("No display");
		exit(1);
	}

	screen=DefaultScreen(d);
	r=RootWindow(d,screen);
	w=XCreateSimpleWindow(d, r, 0, 0, 100, 100, 2, BlackPixel(d, screen), WhitePixel(d, screen));

	XSelectInput(d, w, ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask);

	XMapWindow(d,w);

	while(1){
		XNextEvent(d,&e);
		switch(e.type){
			case Expose:
				printf("Expose!\n");
				break;

			case ConfigureNotify:
				printf("Configure! (%dx%d)\n", e.xconfigure.width, e.xconfigure.height);
				break;

			case KeyPress:
				printf("KeyPress!\n");
				break;

			case ButtonPress:
				XCloseDisplay(d);
				exit(0);
				break;
		}
	}
}
