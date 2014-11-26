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
				//delete one character back, but not over newline
				//TODO
				break;
		}

		text_pos++;
	}
	input[text_pos]=0;
	return true;
}

int string_blockify(TEXTBLOCK** blocks, char* input){
	return false;
}
