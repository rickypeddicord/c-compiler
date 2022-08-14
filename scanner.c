#include "file_util.h"
#include "scanner.h"
#include "parser.h"
#include <ctype.h>

// global variables
int commentFlag = 0;
int intFlag = 0; 
int newLine = 0; 
int count = 0;
int errorCount = 0;
int lineCount = -1;
int errorArrIdx = 0;
extern int endFlag;
extern int scanFlag;
extern int synErrorIdx;
extern int synErrorCount;
extern char synArr[CHAR_LIMIT * 2][CHAR_LIMIT * 2];
extern char synArr2[CHAR_LIMIT * 2][CHAR_LIMIT * 2];
char lineBuffer[CHAR_LIMIT * 2];
char tempBuffer[CHAR_LIMIT * 2];
char intBuffer[CHAR_LIMIT * 2];
char errorArr[CHAR_LIMIT * 2][CHAR_LIMIT * 2];
extern char parseBuffer[CHAR_LIMIT * 2];


int isComment() {
	return commentFlag;
}


int getNewLine() {
	return newLine;
}


// will return the next token from the input file (enum value)
// will create the token buffer which contains the literal token from the input file
// will create the listing file!
Token scanner(char *buffer, FILE *in_file, FILE *out_file, FILE *list_file, int print) {
	int index = 0;
	int lineIndex = 0;
	int i;
	char content;
	commentFlag = 0;
	intFlag = 0;
	newLine = 0;
	Token token;
	
	clear_buffer(buffer);
	clear_buffer(lineBuffer);

	if (print == 1) {
		if (lineCount == -1) {
			lineCount = 1;
			fprintf(list_file, "%d.\t", lineCount);
		}
	}
	
	// get first character
	content = fgetc(in_file);
	
	if (isspace(content)) {
		while (isspace(content)) {
			
			// check for end of line here
			if (content == '\n') {
				// new line flag for listing file
				newLine = 1;
				if (count < 2) { // deal with some lines getting an extra newline character
					strcat(parseBuffer, "\n");
				}
				count++;
				if (print == 1) {
					lineCount++;
				}

			} else {
				// if not a new line character, add characters to linebuffer
				add_char(lineBuffer, content, lineIndex);
				lineIndex++;
			}
			content = fgetc(in_file);
			if (content == '\n') {
				if (print == 1) {
					// if only line character is a new line -- aka empty line
					fprintf(list_file, "\n%d.\t", lineCount); // initalize and print empty line
					// lineCount doesn't need incremented, previous newLine=1 will trigger at bottom conditional and take care of it
				}
			}
		}
	} else {
		while (isspace(content) != 0) {
			content = fgetc(in_file);
		}
		// not a white space character
	}
	
	while (isspace(content) != 0) {
		// continue to get characters until a non-white space character is reached
		content = fgetc(in_file);
	} 
	
	// check if character is an alpha
	if (isalpha(content)) {
		while (isalpha(content) || isdigit(content)) {
			add_char(buffer, content, index);
			index++;
			add_char(lineBuffer, content, lineIndex);
			lineIndex++;
			content = fgetc(in_file);
			
		}
		// add null terminating character to end of buffer
		add_char(buffer, '\0', index);
		add_char(lineBuffer, '\0', lineIndex);
		// check if string is a reserved word and has a token number or is a literal with the ID token value
		token = check_reserved(buffer);
		ungetc(content, in_file); // put character back for next token
		
		// if not an alpha, check if the char is a minus
	} else if (content == '-') {
		add_char(buffer, content, index);
		index++;
		add_char(lineBuffer, content, lineIndex);
		lineIndex++;
		content = fgetc(in_file);
		if (content == '-') {
			// if second - sign, it is a comment. consume all characters until end of line or end of file
			add_char(buffer, content, index);
			index++;
			add_char(lineBuffer, content, lineIndex);
			lineIndex++;
			content = fgetc(in_file);
			
			while (content != EOF && content != '\n') {
				add_char(buffer, content, index);
				index++;
				add_char(lineBuffer, content, lineIndex);
				lineIndex++;
				content = fgetc(in_file);
			}
			ungetc(content, in_file); // put character back for next token
			// add null terminating character to end of buffer
			add_char(buffer, '\0', index);
			add_char(lineBuffer, '\0', lineIndex);
			commentFlag = 1;
		} else if (isdigit(content)) {
			// negative number intliteral
			
			add_char(buffer, content, index); // add minus sign to buffer
			index++;
			add_char(lineBuffer, content, lineIndex);
			lineIndex++;
			content = fgetc(in_file); // get first (already confirmed to exist) digit
			while (isdigit(content)) {
				add_char(buffer, content, index);
				index++;
				add_char(lineBuffer, content, lineIndex);
				lineIndex++;
				content = fgetc(in_file);
			}
			ungetc(content, in_file); // put character back for next token
			// add null terminating character to end of buffer
			add_char(buffer, '\0', index);
			add_char(lineBuffer, '\0', lineIndex);
			strcpy(intBuffer, buffer);
			intFlag = 1;
			token = check_reserved("integer");
		} else {
			// second character is not a second - or a digit
			ungetc(content, in_file); // put character back, token is a MINUSOP
			// add null terminating character to end of buffer
			add_char(buffer, '\0', index);
			add_char(lineBuffer, '\0', lineIndex);
			token = check_reserved(buffer);
		}
	} else if (isdigit(content)) {
		// if its a number without a negative sign
		while (isdigit(content)) {
			add_char(buffer, content, index);
			index++;
			add_char(lineBuffer, content, lineIndex);
			lineIndex++;
			content = fgetc(in_file);
			
		}
		ungetc(content, in_file); // put character back for next token
		// add null terminating character to end of buffer
		add_char(buffer, '\0', index);
		add_char(lineBuffer, '\0', lineIndex);
		strcpy(intBuffer, buffer);
		intFlag = 1;
		token = check_reserved("integer");
			
	} else if (content == EOF) {
		// because there might not be an end of line character for EOF
		// add check here for lexical error
		newLine = 2;
		strcpy(buffer, "SCANEOF");
		token = check_reserved(buffer);
		
	} else if (content == ';') {
		add_char(buffer, content, index);
		index++;
		add_char(lineBuffer, content, lineIndex);
		lineIndex++;
		add_char(buffer, '\0', index);
		add_char(lineBuffer, '\0', lineIndex);
		token = check_reserved(buffer);
		
	} else if (content == '<'){
		add_char(buffer, content, index);
		index++;
		add_char(lineBuffer, content, lineIndex);
		lineIndex++;
		content = fgetc(in_file);
		
		
		if (content == '='){
			add_char(buffer, content, index);
			index++;
			add_char(lineBuffer, content, lineIndex);
			lineIndex++;	
		} else if (content == '>') {
			add_char(buffer, content, index);
			index++;
			add_char(lineBuffer, content, lineIndex);
			lineIndex++;
		} else {
			ungetc(content, in_file); // character not needed, so put it back
		}
		add_char(buffer, '\0', index);
		add_char(lineBuffer, '\0', lineIndex);	
		token = check_reserved(buffer);
		
	} else if (content == '>') {
		add_char(buffer, content, index);
		index++;
		add_char(lineBuffer, content, lineIndex);
		lineIndex++;
		content = fgetc(in_file);
		
		
		if (content == '=') {
			add_char(buffer, content, index);
			index++;
			add_char(lineBuffer, content, lineIndex);
			lineIndex++;
		} else {
			ungetc(content, in_file); // character not needed, so put it back
		}
		add_char(buffer, '\0', index);
		add_char(lineBuffer, '\0', lineIndex);	
		token = check_reserved(buffer);
		
	} else if (content == ':') {
		add_char(buffer, content, index);
		index++;
		add_char(lineBuffer, content, lineIndex);
		lineIndex++;
		content = fgetc(in_file);
		
		if (content == '=') {
			add_char(buffer, content, index);
			index++;
			add_char(lineBuffer, content, lineIndex);
			lineIndex++;
		} else {
			ungetc(content, in_file); // character not needed, so put it back
		}
		add_char(buffer, '\0', index);
		add_char(lineBuffer, '\0', lineIndex);	
		token = check_reserved(buffer);
		
	} else if (content == '+') {
		add_char(buffer, content, index);
		index++;
		add_char(lineBuffer, content, lineIndex);
		lineIndex++;
		add_char(buffer, '\0', index);
		add_char(lineBuffer, '\0', lineIndex);
		token = check_reserved(buffer);
		
	} else if (content == '*') {
		add_char(buffer, content, index);
		index++;
		add_char(lineBuffer, content, lineIndex);
		lineIndex++;
		add_char(buffer, '\0', index);
		add_char(lineBuffer, '\0', lineIndex);
		token = check_reserved(buffer);
		
	} else if (content == '/') {
		add_char(buffer, content, index);
		index++;
		add_char(lineBuffer, content, lineIndex);
		lineIndex++;
		add_char(buffer, '\0', index);
		add_char(lineBuffer, '\0', lineIndex);
		token = check_reserved(buffer);
		
	} else if (content == '!') {
		add_char(buffer, content, index);
		index++;
		add_char(lineBuffer, content, lineIndex);
		lineIndex++;
		add_char(buffer, '\0', index);
		add_char(lineBuffer, '\0', lineIndex);
		token = check_reserved(buffer);
		
	} else if (content == '=') {
		add_char(buffer, content, index);
		index++;
		add_char(lineBuffer, content, lineIndex);
		lineIndex++;
		add_char(buffer, '\0', index);
		add_char(lineBuffer, '\0', lineIndex);
		token = check_reserved(buffer);
		
	} else if (content == ',') {
		add_char(buffer, content, index);
		index++;
		add_char(lineBuffer, content, lineIndex);
		lineIndex++;
		add_char(buffer, '\0', index);
		add_char(lineBuffer, '\0', lineIndex);
		token = check_reserved(buffer);
		
	} else if (content == '(') {
		add_char(buffer, content, index);
		index++;
		add_char(lineBuffer, content, lineIndex);
		lineIndex++;
		add_char(buffer, '\0', index);
		add_char(lineBuffer, '\0', lineIndex);
		token = check_reserved(buffer);
		
	} else if (content == ')') {
		add_char(buffer, content, index);
		index++;
		add_char(lineBuffer, content, lineIndex);
		lineIndex++;
		add_char(buffer, '\0', index);
		add_char(lineBuffer, '\0', lineIndex);
		token = check_reserved(buffer);
		
	} else {
		// doesn't start with a character or digit -- lexical error?
		add_char(buffer, content, index);
		index++;
		add_char(lineBuffer, content, lineIndex);
		lineIndex++;
		add_char(buffer, '\0', index);
		add_char(lineBuffer, '\0', lineIndex);
		token = check_reserved("lexical error");
		strcpy(errorArr[errorArrIdx], buffer);
	}
	
	if (print == 1) {
		// for listing file output
		if (newLine == 1) {
			// new line
			for (i = 0; i < errorArrIdx; i++) {
				fprintf(list_file, "\n\tLexical Error on Line %d: %s not recognized.\n", lineCount - 1, errorArr[i]);
			}
			for (i = 0; i < synErrorIdx; i++) {
				fprintf(list_file, "\n\tSyntax Error on Line %d: ", lineCount - 1);
				if (strlen(synArr[i]) >= 8) {
					fprintf(list_file, "Expected Token: %s\t", synArr[i]);
				} else {
					fprintf(list_file, "Expected Token: %s\t\t", synArr[i]);
				}
				fprintf(list_file, "Actual Token: %s\n", synArr2[i]);
			}
			errorArrIdx = 0; // reset array index for next line
			synErrorIdx = 0; 
			
			fputc('\n', list_file); // push listing file to new line
			fprintf(list_file, "%d.\t", lineCount); // initialize line
			fprintf(list_file, lineBuffer); // print linebuffer
			
		} else if (newLine == 2) {
			// end of file
			if (endFlag == 0 && scanFlag == 0) {
				lineCount++;
				fprintf(list_file, "\n%d.\t", lineCount);
			}
			endFlag = 0; // reset flag so that if SCANEOF was read before it was expected that an extra line is not generated
			scanFlag = 2;
		
		} else {
			// otherwise just print the linebuffer
			if (content == '%') { // edge case for % sign
				add_char(lineBuffer, content, lineIndex);
				lineIndex++;
				add_char(lineBuffer, '\0', lineIndex);
			}
			fprintf(list_file, lineBuffer);
		}
	} 
	
	
	return token;
}

