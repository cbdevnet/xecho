int xecho(CFG* config, XRESOURCES* xres, char* initial_text){
	fd_set readfds;
	struct timeval tv;
	int maxfd, error;
	unsigned i;
	int abort=0;
	XEvent event;

	unsigned window_width=0, window_height=0;
	unsigned display_buffer_length=0, display_buffer_offset;

	TEXTBLOCK** blocks=NULL;
	char* display_buffer=NULL;
	
	//prepare initial block buffer
	if(initial_text){
		if(!string_blockify(&blocks, initial_text)){
			fprintf(stderr, "Failed to blockify initial input text\n");
			return -1;
		}
	}

	//copy initial text to stdin buffer
	if(config->handle_stdin){
		display_buffer_length=STDIN_DATA_CHUNK+(initial_text?strlen(initial_text):0)+1;
		display_buffer=calloc(display_buffer_length, sizeof(char));
		if(!display_buffer){
			fprintf(stderr, "Failed to allocate memory\n");
			return -1;
		}
		if(initial_text){
			strncpy(display_buffer, initial_text, strlen(initial_text));
		}
	}

	while(!abort){
		//handle events
		while(XPending(xres->display)){
			XNextEvent(xres->display, &event);
			//handle events
			switch(event.type){
				case ConfigureNotify:
					fprintf(stderr, "Window configured to %dx%d\n", event.xconfigure.width, event.xconfigure.height);
					if(window_width!=event.xconfigure.width||window_height!=event.xconfigure.height){
						window_width=event.xconfigure.width;
						window_height=event.xconfigure.height;

						fprintf(stderr, "Recalculating blocks\n");

						//recalculate size
						if(!x11_recalculate_blocks(config, xres, blocks, window_width, window_height)){
							fprintf(stderr, "Block calculation failed\n");
							abort=-1;
						}
					}
					else{
						fprintf(stderr, "Configuration not changed, ignoring\n");
					}
					break;
				
				case Expose:
					//draw here
					fprintf(stderr, "Window exposed, clearing and redrawing\n");
					XClearWindow(xres->display, xres->main);
					if(!x11_draw_blocks(config, xres, blocks)){
						fprintf(stderr, "Failed to draw blocks\n");
						abort=-1;
					}
					break;

				case KeyPress:
					switch(event.xkey.keycode){
						case 24:
							abort=-1;
							break;
						case 27:
							fprintf(stderr, "Redrawing on request\n");
							if(!x11_recalculate_blocks(config, xres, blocks, window_width, window_height)){
								fprintf(stderr, "Block calculation failed\n");
								abort=-1;
							}
							XClearWindow(xres->display, xres->main);
							if(!x11_draw_blocks(config, xres, blocks)){
								fprintf(stderr, "Failed to draw blocks\n");
								abort=-1;
							}
							break;
						default:
							fprintf(stderr, "KeyPress %d\n", event.xkey.keycode);
							break;
					}
					break;

				case ClientMessage:
					fprintf(stderr, "Client message\n");
					break;

				default:
					fprintf(stderr, "Unhandled X event\n");
					break;
			}
		}

		XFlush(xres->display);

		if(abort){
			break;
		}

		//prepare select data
		FD_ZERO(&readfds);
		maxfd=-1;
		tv.tv_sec=10;
		tv.tv_usec=0;

		for(i=0;i<xres->xfds.size;i++){
			FD_SET(xres->xfds.fds[i], &readfds);
			if(maxfd<xres->xfds.fds[i]){
				maxfd=xres->xfds.fds[i];
			}
		}

		if(config->handle_stdin){
			FD_SET(fileno(stdin), &readfds);
			if(maxfd<fileno(stdin)){
				maxfd=fileno(stdin);
			}
		}

		error=select(maxfd+1, &readfds, NULL, NULL, &tv);
		if(error>0){
			if(FD_ISSET(fileno(stdin), &readfds)){
				//handle stdin input
				fprintf(stderr, "Data on stdin\n");

				do{
					display_buffer_offset=strlen(display_buffer);
					fprintf(stderr, "Display buffer is %d long, offset is %d\n", display_buffer_length, display_buffer_offset);
					if(display_buffer_length-display_buffer_offset<STDIN_DATA_CHUNK){
						//reallocate
						display_buffer_length+=STDIN_DATA_CHUNK;
						display_buffer=realloc(display_buffer, display_buffer_length*sizeof(char));
						if(!display_buffer){
							fprintf(stderr, "Failed to reallocate display data buffer\n");
							abort=-1;
						}
						fprintf(stderr, "Reallocated display buffer to %d bytes\n", display_buffer_length);
					}

					//read data
					error=read(fileno(stdin),
							display_buffer+display_buffer_offset,
							display_buffer_length-1-display_buffer_offset
						  );

					fprintf(stderr, "Read %d bytes from stdin\n", error);

					//terminate string
					if(error>0){
						display_buffer[display_buffer_offset+error]=0;
					}

				}while(error>0);

				//check if stdin was closed
				if(error==0){
					config->handle_stdin=false;
				}
				
				switch(errno){
					case EAGAIN:
						//would block, so done reading
						//preprocess input data to filter control codes
						if(!string_preprocess(display_buffer, false)){
							fprintf(stderr, "Failed to preprocess input text\n");
							abort=-1;
						}
						fprintf(stderr, "Updated display text to\n\"%s\"\n", display_buffer);

						//blockify
						if(!string_blockify(&blocks, display_buffer)){
							fprintf(stderr, "Failed to blockify updated input\n");
							abort=-1;
						}

						//render new
						if(!x11_recalculate_blocks(config, xres, blocks, window_width, window_height)){
							fprintf(stderr, "Block calculation failed\n");
							abort=-1;
						}
						XClearWindow(xres->display, xres->main);
						if(!x11_draw_blocks(config, xres, blocks)){
							fprintf(stderr, "Failed to draw blocks\n");
							abort=-1;
						}
						break;
					default:
						fprintf(stderr, "Failed to read stdin\n");
						abort=-1;
				}
			}
		}
		else if(error<0){
			perror("select");
			abort=-1;
		}
	}

	//free data
	if(display_buffer){
		free(display_buffer);
	}
	
	if(blocks){
		//free blocks structure
		for(i=0;blocks[i];i++){
			if(blocks[i]->text){
				free(blocks[i]->text);
			}
			free(blocks[i]);
		}
		free(blocks);
	}

	return abort;
}
