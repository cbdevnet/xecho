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
		NULL		//fontname
	};
	int args_end;

	//parse command line arguments
	args_end=args_parse(&config, argc-1, argv+1);
	if(args_end<0){
		return usage(argv[0]);
	}

	//config sanity check
	if(!args_sane(&config)){
		return usage(argv[0]);
	}


	//set up x11 display
	//enter main loop
	
	return 0;
}
