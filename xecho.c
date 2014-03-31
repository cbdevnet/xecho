
#include <stdio.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>

volatile bool xecho_shutdown=false;

int main(int argc, char** argv){
	int i;
	int text_argument=1;
	
	Display* display;
	int screen_no;
	Window root_window, main_window;
	XSetWindowAttributes window_attributes;
	XEvent event;

	Atom wm_state, wm_state_fullscreen;

	for(i=1;i<argc;i++){
		if(!strcmp(argv[i], "--stdin")){
			text_argument=-1;
			break;
		}
		else if(!strcmp(argv[i], "--")){
			text_argument=i+1;
			break;
		}
	}

	if(text_argument<0){
		printf("Reading text from stdin\n");
	}
	else{
		printf("Text to print: ");
		for(i=text_argument;i<argc;i++){
			printf("%s ", argv[i]);
		}
		printf("\n");
	}

	display=XOpenDisplay(NULL);

	if(!display){
		printf("Failed to open display\n");
		return -1;
	}

	wm_state_fullscreen=XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", True);
	wm_state=XInternAtom(display, "_NET_WM_STATE", True);

	screen_no=DefaultScreen(display);
	root_window=RootWindow(display, screen_no);
	
	window_attributes.background_pixel=XBlackPixel(display, screen_no);
	window_attributes.cursor=None;
	window_attributes.event_mask=ExposureMask | KeyPressMask | ButtonPressMask;

	main_window=XCreateWindow(display, root_window, 0, 0, DisplayWidth(display, screen_no), DisplayHeight(display, screen_no), 0, CopyFromParent, InputOutput, CopyFromParent, CWBackPixel | CWCursor | CWEventMask, &window_attributes);	
	
	//set fullscreen mode
	XChangeProperty(display, main_window, wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char*) &wm_state_fullscreen, 1);

	XMapRaised(display, main_window);

	while(!xecho_shutdown){
		//TODO listen to x events as well as stdin
		XNextEvent(display,&event);
		switch(event.type){
			case Expose:
				printf("Expose!\n");
				break;

			case ConfigureNotify:
				printf("Configure! (%dx%d)\n", event.xconfigure.width, event.xconfigure.height);
				break;

			case KeyPress:
				printf("KeyPress!\n");
				break;

			case ButtonPress:
				XCloseDisplay(display);
				exit(0);
				break;
		}
	}

}
