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
		else if(!strcmp(argv[i], "-align")){
			if(++i<argc){
				if(!strcmp(argv[i], "left")){
					config->alignment=ALIGN_LEFT;
				}
				else if(!strcmp(argv[i], "right")){
					config->alignment=ALIGN_RIGHT;
				}
				else if(!strcmp(argv[i], "center")){
					config->alignment=ALIGN_CENTER;
				}
				else{
					fprintf(stderr, "Invalid alignment\n");
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
				config->force_size=strtoul(argv[i], NULL, 10);
			}
			else{
				fprintf(stderr, "No parameter for size\n");
				return -1;
			}
		}
		else if(!strcmp(argv[i], "-independent-lines")){
			config->independent_resize=true;
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

	if(config->verbosity>1){
		fprintf(stderr, "Config summary\n");
		fprintf(stderr, "Verbosity level: %d\n", config->verbosity);
		fprintf(stderr, "Text padding: %d\n", config->padding);
		fprintf(stderr, "Text alignment: %d\n", config->alignment);
		fprintf(stderr, "Resize lines independently: %s\n", config->independent_resize?"true":"false");
		fprintf(stderr, "Handle stdin: %s\n", config->handle_stdin?"true":"false");
		fprintf(stderr, "Forced text size: %d\n", config->force_size);
		fprintf(stderr, "Text colorspec: %s\n", config->text_color);
		fprintf(stderr, "Window colorspec: %s\n", config->bg_color);
		fprintf(stderr, "Font name: %s\n", config->font_name);
	}

	return true;
}

void args_cleanup(CFG* config){
	free(config->text_color);
	free(config->bg_color);
	free(config->font_name);
}
