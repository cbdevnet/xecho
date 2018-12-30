unsigned short colorspec_read_byte(char* cs){
	char* hexmap = "0123456789abcdef";
	unsigned short rv = 0;
	int i;
	if(*cs != 0 && cs[1] != 0){
		for(i = 0; hexmap[i] != 0 && hexmap[i] != cs[0]; i++){
		}
		rv |= (i << 12);
		for(i = 0; hexmap[i] != 0 && hexmap[i] != cs[1]; i++){
		}
		rv |= (i << 8);
	}
	return rv;
}

XftColor colorspec_parse(char* cs, Display* display, int screen){
	XftColor rv = {};
	XRenderColor xrender_color = {0,0,0,0xffff};
	int i;

	if(*cs == '#'){
		if(strlen(cs) != 7){
			fprintf(stderr, "Invalid colorspec length\n");
		}

		for(i = 1; i < strlen(cs); i++){
			if(!isxdigit(cs[i])){
				fprintf(stderr, "Invalid digit in colorspec: %c\n", cs[i]);
				return rv;
			}
		}

		xrender_color.red = colorspec_read_byte(cs + 1);
		xrender_color.green = colorspec_read_byte(cs + 3);
		xrender_color.blue = colorspec_read_byte(cs + 5);

		fprintf(stderr, "Read colorspec %s as r:%04x g:%04x b:%04x\n", cs, xrender_color.red, xrender_color.green, xrender_color.blue);

		if(!XftColorAllocValue(display, DefaultVisual(display, screen), DefaultColormap(display, screen), &xrender_color, &rv)){
			fprintf(stderr, "Failed to allocate color\n");
		}
	}
	else{
		if(!XftColorAllocName(display, DefaultVisual(display, screen), DefaultColormap(display, screen), cs, &rv)){
			fprintf(stderr, "Failed to get color by name\n");
		}
	}
	return rv;
}
