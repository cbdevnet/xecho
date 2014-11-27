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
	if(XAddConnectionWatch(res->display, xconn_watch, (void*)(&(res->xfds)))==0){
		fprintf(stderr, "Failed to register connection watch procedure\n");
		return false;
	}
	
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

bool x11_draw_blocks(CFG* config, XRESOURCES* xres, TEXTBLOCK** blocks){
	unsigned i;
	double current_size;
	XftFont* font=NULL;

	//draw all blocks
	for(i=0;blocks[i]&&blocks[i]->active;i++){
		//load font
		if(!font||(font&&current_size!=blocks[i]->size)){
			if(font){
				XftFontClose(xres->display, font);
			}
			font=XftFontOpen(xres->display, xres->screen,
					XFT_FAMILY, XftTypeString, config->font_name,
					XFT_PIXEL_SIZE, XftTypeDouble, blocks[i]->size,
					NULL
			);
			current_size=blocks[i]->size;
			if(!font){
				fprintf(stderr, "Failed to load block font (%s, %f)\n", config->font_name, current_size);
				return false;
			}
		}

		//draw text
		fprintf(stderr, "Drawing block %d (%s) at %d|%d size %f\n", i, blocks[i]->text, blocks[i]->x, blocks[i]->y, blocks[i]->size);
		XftDrawStringUtf8(xres->drawable, &(xres->text_color), font, blocks[i]->x, blocks[i]->y, (FcChar8*)blocks[i]->text, strlen(blocks[i]->text));
	}

	//clean up the mess
	if(font){
		XftFontClose(xres->display, font);
	}

	return true;
}

bool x11_recalculate_fonts(CFG* config, XRESOURCES* xres, TEXTBLOCK** blocks, unsigned width, unsigned height){
	unsigned i;
	XftFont* font=NULL;
	XGlyphInfo extents;

	unsigned widest_block=0, widest_block_width=0;

	//FIXME respect force_size, still do alignment passes

	//initialize calculation set
	for(i=0;blocks[i]&&blocks[i]->active;i++){
		blocks[i]->calculated=false;
		blocks[i]->y=20;
		blocks[i]->size=10.0;
	}

	//load font for initial tests
	font=XftFontOpen(xres->display, xres->screen,
			XFT_FAMILY, XftTypeString, config->font_name,
			XFT_PIXEL_SIZE, XftTypeDouble, 10.0,
			NULL
	);

	if(!font){
		fprintf(stderr, "Failed to open font %s for initial calculation\n", config->font_name);
		return false;
	}

	//find widest block, using that one for global width maximization
	//height is done by addition of all heights
	for(i=0;blocks[i]&&blocks[i]->active;i++){
		XftTextExtentsUtf8(xres->display, font, (FcChar8*)blocks[i]->text, strlen(blocks[i]->text), &extents);
		if(extents.xOff>widest_block_width){
			widest_block_width=extents.xOff;
			widest_block=i;
		}
	}

	XftFontClose(xres->display, font);

	fprintf(stderr, "Block %d (%s) is widest block with %d at pixelsize 10\n", widest_block, blocks[widest_block]->text, widest_block_width);

	//TODO maximize globally, respect alignment for xvalue
	//TODO if flag is set, optimize per block, respect alignment
	//TODO respect padding
	

	return true;
}
