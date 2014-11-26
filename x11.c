bool x11_init(XRESOURCES* res, CFG* config){
	Window root;
	XSetWindowAttributes window_attributes;
	unsigned width, height;
	Atom wm_state_fullscreen;

	//allocate some structures
	XSizeHints* size_hints=XAllocSizeHints();
	XWMHints* wm_hints=XAllocWMHints();
	XClassHint* class_hints=XAllocClassHint();

	if(!size_hints||!wm_hints||!class_hints){
		fprintf(stderr, "Failed to allocate X data structures\n");
		return false;
	}

	//x data initialization
	res->display=XOpenDisplay(NULL);

	if(!(res->display)){
		fprintf(stderr, "Failed to open display\n");
		XFree(size_hints);
		XFree(wm_hints);
		XFree(class_hints);
		return false;
	}
	
	res->screen=DefaultScreen(res->display);
	root=RootWindow(res->display, res->screen);

	//start xft
	if(!XftInit(NULL)){
		fprintf(stderr, "Failed to initialize Xft\n");
		XFree(size_hints);
		XFree(wm_hints);
		XFree(class_hints);
		return false;
	}

	//set up colors
	res->text_color=colorspec_parse(config->text_color, res->display, res->screen);
	res->bg_color=colorspec_parse(config->bg_color, res->display, res->screen);

	//set up window params
	window_attributes.background_pixel=res->bg_color.pixel;
	window_attributes.cursor=None;
	window_attributes.event_mask=ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask;
	width=DisplayWidth(res->display, res->screen);
	height=DisplayHeight(res->display, res->screen);

	//create window
	res->main=XCreateWindow(res->display, 
				root, 
				0, 
				0, 
				width, 
				height, 
				0, 
				CopyFromParent, 
				InputOutput, 
				CopyFromParent, 
				CWBackPixel | CWCursor | CWEventMask, 
				&window_attributes);

	//set window properties (TODO XSetWMProperties)
	class_hints->res_name="xecho-binary";
	class_hints->res_class="xecho";
	//XSetWMProperties(RESOURCES.display, RESOURCES.main_window, "xecho", NULL, argv, argc, size_hints, wm_hints, class_hints);
	XFree(size_hints);
	XFree(wm_hints);
	XFree(class_hints);
	
	//set fullscreen mode
	wm_state_fullscreen=XInternAtom(res->display, "_NET_WM_STATE_FULLSCREEN", False);
	XChangeProperty(res->display, res->main, XInternAtom(res->display, "_NET_WM_STATE", True), XA_ATOM, 32, PropModeReplace, (unsigned char*) &wm_state_fullscreen, 1);
	
	//make xft drawable from window
	//FIXME visuals & colormaps?
	res->drawable=XftDrawCreate(res->display, res->main, DefaultVisual(res->display, res->screen), DefaultColormap(res->display, res->screen)); 

	if(!res->drawable){
		fprintf(stderr, "Failed to allocate drawable\n");
		return false;
	}
	
	//map window
	XMapRaised(res->display, res->main);
	
	return true;
}

void x11_cleanup(XRESOURCES* xres){
	XftColorFree(xres->display, DefaultVisual(xres->display, xres->screen), DefaultColormap(xres->display, xres->screen), &(xres->text_color));
	XftColorFree(xres->display, DefaultVisual(xres->display, xres->screen), DefaultColormap(xres->display, xres->screen), &(xres->bg_color));
	//FIXME close window?
	XftDrawDestroy(xres->drawable);
	XCloseDisplay(xres->display);
}
