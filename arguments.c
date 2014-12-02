int args_parse(CFG* config, int argc, char** argv){
	unsigned i, c;

	for(i=0;i<argc;i++){
		if(!strcmp(argv[i], "-padding")){
			if(++i<argc){
				config->padding=strtoul(argv[i], NULL, 10);
			}
			else{
				fprintf(stderr, "No parameter for padding\n");
				return -1;
			}
		}
		else if(!strcmp(argv[i], "-linespacing")){
			if(++i<argc){
				config->line_spacing=strtoul(argv[i], NULL, 10);
			}
			else{
				fprintf(stderr, "No parameter for line spacing\n");
				return -1;
			}
		}
		else if(!strcmp(argv[i], "-maxsize")){
			if(++i<argc){
				config->max_size=strtoul(argv[i], NULL, 10);
			}
			else{
				fprintf(stderr, "No parameter for max size\n");
				return -1;
			}
		}
		else if(!strcmp(argv[i], "-align")){
			if(++i<argc){
				switch(argv[i][0]){
					case 'n':
					case 'N':
						switch(argv[i][1]){
							case 'e':
							case 'E':
								config->alignment=ALIGN_NORTHEAST;
								break;
							case 'w':
							case 'W':
								config->alignment=ALIGN_NORTHWEST;
								break;
							default:
								config->alignment=ALIGN_NORTH;
						}
						break;
					case 'e':
					case 'E':
						config->alignment=ALIGN_EAST;
						break;
					case 's':
					case 'S':
						switch(argv[i][1]){
							case 'e':
							case 'E':
								config->alignment=ALIGN_SOUTHEAST;
								break;
							case 'w':
							case 'W':
								config->alignment=ALIGN_SOUTHWEST;
								break;
							default:
								config->alignment=ALIGN_SOUTH;
						}
						break;
					case 'w':
					case 'W':
						config->alignment=ALIGN_WEST;
						break;
					default:
						fprintf(stderr, "Invalid alignment specifier\n");
						return -1;
				}
			}
			else{
				fprintf(stderr, "No parameter for alignment\n");
				return -1;
			}
		}
		else if(!strcmp(argv[i], "-size")){
			if(++i<argc){
				config->force_size=(double)strtoul(argv[i], NULL, 10);
			}
			else{
				fprintf(stderr, "No parameter for size\n");
				return -1;
			}
		}
		else if(!strcmp(argv[i], "-independent-lines")){
			config->independent_resize=true;
		}
		else if(!strcmp(argv[i], "-notext")){
			config->disable_text=true;
		}
		else if(!strcmp(argv[i], "-disable-doublebuffer")){
			config->double_buffer=false;
		}
		else if(!strcmp(argv[i], "-fc")){
			if(++i<argc&&!(config->text_color)){
				config->text_color=calloc(strlen(argv[i])+1, sizeof(char));
				if(!(config->text_color)){
					fprintf(stderr, "Failed to allocate memory\n");
					return -1;
				}
				strncpy(config->text_color, argv[i], strlen(argv[i]));
			}
			else{
				fprintf(stderr, "No parameter for text color or already defined\n");
				return -1;
			}
		}
		else if(!strcmp(argv[i], "-bc")){
			if(++i<argc&&!(config->bg_color)){
				config->bg_color=calloc(strlen(argv[i])+1, sizeof(char));
				if(!(config->bg_color)){
					fprintf(stderr, "Failed to allocate memory\n");
					return -1;
				}
				strncpy(config->bg_color, argv[i], strlen(argv[i]));
			}
			else{
				fprintf(stderr, "No parameter for window color or already defined\n");
				return -1;
			}
		}
		else if(!strcmp(argv[i], "-stdin")){
			config->handle_stdin=true;
		}
		else if(!strcmp(argv[i], "-debugboxes")){
			config->debug_boxes=true;
		}
		else if(!strcmp(argv[i], "-font")){
			if(++i<argc&&!(config->font_name)){
				config->font_name=calloc(strlen(argv[i])+1, sizeof(char));
				if(!(config->font_name)){
					fprintf(stderr, "Failed to allocate memory\n");
					return -1;
				}
				strncpy(config->font_name, argv[i], strlen(argv[i]));
			}
			else{
				fprintf(stderr, "No parameter for font or already defined\n");
				return -1;
			}

		}
		else if(!strncmp(argv[i], "-v", 2)){
			for(c=1;argv[i][c]=='v';c++){
			}
			config->verbosity=c-1;
		}
		else if(!strncmp(argv[i], "--", 2)){
			i++;
			break;
		}
		else{
			break;
		}
	}

	return i+1;
}

