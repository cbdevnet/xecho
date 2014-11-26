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
		0, 		//forced size
		NULL,	 	//text color
		NULL,	 	//background color
		NULL		//font name
	};
	XRESOURCES xres={
		0,
		NULL,
		0,
		NULL,
		{},
		{},
		{NULL, 0}		
	};
	int args_end;
	char* args_text=NULL;

	//parse command line arguments
	args_end=args_parse(&config, argc-1, argv+1);
	if(args_end<0){
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

	//preprocess display text if not from stdin
	if(!(config.handle_stdin)){
		//copy arguments into buffer
		if(!string_preprocess(args_text, true)){
			printf("Failed to preprocess input text\n");
			x11_cleanup(&xres);
			args_cleanup(&config);
			return usage(argv[0]);
		}
	}
	
	//enter main loop
	xecho(&config, &xres, args_text);

	//clear data
	x11_cleanup(&xres);
	args_cleanup(&config);
	return 0;
}
