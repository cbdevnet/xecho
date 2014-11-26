int xecho(CFG* config, XRESOURCES* xres, char* initial_text){
	fd_set readfds;
	struct timeval tv;
	int maxfd, error;
	unsigned i;
	bool abort=false;
	XEvent event;

	while(!abort){
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
				//TODO
			}

			//handle x events
			while(XPending(xres->display)){
				XNextEvent(xres->display, &event);
				//TODO handle events
				switch(event.type){
					case KeyPress:
						return 0;
					default:
						fprintf(stderr, "Unhandled X event\n");
						break;
				}
			}

			XFlush(xres->display);
		}
		else if(error<0){
			perror("select");
			return -1;
		}
	}

	return 0;
}
