bool xfd_add(X_FDS* set, int fd){
	unsigned i;

	if(!set->fds){
		set->fds=malloc(sizeof(int));
		if(!set->fds){
			fprintf(stderr, "xfd_add: Initial alloc failed\n");
			return false;
		}	
		set->size=1;
		set->fds[0]=fd;
		return true;
	}

	for(i=0;i<set->size;i++){
		if(set->fds[i]==fd){
			fprintf(stderr, "xfd_add: Not pushing duplicate entry\n");
			return false;
		}
	}

	set->fds=realloc(set->fds, (set->size+1)*sizeof(int));
	if(!set->fds){
		fprintf(stderr, "xfd_add: Failed to realloc fd set\n");
		return false;
	}

	set->fds[set->size]=fd;
	set->size++;

	return true;
}

bool xfd_remove(X_FDS* set, int fd){
	unsigned i, c;

	for(i=0;i<set->size;i++){
		if(set->fds[i]==fd){
			for(c=i;c<set->size-1;c++){
				set->fds[c]=set->fds[c+1];
			}

			set->size--;
			set->fds=realloc(set->fds, set->size*sizeof(int));
			if(!set->fds&&set->size>0){
				fprintf(stderr, "xfd_remove: Failed to realloc\n");
				return false;
			}
			return true;
		}
	}

	fprintf(stderr, "xfd_remove: FD not in set\n");
	return false;
}

void xfd_free(X_FDS* set){
	if(set->fds){
		free(set->fds);
		set->fds=NULL;
	}
	set->size=0;
}

void xconn_watch(Display* dpy, XPointer client_data, int fd, Bool opening, XPointer* watch_data){
	if(opening){
		fprintf(stderr, "xconn_watch: Internal connection registered\n");
		xfd_add((X_FDS*)client_data, fd);
	}
	else{
		fprintf(stderr, "xconn_watch: Internal connection closed\n");
		xfd_remove((X_FDS*)client_data, fd);
	}
}


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

	//get x socket fds
	if(!xfd_add(&(res->xfds), XConnectionNumber(res->display))){
		fprintf(stderr, "Failed to allocate xfd memory\n");
		return false;
	}
	XAddConnectionWatch(res->display, xconn_watch, (void*)(&(res->xfds)));
	
	return true;
}

void x11_cleanup(XRESOURCES* xres){
	if(!(xres->display)){
		return;
	}

	XftColorFree(xres->display, DefaultVisual(xres->display, xres->screen), DefaultColormap(xres->display, xres->screen), &(xres->text_color));
	XftColorFree(xres->display, DefaultVisual(xres->display, xres->screen), DefaultColormap(xres->display, xres->screen), &(xres->bg_color));
	//FIXME close window?
	if(xres->drawable){
		XftDrawDestroy(xres->drawable);
	}
	XCloseDisplay(xres->display);
	xfd_free(&(xres->xfds));
}