// returns the token of the string
Token check_reserved(char *buffer) {
	Token retToken;
	int i;
	
	// copy to temp buffer and convert to uppercase for comparisons to make case insensitive
	strcpy(tempBuffer, buffer);
	for (i = 0; i < strlen(buffer); i++) {
		tempBuffer[i] = toupper(tempBuffer[i]);
	}
	
	// string from scanner -- check if it is a reserved word, literal, or ID, otherwise it is an error
	if (strcmp(tempBuffer, "BEGIN") == 0) {
		retToken = BEGIN;
	} else if (strcmp(tempBuffer, "-") == 0) {
		retToken = MINUSOP;
	} else if (strcmp(tempBuffer, "INTEGER") == 0 && intFlag == 1) {
		retToken = INTLITERAL;
	} else if (strcmp(tempBuffer, "<=") == 0) {
		retToken = LESSEQUALOP;
	} else if (strcmp(tempBuffer, "<>") == 0) {
		retToken = NOTEQUALOP;
	} else if (strcmp(tempBuffer, "<") == 0) {
		retToken = LESSOP;
	} else if (strcmp(tempBuffer, ";") == 0) {
		retToken = SEMICOLON;
	} else if (strcmp(tempBuffer, "END") == 0) {
		retToken = END;
	} else if (strcmp(tempBuffer, "LEXICAL ERROR") == 0) {
		retToken = ERROR;
	} else if (strcmp(tempBuffer, "SCANEOF") == 0){
		retToken = SCANEOF;
	} else if (strcmp(tempBuffer, ":=") == 0) {
		retToken = ASSIGNOP;
	} else if (strcmp(tempBuffer, "+") == 0) {
		retToken = PLUSOP;
	} else if (strcmp(tempBuffer, "*") == 0) {
		retToken = MULTOP;
	} else if (strcmp(tempBuffer, "/") == 0) {
		retToken = DIVOP;
	} else if (strcmp(tempBuffer, "!") == 0) {
		retToken = NOTOP;
	} else if (strcmp(tempBuffer, "=") == 0) {
		retToken = EQUALOP;
	} else if (strcmp(tempBuffer, ">") == 0) {
		retToken = GREATEROP;
	} else if (strcmp(tempBuffer, ">=") == 0) {
		retToken = GREATEREQUALOP;
	} else if (strcmp(tempBuffer, ",") == 0) {
		retToken = COMMA;
	} else if (strcmp(tempBuffer, "(") == 0) {
		retToken = LPAREN;
	} else if (strcmp(tempBuffer, ")") == 0) {
		retToken = RPAREN;
	} else if (strcmp(tempBuffer, "READ") == 0) {
		retToken = READ;
	} else if (strcmp(tempBuffer, "WRITE") == 0) {
		retToken = WRITE;
	} else if (strcmp(tempBuffer, "IF") == 0) {
		retToken = IF;
	} else if (strcmp(tempBuffer, "THEN") == 0) {
		retToken = THEN;
	} else if (strcmp(tempBuffer, "ELSE") == 0) {
		retToken = ELSE;
	} else if (strcmp(tempBuffer, "ENDIF") == 0) {
		retToken = ENDIF;
	} else if (strcmp(tempBuffer, "WHILE") == 0) {
		retToken = WHILE;
	} else if (strcmp(tempBuffer, "ENDWHILE") == 0) {
		retToken = ENDWHILE;
	} else if (strcmp(tempBuffer, "FALSE") == 0) {
		retToken = FALSEOP;
	} else if (strcmp(tempBuffer, "TRUE") == 0) {
		retToken = TRUEOP;
	} else if (strcmp(tempBuffer, "NULL") == 0) {
		retToken = NULLOP;
	} else {
		// is a variable
		retToken = ID;
	}
	return retToken;
}

