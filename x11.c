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
	res->debug_color=colorspec_parse(config->debug_color, res->display, res->screen);

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
	XftColorFree(xres->display, DefaultVisual(xres->display, xres->screen), DefaultColormap(xres->display, xres->screen), &(xres->debug_color));
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

	//early exit
	if(!blocks||!blocks[0]){
		return true;
	}

	//draw debug blocks if requested
	if(config->debug_boxes){
		for(i=0;blocks[i]&&blocks[i]->active;i++){
			 XftDrawRect(xres->drawable, &(xres->debug_color), blocks[i]->layout_x, blocks[i]->layout_y, blocks[i]->extents.width, blocks[i]->extents.height);
		}
	}

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
				fprintf(stderr, "Failed to load block font (%s, %d)\n", config->font_name, (int)current_size);
				return false;
			}
		}

		//draw text
		fprintf(stderr, "Drawing block %d (%s) at layoutcoords %d|%d size %d\n", i, blocks[i]->text, 
				blocks[i]->layout_x+blocks[i]->extents.x, 
				blocks[i]->layout_y+blocks[i]->extents.y, 
				(int)blocks[i]->size);

		XftDrawStringUtf8(xres->drawable, 
				&(xres->text_color), 
				font, 
				blocks[i]->layout_x+blocks[i]->extents.x, 
				blocks[i]->layout_y+blocks[i]->extents.y, 
				(FcChar8*)blocks[i]->text, 
				strlen(blocks[i]->text));
	}

	//clean up the mess
	if(font){
		XftFontClose(xres->display, font);
	}

	return true;
}

void x11_block_bounds(XRESOURCES* xres, TEXTBLOCK* block, XftFont* font){
	XftTextExtentsUtf8(xres->display, font, (FcChar8*)block->text, strlen(block->text), &(block->extents));
	fprintf(stderr, "Block \"%s\" extents: width %d, height %d, x %d, y %d, xOff %d, yOff %d\n",
			block->text, block->extents.width, block->extents.height, block->extents.x, block->extents.y,
			block->extents.xOff, block->extents.yOff);

	//block->width=extents.xOff;
	//block->height=extents.height;
	//block->x=extents.x;
	//block->y=extents.y;
}


bool x11_maximize_blocks(XRESOURCES* xres, TEXTBLOCK** blocks, unsigned width, unsigned height, char* font_name){
	unsigned i, bounding_width, bounding_height;
	XftFont* font=NULL;
	double current_size=1;
	bool done=false;
	bool scale_up=true;
	unsigned done_block;

	//FIXME this function is where most time is wasted.

	for(i=0;blocks[i]&&blocks[i]->active;i++){
	}

	//no blocks, bail out
	if(i<1){
		return true;
	}

	//find start size
	//sizes in sets to be maximized are always the same,
	//since any pass modifies all active blocks to the same size
	for(i=0;blocks[i]&&blocks[i]->active;i++){
		if(!(blocks[i]->calculated)){
			current_size=blocks[i]->size;
			break;
		}
	}

	do{
		//scale up until out of bounds
		fprintf(stderr, "Doing block maximization for %dx%d bounds with font %s at %d, directionality %s\n", width, height, font_name, (int)current_size, scale_up?"up":"down");
		
		//build font
		font=XftFontOpen(xres->display, xres->screen,
				XFT_FAMILY, XftTypeString, font_name,
				XFT_PIXEL_SIZE, XftTypeDouble, current_size,
				NULL
		);

		if(!font){
			fprintf(stderr, "Failed to allocate font %s at %d\n", font_name, (int)current_size);
			return false;
		}

		//calculate bounding boxes with current size for all nonfinished
		for(i=0;blocks[i]&&blocks[i]->active;i++){
			if(!blocks[i]->calculated){
				x11_block_bounds(xres, blocks[i], font);
				blocks[i]->size=current_size;
			}
		}

		XftFontClose(xres->display, font);
		
		//build bounding box
		bounding_width=0;
		bounding_height=0;
		for(i=0;blocks[i]&&blocks[i]->active;i++){
			bounding_height+=blocks[i]->extents.height;
			if(blocks[i]->extents.width>bounding_width){
				bounding_width=blocks[i]->extents.width;
			}
		}

		fprintf(stderr, "At size %d bounding box is %dx%d\n", (int)current_size, bounding_width, bounding_height);
		if(bounding_width>width||bounding_height>height){
			if(scale_up){
				fprintf(stderr, "Scaled out of window bounds, reversing\n");
				scale_up=false;
				current_size--;
			}
			else{
				fprintf(stderr, "Over window bounds, decreasing font size\n");
				current_size--;
			}
		}
		else{
			if(scale_up){
				fprintf(stderr, "Inside bounds, increasing font size\n");
				current_size++;
			}
			else{
				fprintf(stderr, "Size fixed at %d\n", (int)current_size);
				done=true;
			}
		}
	}while(!done);

	//set active to false for longest
	done_block=string_block_longest(blocks);
	blocks[done_block]->calculated=true;
	fprintf(stderr, "Marked block %d as done\n", done_block);

	return true;
}