bool args_sane(CFG* config){
	//string memory is allocated in order to be able to
	//simply free() them later

	if(!(config->font_name)){
		fprintf(stderr, "No font name specified, using default.\n");
		config->font_name=calloc(strlen(DEFAULT_FONT)+1, sizeof(char));
		if(!(config->font_name)){
			fprintf(stderr, "Failed to allocate memory\n");
			return false;
		}
		strncpy(config->font_name, DEFAULT_FONT, strlen(DEFAULT_FONT));
	}

	if(!(config->bg_color)){
		fprintf(stderr, "No window color specified, using default\n");
		config->bg_color=calloc(strlen(DEFAULT_WINCOLOR)+1, sizeof(char));
		if(!(config->bg_color)){
			fprintf(stderr, "Failed to allocate memory\n");
			return false;
		}
		strncpy(config->bg_color, DEFAULT_WINCOLOR, strlen(DEFAULT_WINCOLOR));
	}

	if(!(config->text_color)){
		fprintf(stderr, "No text color specified, using default\n");
		config->text_color=calloc(strlen(DEFAULT_TEXTCOLOR)+1, sizeof(char));
		if(!(config->text_color)){
			fprintf(stderr, "Failed to allocate memory\n");
			return false;
		}
		strncpy(config->text_color, DEFAULT_TEXTCOLOR, strlen(DEFAULT_TEXTCOLOR));
	}

	if(!(config->debug_color)){
		//TODO make this configurable
		fprintf(stderr, "No debug color specified, using default\n");
		config->debug_color=calloc(strlen(DEFAULT_DEBUGCOLOR)+1, sizeof(char));
		if(!(config->debug_color)){
			fprintf(stderr, "Failed to allocate memory\n");
			return false;
		}
		strncpy(config->debug_color, DEFAULT_DEBUGCOLOR, strlen(DEFAULT_DEBUGCOLOR));
	}

	if(config->verbosity>1){
		fprintf(stderr, "Config summary\n");
		fprintf(stderr, "Verbosity level: %d\n", config->verbosity);
		fprintf(stderr, "Text padding: %d\n", config->padding);
		fprintf(stderr, "Line spacing: %d\n", config->line_spacing);
		fprintf(stderr, "Maximum size: %d\n", config->max_size);
		fprintf(stderr, "Text alignment: %d\n", config->alignment);
		fprintf(stderr, "Resize lines independently: %s\n", config->independent_resize?"true":"false");
		fprintf(stderr, "Handle stdin: %s\n", config->handle_stdin?"true":"false");
		fprintf(stderr, "Draw debug boxes: %s\n", config->debug_boxes?"true":"false");
		fprintf(stderr, "Disable text draw: %s\n", config->disable_text?"true":"false");
		fprintf(stderr, "Forced text size: %d\n", (int)config->force_size);
		fprintf(stderr, "Text colorspec: %s\n", config->text_color);
		fprintf(stderr, "Window colorspec: %s\n", config->bg_color);
		fprintf(stderr, "Debug colorspec: %s\n", config->debug_color);
		fprintf(stderr, "Font name: %s\n", config->font_name);
	}

	return true;
}

void args_cleanup(CFG* config){
	free(config->text_color);
	free(config->bg_color);
	free(config->debug_color);
	free(config->font_name);
}
