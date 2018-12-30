bool xfd_add(X_FDS* set, int fd){
	unsigned i;

	if(!set->fds){
		set->fds = malloc(sizeof(int));
		if(!set->fds){
			fprintf(stderr, "xfd_add: Initial alloc failed\n");
			return false;
		}
		set->size = 1;
		set->fds[0] = fd;
		return true;
	}

	for(i = 0; i < set->size; i++){
		if(set->fds[i] == fd){
			fprintf(stderr, "xfd_add: Not pushing duplicate entry\n");
			return false;
		}
	}

	set->fds = realloc(set->fds, (set->size + 1) * sizeof(int));
	if(!set->fds){
		fprintf(stderr, "xfd_add: Failed to realloc fd set\n");
		return false;
	}

	set->fds[set->size] = fd;
	set->size++;

	return true;
}

bool xfd_remove(X_FDS* set, int fd){
	unsigned i, c;

	for(i = 0; i < set->size; i++){
		if(set->fds[i] == fd){
			for(c = i; c < set->size - 1; c++){
				set->fds[c] = set->fds[c + 1];
			}

			set->size--;
			set->fds = realloc(set->fds, set->size * sizeof(int));
			if(!set->fds && set->size > 0){
				fprintf(stderr, "xfd_remove: Failed to realloc\n");
				return false;
			}
			return true;
		}
	}

	fprintf(stderr, "xfd_remove: FD not in set\n");
	return false;
}

void xfd_free(X_FDS* set){
	if(set->fds){
		free(set->fds);
		set->fds = NULL;
	}
	set->size = 0;
}

void xconn_watch(Display* dpy, XPointer client_data, int fd, Bool opening, XPointer* watch_data){
	if(opening){
		fprintf(stderr, "xconn_watch: Internal connection registered\n");
		xfd_add((X_FDS*)client_data, fd);
	}
	else{
		fprintf(stderr, "xconn_watch: Internal connection closed\n");
		xfd_remove((X_FDS*)client_data, fd);
	}
}

