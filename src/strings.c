bool string_preprocess(char* input, bool handle_escapes){
	unsigned i;
	int c, text_pos = 0;

	if(!input){
		return false;
	}

	for(i = 0; input[i]; i++){
		input[text_pos] = input[i];

		//fprintf(stderr, "Handling input character %c at %d\n", input[i], i);

		if(handle_escapes && input[i] == '\\'){
			//fprintf(stderr, "Handling escape sequence at %d\n", i);
			switch(input[i + 1]){
				case '\\':
					i += 1;
					break;
				case 'n':
					input[text_pos] = '\n';
					i += 1;
					break;
			}
		}

		switch(input[i]){
			//handle tab?
			case '\r':
				//skip back to last newline
				//FIXME handle \r\n
				for(c = text_pos - 1; c >= 0 && input[c]; c--){
					if(input[c] == '\n'){
						break;
					}
				}
				text_pos = c;
				break;
			case '\n':
				//is handled by blockify
				break;
			case '\f':
				//reset text buffer (if data is present)
				if(input[i + 1]){
					text_pos = -1; //is increased directly below
				}
				else{
					//trailing \f, handle in blockify
					//in order to be roughly compatible
					//to sm
				}
				break;
			case '\b':
				//delete one character back, but not over newline
				if(i > 0 && input[i - 1] != '\n'){
					text_pos -= 2;
				}
				break;
		}

		text_pos++;
	}
	input[text_pos] = 0;

	return true;
}

bool string_block_store(TEXTBLOCK* block, char* stream, unsigned length){
	block->text = realloc(block->text, (length + 1) * sizeof(char));
	if(!(block->text)){
		fprintf(stderr, "Failed to allocate memory\n");
		return false;
	}

	strncpy(block->text, stream, length);
	(block->text)[length] = 0;

	block->active = true;
	return true;
}

unsigned string_block_longest(TEXTBLOCK** blocks){
	unsigned i;
	unsigned longest_length = 0, longest_index = 0;
	for(i = 0; blocks[i] && blocks[i]->active; i++){
		if(!(blocks[i]->calculated)){
			if(strlen(blocks[i]->text) > longest_length){
				longest_index = i;
				longest_length = strlen(blocks[i]->text);
			}
		}
	}
	return longest_index;
}

bool string_blockify(TEXTBLOCK*** blocks, char* input){
	unsigned i, num_blocks = 0, blocks_needed = 1, input_offset = 0, current_block = 0;

	for(i = 0; input[i]; i++){
		if(input[i] == '\n' || input[i] == '\f'){
			blocks_needed++;
		}
	}

	if(!(*blocks)){
		//fprintf(stderr, "Initially allocating block container for %d pointers\n", blocks_needed+1);
		//allocate array structure
		(*blocks) = calloc(blocks_needed + 1, sizeof(TEXTBLOCK*));
		if(!(*blocks)){
			fprintf(stderr, "Failed to allocate memory\n");
			return false;
		}
		num_blocks = blocks_needed;
	}
	else{
		for(; (*blocks)[num_blocks]; num_blocks++){
		}

		//fprintf(stderr, "%d blocks currently initialized in set, need %d\n", num_blocks, blocks_needed);
		if(num_blocks < blocks_needed){
			//reallocate for more slots
			(*blocks) = realloc((*blocks), (blocks_needed + 1) * sizeof(TEXTBLOCK*));
			if(!(*blocks)){
				fprintf(stderr, "Failed to allocate memory\n");
				return false;
			}

			for(; num_blocks <= blocks_needed; num_blocks++){
				(*blocks)[num_blocks] = NULL;
			}

			//fprintf(stderr, "After alloc session, now at %d slots\n", num_blocks);
			num_blocks--;
		}
		else{
			//fprintf(stderr, "Not touching block set, is big enough\n");
		}
	}

	//fill empty pointers (else the counting structure is borken)
	for(i = 0; i < num_blocks; i++){
		if(!(*blocks)[i]){
			(*blocks)[i] = calloc(1, sizeof(TEXTBLOCK));
		}
		(*blocks)[i]->active = false;
	}

	for(i = 0; input[i]; i++){
		if(input[i] == '\n' || input[i] == '\f'){
			if(!string_block_store((*blocks)[current_block++], input + input_offset, i - input_offset)){
				return false;
			}
			input_offset = i + 1;
		}
	}
	if(!string_block_store((*blocks)[current_block], input + input_offset, i - input_offset)){
		return false;
	}

	if(((*blocks)[current_block]->text)[0] == 0){
		//fprintf(stderr, "Disabling last block, was empty\n");
		(*blocks)[current_block]->active = false;
	}

	return true;
}