// identifies the string for the token
char* token_ident(Token token, char *tokenChar) {
	char *tokenStr;
	
	if (token == BEGIN) {
		tokenStr = "BEGIN";
	} else if (token == INTLITERAL) {
		tokenStr = "INTLITERAL";
	} else if (token == ID) {
		tokenStr = "ID";
	} else if (token == MINUSOP) {
		tokenStr = "MINUSOP";
	} else if (token == ASSIGNOP) {
		tokenStr = "ASSIGNOP";
	} else if (token == SEMICOLON) {
		tokenStr = "SEMICOLON";
	} else if (token == END) {
		tokenStr = "END";
	} else if (token == SCANEOF) {
		tokenStr = "SCANEOF";
	} else if (token == LESSEQUALOP) {
		tokenStr = "LESSEQUALOP";
	} else if (token == LESSOP) {
		tokenStr = "LESSOP";
	}  else if (token == NOTEQUALOP) {
		tokenStr = "NOTEQUALOP";
	} else if (token == ASSIGNOP) {
		tokenStr = "ASSIGNOP";
	} else if (token == PLUSOP) {
		tokenStr = "PLUSOP";
	} else if (token == MULTOP) {
		tokenStr = "MULTOP";
	} else if (token == DIVOP) {
		tokenStr = "DIVOP";
	} else if (token == NOTOP) {
		tokenStr = "NOTOP";
	} else if (token == EQUALOP) {
		tokenStr = "EQUALOP";
	} else if (token == GREATEROP) {
		tokenStr = "GREATEROP";
	} else if (token == GREATEREQUALOP) {
		tokenStr = "GREATEREQUALOP";
	} else if (token == COMMA) {
		tokenStr = "COMMA";
	} else if (token == LPAREN) {
		tokenStr = "LPAREN";
	} else if (token == RPAREN) {
		tokenStr = "RPAREN";
	} else if (token == READ) {
		tokenStr = "READ";
	} else if (token == WRITE) {
		tokenStr = "WRITE";
	} else if (token == IF) {
		tokenStr = "IF";
	} else if (token == THEN) {
		tokenStr = "THEN";
	} else if (token == ELSE) {
		tokenStr = "ELSE";
	} else if (token == ENDIF) {
		tokenStr = "ENDIF";
	} else if (token == WHILE) {
		tokenStr = "WHILE";
	} else if (token == ENDWHILE) {
		tokenStr = "ENDWHILE";
	} else if (token == FALSEOP) {
		tokenStr = "FALSEOP";
	} else if (token == TRUEOP) {
		tokenStr = "TRUEOP";
	} else if (token == NULLOP) {
		tokenStr = "NULLOP";
	} else {
		tokenStr = "ERROR";
	}
	return tokenStr;
}


// add a character to the buffer
void add_char(char *buffer, char c, int index) {
	buffer[index] = c;
}

//  clears the buffer
void clear_buffer(char *buffer)
{
	memset(buffer, '\0', CHAR_LIMIT * 2);
	
	
}
