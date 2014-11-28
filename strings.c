bool string_preprocess(char* input, bool handle_escapes){
	unsigned i, c;
	int text_pos=0;

	if(!input){
		return false;
	}

	for(i=0;input[i];i++){
		input[text_pos]=input[i];

		//fprintf(stderr, "Handling input character %c at %d\n", input[i], i);

		if(handle_escapes&&input[i]=='\\'){
			//fprintf(stderr, "Handling escape sequence at %d\n", i);
			switch(input[i+1]){
				case '\\':
					i+=1;
					break;
				case 'n':
					input[text_pos]='\n';
					i+=1;
					break;
			}
		}

		switch(input[i]){
			//handle tab?
			case '\r':
				//skip back to last newline
				for(c=text_pos-1;c>=0&&input[c];c--){
					if(input[c]=='\n'){
						break;
					}
				}
				text_pos=c;
				break;
			case '\n':
				//is handled by blockify
				break;
			case '\f':
				//reset text buffer
				text_pos=-1; //is increased directly below
				break;
			case '\b':
				//TODO delete one character back, but not over newline
				break;
		}

		text_pos++;
	}
	input[text_pos]=0;
	return true;
}

bool string_block_store(TEXTBLOCK* block, char* stream, unsigned length){
	block->text=realloc(block->text, (length+1)*sizeof(char));
	if(!(block->text)){
		fprintf(stderr, "Failed to allocate memory\n");
		return false;
	}

	strncpy(block->text, stream, length);
	(block->text)[length]=0;

	block->active=true;
	return true;
}

bool string_blockify(TEXTBLOCK*** blocks, char* input){
	unsigned i, num_blocks=0, blocks_needed=1, input_offset=0, current_block=0;

	for(i=0;input[i];i++){
		if(input[i]=='\n'){
			blocks_needed++;
		}
	}
	
	if(!(*blocks)){
		fprintf(stderr, "Allocating block container for %d blocks\n", blocks_needed);
		//allocate array structure
		*blocks=calloc(sizeof(TEXTBLOCK*), blocks_needed+1);
		if(!(*blocks)){
			fprintf(stderr, "Failed to allocate memory\n");
			return false;
		}
		num_blocks=blocks_needed;
	}
	else{
		for(i=0;(*blocks)[i];i++){
			num_blocks++;
		}
		fprintf(stderr, "%d blocks currently allocated in set, need %d\n", num_blocks, blocks_needed);
		if(num_blocks<blocks_needed){
			//reallocate for more slots
			(*blocks)=realloc((*blocks), (blocks_needed+1)*sizeof(TEXTBLOCK*));
			if(!(*blocks)){
				fprintf(stderr, "Failed to allocate memory\n");
				return false;
			}
			
			for(num_blocks=num_blocks+1;num_blocks<blocks_needed;num_blocks++){
				blocks[num_blocks]=NULL;
			}
			blocks[num_blocks]=NULL;

			fprintf(stderr, "After alloc session, now at %d slots\n", num_blocks);
		}
		else{
			fprintf(stderr, "Not touching block set, is big enough\n");
		}
	}

	//fill empty pointers (else the counting structure is borken)
	for(i=0;i<num_blocks;i++){
		if(!(*blocks)[i]){
			(*blocks)[i]=calloc(1, sizeof(TEXTBLOCK));
		}
		(*blocks)[i]->active=false;
	}

	for(i=0;input[i];i++){
		if(input[i]=='\n'){
			fprintf(stderr, "Storing block %d\n", current_block);
			if(!string_block_store((*blocks)[current_block++], input+input_offset, i-input_offset)){
				return false;
			}
			input_offset=i+1;
		}
	}
	fprintf(stderr, "Storing last block %d\n", current_block);
	if(!string_block_store((*blocks)[current_block++], input+input_offset, i-input_offset)){
		return false;
	}

	return true;
}
