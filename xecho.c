#include "xecho.h"

int usage(char* fn){
	printf("Usage info for %s\n", fn);
	return 1;
}

int main(int argc, char** argv){
	CFG config={
		0,		//verbosity
		0, 		//padding
		ALIGN_CENTER, 	//alignment
		false, 		//independent resize
		false, 		//handle stdin
		false,		//draw debug boxes
		0, 		//forced size
		NULL,	 	//text color
		NULL,	 	//background color
		NULL,		//debug color name
		NULL		//font name
	};
	XRESOURCES xres={
		0,
		NULL,
		0,
		NULL,
		{},
		{},
		{},
		{NULL, 0}		
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
		x11_cleanup(&xres);
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
			x11_cleanup(&xres);
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
	x11_cleanup(&xres);
	args_cleanup(&config);

	if(args_text){
		free(args_text);
	}

	return 0;
}
