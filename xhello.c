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
