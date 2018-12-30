#include "xecho.h"

int usage(char* fn){
	printf("xecho - Render text to X\n\n");
	printf("Usage: %s <arguments> <text>\n", fn);
	printf("Recognized options:\n");
	printf("\t--\t\t\t\tStop argument parsing,\n\t\t\t\t\ttreat all following arguments as content text\n\n");
	printf("\t-font <fontspec>\t\tSelect font by FontConfig name\n\n");
	printf("\t-fc <colorspec>\t\t\tSet text color by name or HTML code\n\n");
	printf("\t-bc <colorspec>\t\t\tSet background color by name or code\n\n");
	printf("\t-dc <colorspec>\t\t\tSet debug color by name or code\n\n");
	printf("\t-title <title>\t\t\tSet window title\n\n");
	printf("\t-size <n>\t\t\tRender at font size n\n\n");
	printf("\t-maxsize <n>\t\t\tLimit font size to n at max\n\n");
	printf("\t-align [n|ne|e|se|s|sw|w|nw]\tAlign text\n\n");
	printf("\t-padding <n>\t\t\tPad text by n pixels\n\n");
	printf("\t-linespacing <n>\t\tPad lines by n pixels\n\n");
	printf("Recognized flags:\n");
	printf("\t-no-stdin\t\t\tDisable content updates from stdin\n\n");
	printf("\t-windowed\t\t\tDo not try to force a fullscreen window\n\n");
	printf("\t-independent-lines\t\tResize every line individually\n\n");
	printf("\t-debugboxes\t\t\tDraw debug boxes\n\n");
	printf("\t-disable-text\t\t\tDo not render text at all.\n\t\t\t\t\tMight be useful for playing tetris.\n\n");
	printf("\t-disable-doublebuffer\t\tDo not use XDBE\n\n");
	printf("\t-h | -help | --help\t\tPrint this usage information\n\n");
	printf("\t-v[v[v]]\t\t\tIncrease output verbosity\n\n");
	printf("stdin content update protocol:\n");
	printf("\t\\f (Form feed) clears text,\n");
	printf("\t\\r (Carriage return) clears current line\n\n");
	return 1;
}

void errlog(CFG* config, unsigned level, char* fmt, ...){
	va_list args;
	va_start(args, fmt);
	if(config->verbosity >= level){
		vfprintf(stderr, fmt, args);
	}
	va_end(args);
}

int main(int argc, char** argv){
	CFG config = {
		.verbosity = 0,
		.padding = 0,
		.line_spacing = 0,
		.max_size = 0,
		.alignment = ALIGN_CENTER,
		.independent_resize = false,
		.handle_stdin = true,
		.debug_boxes = false,
		.disable_text = false,
		.double_buffer = true,
		.windowed = false,
		.print_usage = false,
		.force_size = 0,
		.text_color = NULL,
		.bg_color = NULL,
		.debug_color = NULL,
		.font_name = NULL,
		.window_name = NULL
	};

	XRESOURCES xres = {
		.screen = 0,
		.display = NULL,
		.main = 0,
		.back_buffer = 0,
		.drawable = NULL,
		.text_color = {},
		.bg_color = {},
		.debug_color = {},
		.wm_delete = 0,
		.xfds = {NULL, 0}
	};

	int args_end;
	unsigned text_length, i;
	char* args_text = NULL;
	long flags;

	//parse command line arguments
	args_end = args_parse(&config, argc - 1, argv + 1);
	if(argc - args_end < 1 && !config.handle_stdin){
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
	if(argc - args_end > 0){
		//copy arguments into buffer
		text_length = 0;
		for(i = args_end; i < argc; i++){
			text_length += strlen(argv[i]) + 1;
		}

		args_text = calloc(text_length + 1, sizeof(char));

		text_length = 0;
		for(i = args_end; i < argc; i++){
			strncpy(args_text + text_length, argv[i], strlen(argv[i]));
			text_length += strlen(argv[i]);
			args_text[text_length++] = ' ';
		}
		args_text[((text_length > 0) ? text_length:1) - 1] = 0;

		errlog(&config, LOG_INFO, "Input text:\n\"%s\"\n", args_text);

		//preprocess
		if(!string_preprocess(args_text, true)){
			fprintf(stderr, "Failed to preprocess input text\n");
			x11_cleanup(&xres, &config);
			args_cleanup(&config);
			return usage(argv[0]);
		}

		errlog(&config, LOG_DEBUG, "Printing text:\n\"%s\"\n", args_text);
	}

	//prepare stdin
	if(config.handle_stdin){
		errlog(&config, LOG_INFO, "Marking stdin as nonblocking\n");
		flags = fcntl(0, F_GETFL, 0);
		flags |= O_NONBLOCK;
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

	errlog(&config, LOG_INFO, "xecho shutdown ok\n");

	return 0;
}
