bool x11_init(XRESOURCES* res, CFG* config){
	Window root;
	XSetWindowAttributes window_attributes;

	res->display=XOpenDisplay(NULL);

	if(!(res->display)){
		fprintf(stderr, "Failed to open display\n");
		return false;
	}
	
	res->screen=DefaultScreen(res->display);
	root=RootWindow(res->display, res->screen);

	XftInit(NULL);

	res->text_color=colorspec_parse(config->text_color, res->display, res->screen);
	res->bg_color=colorspec_parse(config->bg_color, res->display, res->screen);
	return true;
}