bool x11_align_blocks(XRESOURCES* xres, CFG* config, TEXTBLOCK** blocks, unsigned width, unsigned height){
	//align blocks within bounding rectangle according to configured alignment
	unsigned i, total_height=0, current_height=0;

	for(i=0;blocks[i]&&blocks[i]->active;i++){
		total_height+=blocks[i]->extents.height;
	}

	//FIXME this might underflow in some cases
	for(i=0;blocks[i]&&blocks[i]->active;i++){
		//align x axis
		switch(config->alignment){
			case ALIGN_NORTH:
			case ALIGN_SOUTH:
			case ALIGN_CENTER:
				//centered
				blocks[i]->layout_x=(width-(blocks[i]->extents.width))/2;
				break;
			case ALIGN_NORTHWEST:
			case ALIGN_WEST:
			case ALIGN_SOUTHWEST:
				//left
				blocks[i]->layout_x=config->padding;
				break;
			case ALIGN_NORTHEAST:
			case ALIGN_EAST:
			case ALIGN_SOUTHEAST:
				//right
				blocks[i]->layout_x=width-(blocks[i]->extents.width)-config->padding;
				break;
		}

		//align y axis
		switch(config->alignment){
			case ALIGN_WEST:
			case ALIGN_EAST:
			case ALIGN_CENTER:
				//centered
				blocks[i]->layout_y=((height-total_height)/2)+current_height;
				current_height+=blocks[i]->extents.height;
				break;
			case ALIGN_NORTHWEST:
			case ALIGN_NORTH:
			case ALIGN_NORTHEAST:
				//top
				blocks[i]->layout_y=(config->padding)+current_height;
				current_height+=blocks[i]->extents.height;
				break;
			case ALIGN_SOUTHWEST:
			case ALIGN_SOUTH:
			case ALIGN_SOUTHEAST:
				//bottom
				blocks[i]->layout_y=height-total_height-(config->padding);
				total_height-=blocks[i]->extents.height;
				break;
		}
	}

	return true;
}

bool x11_recalculate_blocks(CFG* config, XRESOURCES* xres, TEXTBLOCK** blocks, unsigned width, unsigned height){
	unsigned i;
	XftFont* font=NULL;

	unsigned start_size;
	unsigned num_blocks=0;
	unsigned layout_width, layout_height;
	unsigned longest_length;

	//early exit.
	if(!blocks||!blocks[0]){
		return true;
	}

	//calculate layout volume
	if(width<config->padding){
		layout_width=width;
	}
	else{
		layout_width=width-config->padding;
	}
	if(height<config->padding){
		layout_height=height;
	}
	else{
		layout_height=height-config->padding;
	}

	//initialize calculation set
	for(i=0;blocks[i]&&blocks[i]->active;i++){
		blocks[i]->calculated=false;
		num_blocks++;
	}

	//guess initial font size
	longest_length=strlen(blocks[string_block_longest(blocks)]->text);
	if(longest_length<1){
		longest_length++;
	}
	start_size=fabs(layout_width/longest_length);
	fprintf(stderr, "Guessing initial size %d\n", start_size);

	if(config->force_size==0){
		//do binary search for match size
		blocks[0]->size=start_size;

		//do multiple passes if flag is set
		i=0;
		do{
			fprintf(stderr, "Running maximizer for pass %d (%d blocks)\n", i, num_blocks);
			if(!x11_maximize_blocks(xres, blocks, layout_width, layout_height, config->font_name)){
				return false;
			}
			i++;
		}
		while(config->independent_resize&&i<num_blocks);
	}
	else{
		//load font with forced size
		font=XftFontOpen(xres->display, xres->screen,
				XFT_FAMILY, XftTypeString, config->font_name,
				XFT_PIXEL_SIZE, XftTypeDouble, config->force_size,
				NULL
		);

		if(!font){
			fprintf(stderr, "Could not load font\n");
			return false;
		}

		//bounds calculation
		for(i=0;blocks[i]&&blocks[i]->active;i++){
			x11_block_bounds(xres, blocks[i], font);
			blocks[i]->size=config->force_size;
		}

		XftFontClose(xres->display, font);
	}
	
	//do alignment pass
	if(!x11_align_blocks(xres, config, blocks, width, height)){
		return false;
	}

	return true;
}
