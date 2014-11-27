int xecho(CFG* config, XRESOURCES* xres, char* initial_text){
	fd_set readfds;
	struct timeval tv;
	int maxfd, error;
	unsigned i;
	bool abort=false;
	XEvent event;

	unsigned window_width=0, window_height=0;

	TEXTBLOCK** blocks=NULL; //TODO free
	
	//prepare initial block buffer
	if(initial_text){
		if(!string_blockify(&blocks, initial_text)){
			return -1;
		}
	}

	while(!abort){
		//handle events
		while(XPending(xres->display)){
			XNextEvent(xres->display, &event);
			//handle events
			switch(event.type){
				case ConfigureNotify:
					if(window_width!=event.xconfigure.width||window_height!=event.xconfigure.height){
						window_width=event.xconfigure.width;
						window_height=event.xconfigure.height;
						XClearWindow(xres->display, xres->main);

						fprintf(stderr, "Recalculating font sizes\n");
						//recalculate size
						if(!x11_recalculate_fonts(config, xres, blocks)){
							fprintf(stderr, "Font size calculation failed\n");
							return -1;
						}
					}

					fprintf(stderr, "Window configured to %dx%d\n", window_width, window_height);
					break;
				
				case Expose:
					//TODO draw here
					fprintf(stderr, "Window exposed\n");
					break;

				case KeyPress:
					switch(event.xkey.keycode){
						case 24:
							abort=true;
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
				//TODO read
				//TODO preprocess
				//TODO blockify
			}

		}
		else if(error<0){
			perror("select");
			abort=true;
		}
	}

	return 0;
}
