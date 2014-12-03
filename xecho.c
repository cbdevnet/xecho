#include "xecho.h"

int usage(char* fn){
	printf("xecho - Render text to X\n\n");
	printf("Usage: %s <arguments> <text>\n", fn);
	printf("Recognized options:\n");
	printf("\t--\t\t\t\tStop argument parsing,\n\t\t\t\t\ttreat all following arguments as text\n\n");
	printf("\t-font <fontspec>\t\tSelect font by FontConfig name\n\n");
	printf("\t-fc <colorspec>\t\t\tSet text color by name or HTML code\n\n");
	printf("\t-bc <colorspec>\t\t\tSet background color by name or code\n\n");
	printf("\t-size <n>\t\t\tRender at font size n\n\n");
	printf("\t-maxsize <n>\t\t\tLimit font size to n at max\n\n");
	printf("\t-align [n|ne|e|se|s|sw|w|nw]\tAlign text\n\n");
	printf("\t-padding <n>\t\t\tPad text by n pixels\n\n");
	printf("\t-linespacing <n>\t\tPad lines by n pixels\n\n");
	printf("Recognized flags:\n");
	printf("\t-stdin\t\t\t\tUpdate text from stdin,\n\t\t\t\t\t\\f (Form feed) clears text,\n\t\t\t\t\t\\r (Carriage return) clears current line\n\n");
	printf("\t-independent-lines\t\tResize every line individually\n\n");
	printf("\t-debugboxes\t\t\tDraw debug boxes\n\n");
	printf("\t-disable-text\t\t\tDo not render text at all.\n\t\t\t\t\tMight be useful for playing tetris.\n\n");
	printf("\t-disable-doublebuffer\t\tDo not use XDBE\n\n");
	printf("\t-v[v[v]]\t\t\tIncrease output verbosity\n\n");
	return 1;
}

int main(int argc, char** argv){
	CFG config={
		0,		//verbosity
		0, 		//padding
		0,		//line spacing
		0,		//max size
		ALIGN_CENTER, 	//alignment
		false, 		//independent resize
		false, 		//handle stdin
		false,		//draw debug boxes
		false,		//disable text drawing
		true,		//use double buffering
		0, 		//forced size
		NULL,	 	//text color
		NULL,	 	//background color
		NULL,		//debug color name
		NULL,		//font name
	};
	XRESOURCES xres={
		0,		//screen
		NULL,		//display
		0,		//window
		0,		//back buffer
		NULL,		//xft drawable
		{},		//text color
		{},		//bg color
		{},		//debug color
		{NULL, 0}	//xfd set
	};
	int args_end;
	unsigned text_length, i;
	char* args_text=NULL;
	long flags;

	//parse command line arguments
	args_end=args_parse(&config, argc-1, argv+1);
	if(argc-args_end<1&&!config.handle_stdin){
		return usage(argv[0]);
	}

	//config sanity check
	if(!args_sane(&config)){
		args_cleanup(&config);
		return usage(argv[0]);
	}

	//set up x11 display
	if(!x11_init(&xres, &config)){
		x11_cleanup(&xres, &config);
		args_cleanup(&config);
		return usage(argv[0]);
	}

	//preprocess display text if given
	if(argc-args_end>0){
		//copy arguments into buffer
		text_length=0;
		for(i=args_end;i<argc;i++){
			text_length+=strlen(argv[i])+1;
		}

		args_text=calloc(text_length, sizeof(char));

		text_length=0;
		for(i=args_end;i<argc;i++){
			strncpy(args_text+text_length, argv[i], strlen(argv[i]));
			text_length+=strlen(argv[i]);
			args_text[text_length++]=' ';
		}
		args_text[((text_length>0)?text_length:1)-1]=0;

		fprintf(stderr, "Input text:\n\"%s\"\n", args_text);

		//preprocess
		if(!string_preprocess(args_text, true)){
			printf("Failed to preprocess input text\n");
			x11_cleanup(&xres, &config);
			args_cleanup(&config);
			return usage(argv[0]);
		}

		fprintf(stderr, "Printing text:\n\"%s\"\n", args_text);
	}

	//prepare stdin
	if(config.handle_stdin){
		fprintf(stderr, "Marking stdin as nonblocking\n");
		flags=fcntl(0, F_GETFL, 0);
		flags|=O_NONBLOCK;
		fcntl(0, F_SETFL, flags);
	}
	
	//enter main loop
	xecho(&config, &xres, args_text);

	//clear data
	x11_cleanup(&xres, &config);
	args_cleanup(&config);

	if(args_text){
		free(args_text);
	}

	fprintf(stderr, "xecho shutdown ok\n");

	return 0;
}
