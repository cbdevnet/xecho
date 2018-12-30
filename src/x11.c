bool x11_init(XRESOURCES* res, CFG* config){
	Window root;
	XSetWindowAttributes window_attributes;
	unsigned width, height;
	Atom wm_state_fullscreen;
	int xdbe_major, xdbe_minor;
	XTextProperty window_name;
	pid_t pid = getpid();

	//allocate some structures
	XSizeHints* size_hints = XAllocSizeHints();
	XWMHints* wm_hints = XAllocWMHints();
	XClassHint* class_hints = XAllocClassHint();

	if(!size_hints || !wm_hints || !class_hints){
		fprintf(stderr, "Failed to allocate X data structures\n");
		return false;
	}

	//x data initialization
	res->display = XOpenDisplay(NULL);

	if(!(res->display)){
		fprintf(stderr, "Failed to open display\n");
		XFree(size_hints);
		XFree(wm_hints);
		XFree(class_hints);
		return false;
	}

	if(config->double_buffer){
		config->double_buffer = (XdbeQueryExtension(res->display, &xdbe_major, &xdbe_minor) != 0);
	}
	else{
		config->double_buffer = false;
	}
	errlog(config, LOG_INFO, "Double buffering %s\n", config->double_buffer ? "enabled":"disabled");

	res->screen = DefaultScreen(res->display);
	root = RootWindow(res->display, res->screen);

	//start xft
	if(!XftInit(NULL)){
		fprintf(stderr, "Failed to initialize Xft\n");
		XFree(size_hints);
		XFree(wm_hints);
		XFree(class_hints);
		return false;
	}

	//set up colors
	res->text_color = colorspec_parse(config->text_color, res->display, res->screen);
	res->bg_color = colorspec_parse(config->bg_color, res->display, res->screen);
	res->debug_color = colorspec_parse(config->debug_color, res->display, res->screen);

	//set up window params
	window_attributes.background_pixel = res->bg_color.pixel;
	window_attributes.cursor = None;
	window_attributes.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask;
	width = DisplayWidth(res->display, res->screen);
	height = DisplayHeight(res->display, res->screen);

	//create window
	res->main = XCreateWindow(res->display,
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

	//set window properties
	if(XStringListToTextProperty(&(config->window_name), 1, &window_name) == 0){
		fprintf(stderr, "Failed to create string list, aborting\n");
		return false;
	}

	wm_hints->flags = 0;
	class_hints->res_name = "xecho";
	class_hints->res_class = "xecho";

	XSetWMProperties(res->display, res->main, &window_name, NULL, NULL, 0, NULL, wm_hints, class_hints);

	XFree(window_name.value);
	XFree(size_hints);
	XFree(wm_hints);
	XFree(class_hints);

	//set fullscreen mode
	if(!config->windowed){
		wm_state_fullscreen = XInternAtom(res->display, "_NET_WM_STATE_FULLSCREEN", False);
		XChangeProperty(res->display, res->main, XInternAtom(res->display, "_NET_WM_STATE", False), XA_ATOM, 32, PropModeReplace, (unsigned char*) &wm_state_fullscreen, 1);
	}

	XChangeProperty(res->display, res->main, XInternAtom(res->display, "_NET_WM_PID", False), XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&pid, 1);

	//allocate back drawing buffer
	if(config->double_buffer){
		res->back_buffer = XdbeAllocateBackBufferName(res->display, res->main, XdbeBackground);
	}

	//make xft drawable from window
	res->drawable = XftDrawCreate(res->display, (config->double_buffer?res->back_buffer:res->main), DefaultVisual(res->display, res->screen), DefaultColormap(res->display, res->screen));

	if(!res->drawable){
		fprintf(stderr, "Failed to allocate drawable\n");
		return false;
	}

	//register for WM_DELETE_WINDOW messages
	res->wm_delete = XInternAtom(res->display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(res->display, res->main, &(res->wm_delete), 1);

	//map window
	XMapRaised(res->display, res->main);

	//get x socket fds
	if(!xfd_add(&(res->xfds), XConnectionNumber(res->display))){
		fprintf(stderr, "Failed to allocate xfd memory\n");
		return false;
	}
	if(XAddConnectionWatch(res->display, xconn_watch, (void*)(&(res->xfds))) == 0){
		fprintf(stderr, "Failed to register connection watch procedure\n");
		return false;
	}

	return true;
}

void x11_cleanup(XRESOURCES* xres, CFG* config){
	if(!(xres->display)){
		return;
	}

	XftColorFree(xres->display, DefaultVisual(xres->display, xres->screen), DefaultColormap(xres->display, xres->screen), &(xres->text_color));
	XftColorFree(xres->display, DefaultVisual(xres->display, xres->screen), DefaultColormap(xres->display, xres->screen), &(xres->bg_color));
	XftColorFree(xres->display, DefaultVisual(xres->display, xres->screen), DefaultColormap(xres->display, xres->screen), &(xres->debug_color));
	if(xres->drawable){
		XftDrawDestroy(xres->drawable);
	}
	if(config->double_buffer){
		XdbeDeallocateBackBufferName(xres->display, xres->back_buffer);
	}
	XCloseDisplay(xres->display);
	xfd_free(&(xres->xfds));
}

bool x11_draw_blocks(CFG* config, XRESOURCES* xres, TEXTBLOCK** blocks){
	unsigned i;
	double current_size;
	XftFont* font = NULL;

	//early exit
	if(!blocks || !blocks[0]){
		return true;
	}

	//draw debug blocks if requested
	if(config->debug_boxes){
		for(i = 0; blocks[i] && blocks[i]->active; i++){
			 XftDrawRect(xres->drawable, &(xres->debug_color), blocks[i]->layout_x, blocks[i]->layout_y, blocks[i]->extents.width, blocks[i]->extents.height);
		}
	}

	//draw boxes only
	if(config->disable_text){
		return true;
	}

	//draw all blocks
	for(i = 0; blocks[i] && blocks[i]->active; i++){
		//load font
		if(!font || (font && current_size != blocks[i]->size)){
			if(font){
				XftFontClose(xres->display, font);
			}
			font = XftFontOpen(xres->display, xres->screen,
					XFT_FAMILY, XftTypeString, config->font_name,
					XFT_PIXEL_SIZE, XftTypeDouble, blocks[i]->size,
					NULL
			);
			current_size = blocks[i]->size;
			if(!font){
				fprintf(stderr, "Failed to load block font (%s, %d)\n", config->font_name, (int)current_size);
				return false;
			}
		}

		//draw text
		errlog(config, LOG_DEBUG, "Drawing block %d (%s) at layoutcoords %d|%d size %d\n", i, blocks[i]->text,
				blocks[i]->layout_x + blocks[i]->extents.x,
				blocks[i]->layout_y + blocks[i]->extents.y,
				(int)blocks[i]->size);

		XftDrawStringUtf8(xres->drawable,
				&(xres->text_color),
				font,
				blocks[i]->layout_x + blocks[i]->extents.x,
				blocks[i]->layout_y + blocks[i]->extents.y,
				(FcChar8*)blocks[i]->text,
				strlen(blocks[i]->text));
	}

	//clean up the mess
	if(font){
		XftFontClose(xres->display, font);
	}

	return true;
}

bool x11_blocks_resize(XRESOURCES* xres, CFG* config, TEXTBLOCK** blocks, XGlyphInfo* bounding_box, double size){
	XftFont* font = NULL;
	unsigned bounding_width = 0, bounding_height = 0;
	unsigned i;

	//load font with at supplied size
	font=XftFontOpen(xres->display, xres->screen,
			XFT_FAMILY, XftTypeString, config->font_name,
			XFT_PIXEL_SIZE, XftTypeDouble, size,
			NULL
	);

	if(!font){
		fprintf(stderr, "Could not load font\n");
		return false;
	}

	//fprintf(stderr, "Block \"%s\" extents: width %d, height %d, x %d, y %d, xOff %d, yOff %d\n",
	//		block->text, block->extents.width, block->extents.height, block->extents.x, block->extents.y,
	//		block->extents.xOff, block->extents.yOff);

	//bounds calculation
	for(i = 0; blocks[i] && blocks[i]->active; i++){
		//update only not yet calculated blocks
		if(!(blocks[i]->calculated)){
			XftTextExtentsUtf8(xres->display, font, (FcChar8*)blocks[i]->text, strlen(blocks[i]->text), &(blocks[i]->extents));
			errlog(config, LOG_DEBUG, "Recalculated block %d (%s) extents: %dx%d\n", i, blocks[i]->text, blocks[i]->extents.width, blocks[i]->extents.height);
			blocks[i]->size = size;
		}

		//calculate bounding box over all
		bounding_height += blocks[i]->extents.height;
		if(blocks[i]->extents.width > bounding_width){
			bounding_width = blocks[i]->extents.width;
		}
	}

	if(bounding_box){
		bounding_box->width = bounding_width;
		bounding_box->height = bounding_height;
	}

	XftFontClose(xres->display, font);
	return true;
}

bool x11_maximize_blocks(XRESOURCES* xres, CFG* config, TEXTBLOCK** blocks, unsigned width, unsigned height){
	unsigned i, num_blocks = 0;
	double current_size = 1;
	unsigned bound_low, bound_high, bound_delta;
	unsigned done_block, longest_block;
	XGlyphInfo bbox;
	bool break_loop = false;

	int bounds_delta = 4; //initial secondary bound delta

	//count blocks
	for(i = 0; blocks[i] && blocks[i]->active; i++){
		if(!blocks[i]->calculated){
			num_blocks++;
		}
	}

	//no blocks, bail out
	if(num_blocks < 1 || width < 1 || height < 1){
		errlog(config, LOG_DEBUG, "Maximizer bailing out, nothing to do\n");
		return true;
	}

	errlog(config, LOG_DEBUG, "Maximizer running for %dx%d bounds\n", width, height);

	//guess primary bound
	//sizes in sets to be maximized are always the same,
	//since any pass modifies all active blocks to the same size
	longest_block = string_block_longest(blocks);
	if(blocks[longest_block]->size == 0){
		if(config->max_size > 0){
			//use max size as primary bound
			current_size = config->max_size;
		}
		else{
			//educated guess
			current_size = roundf(width / ((strlen(blocks[longest_block]->text) > 0) ? strlen(blocks[longest_block]->text):1));
		}
	}
	else{
		//use last known size as primary bound
		current_size = blocks[longest_block]->size;
	}
	errlog(config, LOG_DEBUG, "Guessing primary bound %d\n", (int)current_size);

	//find secondary bound for binary search
	if(!x11_blocks_resize(xres, config, blocks, &bbox, current_size)){
		fprintf(stderr, "Failed to resize blocks to primary bound\n");
	}

	if(bbox.height > height || bbox.width > width){
		//primary bound is upper bound, search down
		bounds_delta *= -1;
	}
	errlog(config, LOG_DEBUG, "Primary bound is %s than bounding box\n", (bounds_delta < 0) ? "bigger":"smaller");

	do{
		bounds_delta *= 2;

		if(current_size + bounds_delta < 1){
			errlog(config, LOG_DEBUG, "Search went out of permissible range\n");
			bounds_delta = -current_size; //FIXME this might fail when the condition is met with an overflow
			break;
		}

		if(!x11_blocks_resize(xres, config, blocks, &bbox, current_size + bounds_delta)){
			fprintf(stderr, "Failed to resize blocks to size %d\n", (int)current_size + bounds_delta);
			return false;
		}

		if(bbox.width < 1 || bbox.height < 1){
			errlog(config, LOG_DEBUG, "Bounding box was empty\n");
			return true;
		}

		errlog(config, LOG_DEBUG, "With bounds_delta %d bounding box is %dx%d\n", bounds_delta, bbox.width, bbox.height);
	}
	//loop until direction needs to be reversed
	while(	((bounds_delta < 0) && (bbox.width > width || bbox.height > height)) //searching lower bound, break if within bounds
		|| ((bounds_delta > 0) && (bbox.width <= width && bbox.height <= height))); //searching upper bound, break if out of bounds
	errlog(config, LOG_DEBUG, "Calculated secondary bound %d via offset %d\n", (int)current_size + bounds_delta, bounds_delta);

	//prepare bounds for binary search
	if(bounds_delta < 0){
		bound_low = current_size + bounds_delta;
		bound_high = current_size; //cant optimize here if starting bound matches exactly
	}
	else{
		bound_high = current_size + bounds_delta;
		bound_low = current_size; //cant optimize here if starting bound matches exactly
	}

	if(config->max_size > 0 && bound_high > config->max_size){
		errlog(config, LOG_DEBUG, "Enforcing size constraint\n");
		bound_high = config->max_size;
	}
	if(config->max_size > 0 && bound_low > config->max_size){
		bound_low = 1;
	}

	//binary search for final size
	do{
		bound_delta = bound_high - bound_low;
		current_size = bound_low + ((double)bound_delta / (double)2);

		//stupid tiebreaker implementation
		if(bound_delta / 2 == 0){
			if(break_loop){
				current_size = bound_low;
				bound_delta = 0;
			}
			else{
				break_loop = true;
			}
		}

		errlog(config, LOG_DEBUG, "Binary search testing size %d, hi %d, lo %d, delta %d\n", (int)current_size, bound_high, bound_low, bound_delta);

		if(!x11_blocks_resize(xres, config, blocks, &bbox, current_size)){
			fprintf(stderr, "Failed to resize blocks to test size %d\n", (int)current_size);
			return false;
		}

		if(bbox.width < 1 || bbox.height < 1){
			errlog(config, LOG_DEBUG, "Bounding box is 0, bailing out\n");
			break;
		}

		if(bbox.width > width || bbox.height > height){
			//out of bounds
			bound_high = current_size;
			errlog(config, LOG_DEBUG, "-> OOB\n");
		}
		else{
			//inside bounds
			bound_low = current_size;
			errlog(config, LOG_DEBUG, "-> OK\n");
		}

	}while(bound_delta > 0);
	errlog(config, LOG_DEBUG, "Final size is %d\n", (int)current_size);

	//set active to false for longest
	//FIXME find longest by actual extents
	done_block = string_block_longest(blocks);
	blocks[done_block]->calculated = true;
	errlog(config, LOG_DEBUG, "Marked block %d as done\n", done_block);

	return true;
}

bool x11_align_blocks(XRESOURCES* xres, CFG* config, TEXTBLOCK** blocks, unsigned width, unsigned height){
	//align blocks within bounding rectangle according to configured alignment
	unsigned i, total_height = 0, current_height = 0;

	for(i = 0; blocks[i] && blocks[i]->active; i++){
		total_height += blocks[i]->extents.height;
	}

	if(i > 0){
		total_height += config->line_spacing * (i - 1);
	}

	//FIXME this might underflow in some cases
	for(i = 0; blocks[i] && blocks[i]->active; i++){
		//align x axis
		switch(config->alignment){
			case ALIGN_NORTH:
			case ALIGN_SOUTH:
			case ALIGN_CENTER:
				//centered
				blocks[i]->layout_x = (width - (blocks[i]->extents.width)) / 2;
				break;
			case ALIGN_NORTHWEST:
			case ALIGN_WEST:
			case ALIGN_SOUTHWEST:
				//left
				blocks[i]->layout_x = config->padding;
				break;
			case ALIGN_NORTHEAST:
			case ALIGN_EAST:
			case ALIGN_SOUTHEAST:
				//right
				blocks[i]->layout_x = width - (blocks[i]->extents.width) - config->padding;
				break;
		}

		//align y axis
		switch(config->alignment){
			case ALIGN_WEST:
			case ALIGN_EAST:
			case ALIGN_CENTER:
				//centered
				blocks[i]->layout_y = ((height - total_height) / 2)
							+ current_height
							+ ((current_height > 0) ? config->line_spacing:0);
				current_height += blocks[i]->extents.height
						+ ((current_height > 0) ? config->line_spacing:0);
				break;
			case ALIGN_NORTHWEST:
			case ALIGN_NORTH:
			case ALIGN_NORTHEAST:
				//top
				blocks[i]->layout_y = (config->padding)
							+ current_height
							+ ((current_height > 0) ? config->line_spacing:0);
				current_height += blocks[i]->extents.height
						+ ((current_height > 0) ? config->line_spacing:0);
				break;
			case ALIGN_SOUTHWEST:
			case ALIGN_SOUTH:
			case ALIGN_SOUTHEAST:
				//bottom
				blocks[i]->layout_y = height - total_height - (config->padding)
						+ ((current_height > 0) ? config->line_spacing:0);
				total_height -= (blocks[i]->extents.height
						+ ((current_height > 0) ? config->line_spacing:0));
				current_height += blocks[i]->extents.height
						+ ((current_height > 0) ? config->line_spacing:0);
				break;
		}
	}

	return true;
}

bool x11_recalculate_blocks(CFG* config, XRESOURCES* xres, TEXTBLOCK** blocks, unsigned width, unsigned height){
	unsigned i, num_blocks = 0;
	unsigned layout_width = width, layout_height = height;

	//early exit.
	if(!blocks || !blocks[0]){
		return true;
	}

	//initialize calculation set
	for(i = 0; blocks[i] && blocks[i]->active; i++){
		errlog(config, LOG_INFO, "Block %d: %s\n", i, blocks[i]->text);
		if(blocks[i]->text[0]){
			blocks[i]->calculated = false;
		}
		else{
			//disable obviously empty blocks before running maximizer
			errlog(config, LOG_DEBUG, "Disabling empty block %d\n", i);
			blocks[i]->calculated = true;
			blocks[i]->extents.width = 0;
			blocks[i]->extents.height = 0;
			blocks[i]->extents.x = 0;
			blocks[i]->extents.y = 0;
		}
		num_blocks++;
	}

	//calculate layout volume
	if(width > (2 * config->padding)){
		layout_width -= 2 * config->padding;
	}
	if(height > (2 * config->padding)){
		errlog(config, LOG_DEBUG, "Subtracting %d pixels for height padding\n", config->padding);
		layout_height -= 2 * config->padding;
	}
	if(num_blocks > 1 && (((num_blocks - 1) * (config->line_spacing) < layout_height))){
		errlog(config, LOG_DEBUG, "Subtracting %d pixels for linespacing\n", (num_blocks - 1) * config->line_spacing);
		layout_height -= (num_blocks - 1) * config->line_spacing;
	}

	errlog(config, LOG_INFO, "Window volume %dx%d, layout volume %dx%d\n", width, height, layout_width, layout_height);

	if(config->force_size == 0){
		//do binary search for match size
		i = 0;
		do{
			errlog(config, LOG_DEBUG, "Running maximizer for pass %d (%d blocks)\n", i, num_blocks);
			if(!x11_maximize_blocks(xres, config, blocks, layout_width, layout_height)){
				return false;
			}
			i++;
		}
		//do multiple passes if flag is set
		while(config->independent_resize && i < num_blocks);
	}
	else{
		//render with forced size
		if(!x11_blocks_resize(xres, config, blocks, NULL, config->force_size)){
			fprintf(stderr, "Failed to resize blocks\n");
			return false;
		}
	}

	//do alignment pass
	if(!x11_align_blocks(xres, config, blocks, width, height)){
		return false;
	}

	return true;
}
