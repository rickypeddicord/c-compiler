#include "file_util.h"
#include "parser.h"
#include "scanner.h"
#include <string.h>
#include <ctype.h>
#include <time.h>

// global variables
FILE *inputFile;
FILE *outputFile;
FILE *listFile;
FILE *tempFile;
char buffer[CHAR_LIMIT * 2];
char parseBuffer[CHAR_LIMIT * 2];
char tokenChar[50];
char commentBuffer[CHAR_LIMIT * 2];
int nextflag;
int errorflag;
int comflag = 0;
int endFlag = 0;
int lexFlag = 0;
int scanFlag = 0;
int flag = 0;
int idFlag = 0;
extern int count;
extern int lineCount;
extern char errorArr[CHAR_LIMIT * 2][CHAR_LIMIT * 2];
int synErrorIdx = 0;
int synErrorCount = 0;
char synArr[CHAR_LIMIT * 2][CHAR_LIMIT * 2];
char synArr2[CHAR_LIMIT * 2][CHAR_LIMIT * 2];
extern int errorArrIdx;
extern int errorCount;
extern int newLine;
extern char lineBuffer[CHAR_LIMIT * 2];
extern char tempBuffer[CHAR_LIMIT * 2];
extern char intBuffer[CHAR_LIMIT * 2];

char symbolTable[CHAR_LIMIT * 2][CHAR_LIMIT * 2];
int symbolIndex = 0;
int tempCount = 1;
int tabCounter = 0;



int lookup(char *string) {
	// 1 true
	// 0 false
	
	// return a logical depending on if the string exists in the symbol table
	int i;
	int logical = 0;
	
	for (i = 0; i < symbolIndex; i++) {
		if (strcmp(string, symbolTable[i]) == 0) {
			logical = 1;
		}
	}
	return logical;
}

void enter(char *string) {
	// add string to symbol table
	strcpy(symbolTable[symbolIndex], string);
	symbolIndex++;
}

void check_id(char *string) {
	// check if the string exists in the symbol table
	// if it doesn't add it to the symbol table and print it to the output file
	
	if (lookup(string) == 0) {
		enter(string);
		fprintf(outputFile, "%s %s%c", "\tint", string, ';');
		
		if (comflag) {
			// if a ID declaration contains a comment, include it
			fprintf(outputFile, "%s", commentBuffer);
			comflag = 0;
		}
		fprintf(outputFile, "\n");
	}
}

void generate(char *s1, char *s2, char *s3, char *s4, char *s5, char *s6, char *s7, char *s8) {
	// combine multiple strings to build and print a statement to the temp file
	char outputStr[500];
	char tabStr[50];
	int i;
	
	// calculate if tabs are needed to make code blocks look neat
	strcpy(tabStr, "");
	sprintf(outputStr, "%s%s%s%s%s%s%s%s", s1, s2, s3, s4, s5, s6, s7, s8);
	if (tabCounter > 0) {
		for (i = 0; i < tabCounter; i++) {
			strcat(tabStr, "\t");
		}
		fprintf(tempFile, "%s", tabStr);
		fprintf(tempFile, "%s", outputStr);
	} else {
		fprintf(tempFile, "%s", outputStr);
	}
	
	if (comflag) {
		// if a line has a comment, include it
		fprintf(tempFile, "%s", commentBuffer);
		comflag = 0;
	}
	fprintf(tempFile, "\n");
}

char *get_temp() {
	// create a new temp variable
	char tempStr[50];
	char *temp_ptr = tempStr;
	sprintf(tempStr, "%s%d", "Temp", tempCount);
	tempCount++;
	check_id(tempStr);
	
	return temp_ptr;
}

void action_start() {
	// initailize output C file with heading
	time_t t;
	time(&t);
	fprintf(outputFile, "// Current Date and Time:\n");
	fprintf(outputFile, "// %s\n", ctime(&t));
	fprintf(outputFile, "\n");
	fprintf(outputFile, "#include <stdio.h>\n");
	fprintf(outputFile, "\n");
	fprintf(outputFile, "int main()\n");
	fprintf(outputFile, "{\n");
	tabCounter++;
	fprintf(outputFile, "\n");
}

void action_assign(expr_record target, expr_record source) {
	// will receieve two expression records one for the left hand side and one for the right hand side of the assignment.
	// it will call generate passing the two contents of the expression records along with the '=' so that a correct C assignment is created
	generate(target.data, " = ", source.data, ";", "", "", "", "");
}

void action_read_id(expr_record record) {
	// will receieve an expression record and generate a scanf statement
	generate("scanf(\"%d\", &", record.data, ")", ";", "", "", "", "");
}

void action_write_expr(expr_record record) {
	// will receieve an expression record and generate a printf statement
	generate("printf(\"%d\\n\", ", record.data, ")", ";", "", "", "", "");
}

void action_if_statement(expr_record record) {
	// generate if statement with expression record as the condition
	generate("if", " (", record.data, ")", "", "", "", "");
	generate("{", "", "", "", "", "", "", "");
}

void action_else_statement() {
	// generate else statement for the preceeding if statement
	generate("}", "", "", "", "", "", "", "");
	generate("else", "", "", "", "", "", "", "");
	generate("{", "", "", "", "", "", "", "");
}

void action_end_statement() {
	// generate closing bracket for either the end of an if statement or end of while statement
	generate("}", "", "", "", "", "", "", "");
}

void action_while_statement(expr_record record) {
	// generate while statement with expression record as the condition
	generate("while", " (", record.data, ")", "", "", "", "");
	generate("{", "", "", "", "", "", "", "");
}


expr_record action_gen_infix(expr_record left_operand, op_record op, expr_record right_operand) {
	// will accept two expression records. one for the left hand side and one for the right hand side as well as an operation record
	// it creates a third expression record and sets it's kind to TEMPEXPR
	// it sets it's string to a new TempID by using gettemp()
	// it generates C code with generate for the assignment of the left expression operation right expression to the Temp
	// it then returns the Temp (3rd) expression record
	expr_record expr;
	expr.kind = TEMPEXPR;
	strcpy(expr.data, get_temp());
	
	generate(expr.data, " = ", left_operand.data, " ", op.oper, " ", right_operand.data, ";");
	
	return expr;
	
}

expr_record action_process_id() {
	// creates an expression record and sets it's kind to IDEXPR
	// it sets it's string to the contents of the token buffer which is the ID
	// it then returns the ID expression record
	expr_record temp;
	check_id(buffer);
	temp.kind = IDEXPR;
	strcpy(temp.data, buffer);
	
	return temp;
}

expr_record action_process_literal() {
	// creates an expression record and sets it's kind to LITERALEXPR
	// sets it's string to the contents of the token buffer which is the number
	
	char tempBuffer[CHAR_LIMIT * 2];
	int i;
	
	expr_record temp;
	temp.kind = LITERALEXPR;
	
	strcpy(tempBuffer, buffer);
	for (i = 0; i < strlen(tempBuffer); i++) {
		tempBuffer[i] = toupper(tempBuffer[i]);
	}
	
	if (strcmp(tempBuffer, "TRUE") == 0) {
		strcpy(temp.data, "1");
	} else if (strcmp(tempBuffer, "FALSE") == 0) {
		strcpy(temp.data, "0");
	} else if (strcmp(tempBuffer, "NULL") == 0) {
		strcpy(temp.data, "0");
	} else {
		strcpy(temp.data, buffer);
	}
	
	return temp;
}

op_record action_process_op() {
	// creates an operation record
	// it sets it's string to the type of operation specified
	op_record o;
	char tempBuffer[CHAR_LIMIT * 2];
	int i;
	
	strcpy(tempBuffer, buffer);
	for (i = 0; i < strlen(tempBuffer); i++) {
		tempBuffer[i] = toupper(tempBuffer[i]);
	}
	
	if (strcmp(tempBuffer, "TRUE") == 0) {
		strcpy(o.oper, "1");
	} else if (strcmp(tempBuffer, "FALSE") == 0) {
		strcpy(o.oper, "0");
	} else if (strcmp(tempBuffer, "NULL") == 0) {
		strcpy(o.oper, "0");
	} else if (strcmp(tempBuffer, "<>") == 0) {
		strcpy(o.oper, "!=");
	} else {
		strcpy(o.oper, buffer);
	}
	
	return o;
}

void action_finish() {
	// write a descriptive closing to the listing and output files
	// concatenates the Output and Temp files together
	char c;
	char content;
	
	fprintf(tempFile, "\n");
	fprintf(tempFile, "return 0;\n");
	tabCounter--;
	fprintf(tempFile, "}\n\n");
	if (errorCount > 0 || synErrorCount > 0) {
		fprintf(tempFile, "// Program Compiled with %d Errors.", errorCount + synErrorCount);
	} else {
		fprintf(tempFile, "// Program Compiled with No Errors.");
	}
	fprintf(outputFile, "\n");
	
	// append temp file to end of output file
	rewind(tempFile);
	c = fgetc(tempFile);
	while (c != EOF) {
		fputc(c, outputFile);
		c = fgetc(tempFile);
	}
	wrapUp();
}

void add_op(op_record *op) {
	// process add op
	*op = action_process_op();
}

void mult_op(op_record *op) {
	// process mult op
	*op = action_process_op();
}

void rel_op(op_record *op) {
	// process rel op
	*op = action_process_op();
}


// system_goal, the first production
int system_goal(char *buffer, FILE *in_file, FILE *out_file, FILE *list_file, FILE *temp_file) {
	int bool = 1;
	buffer = buffer;
	inputFile = in_file;
	outputFile = out_file;
	listFile = list_file;
	tempFile = temp_file;
	
	if (program()) { // if program() returns error-free
		bool = 1;
	} else { // otherwise error
		bool = 0;
	}
	
	if (match(SCANEOF) == 0) { // keep matching until we hit the end of the file
		while (match(SCANEOF) == 0);
	}
	action_finish();
	return bool;
	
	
}

// next_token calls the scanner for a token (non-destructive), it returns the next token but the file pointer remains unchanged
// next_token is used by the parser to determine which path to follow when a production has multiple RHS's
// Now, what do we do if next_token returns a lexical error?
Token next_token (){
	Token next;
	int count = 0;
	int i;
	char temp;
	
	int size;
	
	size = ftell(inputFile);
	next = scanner(buffer, inputFile, outputFile, listFile, 0);
	
	
	temp = fgetc(inputFile); // grab next character
	
	// make non-destructive if EOF character is right after END token
	if (next == 1 && temp == EOF) {
			ungetc(temp, inputFile); // put back EOF character
			fseek(inputFile, -3, SEEK_CUR); // seek to before END
		
	} else {
		// make every other scenario non-destructive
		ungetc(temp, inputFile); // put back temp character
		for (i = 0; i < strlen(lineBuffer); i++) {
    	count++;
	}
		
	for (i = count - 1; i >= 0; i--) {
		
		ungetc(lineBuffer[i], inputFile);
		
		} 
	}
	
	if (getNewLine() == 1) {
		// if it is a new line, put the newline character back
		ungetc('\n', inputFile);
	}
	

	return next;
}

// Match receives a token and calls the scanner for a token (destructive), it returns true or false
// True is returned if the token sent and the token from the scanner match
// False is returned if the token sent and the token from the scanner do not match
// Additionally, a syntax error message is printed when the tokens do not match
int match(Token t){
	int match = 0;
	Token check;
	char temp;
	char copyBuffer[CHAR_LIMIT * 2];
	int i;
	lexFlag = 0;
	
	check = scanner(buffer, inputFile, outputFile, listFile, 1);
	
	if (isComment()) {
		// copy comment to commentBuffer
		comflag = 1;
		strcpy(commentBuffer, " // ");
		strcat(commentBuffer, buffer + 2);
	}
	
	// if the tokens match or a comment is detected
	if (t == check || isComment() && t == BEGIN) {
		match = 1;
		
		// if there is not a comment
		if (isComment() == 0) {
			if (newLine == 1) {
				if (parseBuffer[0] == 'I' && parseBuffer[1] == 'F' && parseBuffer[strlen(parseBuffer) - 2] == ')') {
					strcat(parseBuffer, "\n"); // IF Statement without a "THEN" on the same line needs a newline char appended
				}
				
				//fprintf(outputFile, "\nStatement: %s\n", parseBuffer); // print buffer
				clear_buffer(parseBuffer);
				count = 0; // reset count of newline characters for scanner (used for consistent output file formatting)
			} else if (newLine == 2) {
				// end-of-file scenario
				//fprintf(outputFile, "\nStatement: %s\n\n", parseBuffer); // print buffer
				clear_buffer(parseBuffer);
				
				
				// Errors only need to be printed here because its impossible for a statement or EOF to finish with a token that results in an error
				for (i = 0; i < errorArrIdx; i++) {
					fprintf(listFile, "\n\tLexical Error on Line %d: %s not recognized.\n", lineCount - 1, errorArr[i]);
			}
				for (i = 0; i < synErrorIdx; i++) {
					fprintf(listFile, "\n\tSyntax Error on Line %d: ", lineCount - 1);
					if (strlen(synArr[i]) >= 8) {
						fprintf(listFile, "Expected Token: %s\t", synArr[i]);
					} else {
						fprintf(listFile, "Expected Token: %s\t\t", synArr[i]);
					}
					fprintf(listFile, "Actual Token: %s\n", synArr2[i]);
				}
			errorArrIdx = 0; // reset indexes for next line
			synErrorIdx = 0; // reset indexes for next line
			
			if (endFlag == 0 && scanFlag == 0 && strcmp(tempBuffer, "SCANEOF")) {
				// handle edge case for printing last line
				fprintf(listFile, "\n%d.\t", lineCount);
			}
			
			if (flag) {
				// handle other edge case for printing last line
				fprintf(listFile, "\n%d.\t", lineCount);
			}

			// print total file errors
			if (errorCount == 0 && synErrorCount == 0) {
				fprintf(listFile, "\n\nInput File Compiled without Errors\n");
			} else {
				fprintf(listFile, "\n\nInput File Compiled with %d Errors\n", errorCount + synErrorCount);
			}
			fprintf(listFile, "Total Numer of Lexical Errors: %d\t", errorCount);
			fprintf(listFile, "Total Number of Syntax Errors: %d\n", synErrorCount);
				
			} 
			
			// print expected and actual tokens
			strcpy(tokenChar, token_ident(t, tokenChar));
			
			/*if (strlen(tokenChar) >= 8) {
				fprintf(outputFile, "Expected Token: %s\t", tokenChar);
			} else {
				fprintf(outputFile, "Expected Token: %s\t\t", tokenChar);
			}
			
			if (check == INTLITERAL) {
				fprintf(outputFile, "Actual Token: %s\n", intBuffer);
			} else {
				fprintf(outputFile, "Actual Token: %s\n", tempBuffer);
			}
			
			if (strcmp(tempBuffer, "SCANEOF") == 0) {
				// print final statement when SCANEOF is reached
				fprintf(outputFile, "\nStatement: %s\n", tempBuffer);
			}*/
			
			
			// uppercase the buffer before we print it
			strcat(parseBuffer, buffer);
			for (i = 0; i < strlen(parseBuffer); i++) {
				parseBuffer[i] = toupper(parseBuffer[i]);
			}
		} 
			
	} else {
		// if the tokens do not match
		match = 0;
		
		// fill syntax error arrays
		strcpy(tokenChar, token_ident(t, tokenChar));
		strcpy(synArr[synErrorIdx], tokenChar);
		if (check == INTLITERAL) {
			strcpy(synArr2[synErrorIdx], intBuffer);
		} else {
			strcpy(synArr2[synErrorIdx], tempBuffer);
		}
		synErrorIdx++;
		synErrorCount++;
		
		// if it is not a comment
		if (isComment() == 0) {
			if (newLine == 1) {
				
				if (parseBuffer[0] == 'I' && parseBuffer[1] == 'F' && parseBuffer[strlen(parseBuffer) - 2] == ')') {
					strcat(parseBuffer, "\n"); // IF Statement without a "THEN" on the same line needs a newline char appended
				}
				//fprintf(outputFile, "\nStatement: %s\n", parseBuffer); // print buffer
				clear_buffer(parseBuffer);
				count = 0; // reset count of newline characters for scanner (used for consistent output file formatting)
			} else if (newLine == 2) {
				// end-of-file scenario
				//fprintf(outputFile, "\nStatement: %s\n\n", parseBuffer); // print buffer
				clear_buffer(parseBuffer);
					
			} 
			// print syntax error and expected and actual token to output file
			//fprintf(outputFile, "Syntax Error:\n");
			/*if (strlen(tokenChar) >= 8) {
				fprintf(outputFile, "Expected Token: %s\t", tokenChar);
			} else {
				fprintf(outputFile, "Expected Token: %s\t\t", tokenChar);
			}
			
			if (check == INTLITERAL) {
				fprintf(outputFile, "Actual Token: %s\n\n", intBuffer);
			} else {
				fprintf(outputFile, "Actual Token: %s\n\n", tempBuffer);
			}*/
			
			if (strcmp(tempBuffer, "SCANEOF") == 0 && strcmp(tokenChar, "END") == 0) {
				// flags for edge case
				endFlag = 1;
				lineCount++;
				scanFlag = 1;
			}
			if (strcmp(tempBuffer, "END") == 0 && strcmp(tokenChar, "SCANEOF") == 0) {
				// flags for other edge case
				endFlag = 1;
				flag = 1;
				lineCount++;
			}
			
			// rebuild statement if it contains a lexical error
			if (strcmp(tempBuffer, "LEXICAL ERROR") == 0) {
				errorArrIdx++;
				errorCount++;
				strcpy(copyBuffer, " (");
				strcat(copyBuffer, buffer);
				strcat(copyBuffer, ") LEXICAL ERROR ");
				strcpy(buffer, copyBuffer);
			}
			
			// uppercase the entire buffer before printing
			strcat(parseBuffer, buffer);
			for (i = 0; i < strlen(parseBuffer); i++) {
				parseBuffer[i] = toupper(parseBuffer[i]);
			}
		} 
		
	}
	
	return match;
			
	
}


int program(){
	int progFlag = 1;
	action_start();
	// main program productions
	if (match(BEGIN) == 0) progFlag = 0;
	if (statement_list() == 0) progFlag = 0;
	if (match(END) == 0) progFlag = 0;
	
	return progFlag;
	
}

int statement_list(){
	nextflag = 1; // flag to determine if we should continue processing statements
	errorflag = 1; // no error is 1, error is 0
	
	do {
		statement();	// keep running production while there are statements to process
	} while (nextflag);
	
	return errorflag;
}

expr_record ident() {
	expr_record t;
	idFlag = 1;
	if (match(ID) == 0) idFlag = 0;
	t = action_process_id();
	return t;
} 

int statement(){
	Token next;
	expr_record target;
	expr_record source;
	errorflag = 1;
	int loopflag = 1;
	
	next=next_token();
	
	switch (next){
	case 10: // ID
		target = ident();
		match(ASSIGNOP);
		expression(&source);
		action_assign(target, source);
		nextflag = 1;
		// if there is an error in the sequence keep going until a semicolon is matched or we peek EOF
		if (match(SEMICOLON) == 0) {
			while (match(SEMICOLON) == 0 && loopflag) {
				if (next_token() == SCANEOF) {
					loopflag = 0;
					nextflag = 0;
				}
			}
		}
		break;
	case 2: // READ
		match(READ);
		match(LPAREN);
		id_list();
		match(RPAREN);
		nextflag = 1;
		// if there is an error in the sequence keep going until a semicolon is matched or we peek EOF
		if (match(SEMICOLON) == 0 && loopflag) {
			while (match(SEMICOLON) == 0) {
				if (next_token() == SCANEOF) {
					loopflag = 0;
					nextflag = 0;
				} 
			}
		}
		break;
	case 3: // WRITE
		match(WRITE);
		match(LPAREN);
		expr_list();
		match(RPAREN);
		nextflag = 1;
		// if there is an error in the sequence keep going until a semicolon is matched or we peek EOF
		if (match(SEMICOLON) == 0) {
			while (match(SEMICOLON) == 0 && loopflag) {
				if (next_token() == SCANEOF) {
					loopflag = 0;
					nextflag = 0;
				}
			}
		}
		break;
	case 4: // IF-THEN
		match(IF);
		match(LPAREN);
		target = condition();
		match(RPAREN);
		action_if_statement(target);
		tabCounter++;
		nextflag = 1;
		// if there is an error in the sequence keep going until a semicolon is matched or we peek EOF
		if (match(THEN) == 0) {
			while (match(THEN) == 0 && loopflag) {
				if (next_token() == SCANEOF) {
					loopflag = 0;
					nextflag = 0;
				}
			}
		}
		statement_list();
		nextflag = IFTail();
		break;
	case 8: // WHILE
		match(WHILE);
		match(LPAREN);
		target = condition();
		match(RPAREN);
		action_while_statement(target);
		tabCounter++;
		statement_list();
		nextflag = 1;
		// if there is an error in the sequence keep going until a semicolon is matched or we peek EOF
		if (match(ENDWHILE) == 0) {
			while (match(ENDWHILE) == 0 && loopflag) {
				if (next_token() == SCANEOF) {
					loopflag = 0;
					nextflag = 0;
				}
			}
		}
		tabCounter--;
		action_end_statement();
		break;
	default:
		// Handle comments
		if (isComment()) {
			// as comments are not a token it will default to the first available token which is BEGIN
			match(BEGIN);
			break;
		}
		errorflag = 0;
		nextflag = 0;
	} 
}



int id_list(){
	expr_record id;
	int errorFlag = 1;
	// ID list production
	id = ident();
	if (idFlag == 0) errorFlag = 0;
	action_read_id(id);
	while(next_token() == 18){ // COMMA
		if (match(COMMA) == 0) errorFlag = 0;
		if (id_list() == 0) errorFlag = 0;
	}
	return errorFlag;
	
}

int IFTail() {
	int loopflag = 1;
	// ELSE-ENDIF Production
	if (next_token() == 6) { // ELSE
		match(ELSE);
		tabCounter--;
		action_else_statement();
		tabCounter++;
		statement_list();
	}
	
	// if there is an error in the sequence keep going until a semicolon is matched or we peek EOF
	if (match(ENDIF) == 0) {
		while (match(ENDIF) == 0 && loopflag) {
			if (next_token() == SCANEOF) loopflag = 0;
		}
	}
	tabCounter--;
	action_end_statement();
	return loopflag;
}

int expr_list(){
	expr_record id;
	int errorFlag = 1;
	// expression list production
	if (expression(&id) == 0) errorFlag = 0;
	action_write_expr(id); // pass expression record
	while(next_token() == 18){ // COMMA
		if (match(COMMA) == 0) errorFlag = 0;
		if (expr_list() == 0) errorFlag = 0;
	}
}

int expression (expr_record *result) {
	expr_record left_operand;
	expr_record right_operand;
	op_record op;
	int errorFlag = 1;
	// expression production
	if (term(&left_operand) == 0) errorFlag = 0;
	while(next_token() == 20 || next_token() == 21) { // PLUSOP || MINUSOP
		if (next_token() == 20) {
			match(PLUSOP);
			add_op(&op);
		} 
		if (next_token() == 21) {
			match(MINUSOP);
			add_op(&op);
		}
		if (term(&right_operand) == 0) errorFlag = 0;
		left_operand = action_gen_infix(left_operand, op, right_operand);
	}
	*result = left_operand;
	return errorFlag;	
}

int term (expr_record *operand) {
	expr_record left_operand;
	expr_record right_operand;
	op_record op;
	int errorFlag = 1;
	// term production
	if (factor(&left_operand) == 0) errorFlag = 0;
	while(next_token() == 22 || next_token() == 23) { // MULTOP || DIVOP
		if (next_token() == 22) {
			match(MULTOP);
			mult_op(&op);
		}
		if (next_token() == 23) {
			match(DIVOP);
			mult_op(&op);
		}
		if (factor(&right_operand) == 0) errorFlag = 0;
		left_operand = action_gen_infix(left_operand, op, right_operand);
	}
	*operand = left_operand;
	return errorFlag;
}

int factor(expr_record *operand) {
	Token next = next_token();
	int errorFlag = 1;
	expr_record t;
	// factor production
	switch(next) {
		case 10: // ID
			*operand = ident();
			break;
		case 11: // INTLITERAL
			match(INTLITERAL);
			*operand = action_process_literal();
			break;
		case 21: // MINUSOP
			match(MINUSOP);
			action_process_op(); // not 100% sure if needed here?
			factor(&t);
			*operand = t;
			break;
		case 15: // LPAREN
			match(LPAREN);
			expression(&t);
			*operand = t;
			match(RPAREN);
			break;
		default:
			errorFlag = 0;
		
	}
	return errorFlag;
}


expr_record condition() {
	//int errorFlag = 1;
	// flag 0 for normal, 1 for if, 2 for while
	expr_record left_operand;
	expr_record right_operand;
	op_record op;
	// condition production
	//if (addition(&left_operand) == 0) errorFlag = 0;
	addition(&left_operand);
	while (next_token() == 25 || next_token() == 26 || next_token() == 27 || next_token() == 28 || 
		next_token() == 29 || next_token() == 30) {
			if (next_token() == 25) {
				match(LESSOP);
				rel_op(&op);
			}
			if (next_token() == 26) {
				match(LESSEQUALOP);
				rel_op(&op);
			}
			if (next_token() == 27) {
				match(GREATEROP);
				rel_op(&op);
			}
			if (next_token() == 28) {
				match(GREATEREQUALOP);
				rel_op(&op);
			}
			if (next_token() == 29) {
				match(EQUALOP);
				rel_op(&op);
			}
			if (next_token() == 30) {
				match(NOTEQUALOP);
				rel_op(&op);
			}
			//if (addition(&right_operand) == 0) errorFlag = 0;
			addition(&right_operand);
			left_operand = action_gen_infix(left_operand, op, right_operand);
		}
		return left_operand;
		//return errorFlag;
}

int addition(expr_record *operand) {
	expr_record left_operand;
	expr_record right_operand;
	op_record op;
	int errorFlag = 1;
	// addition production
	if (multiplication(&left_operand) == 0) errorFlag = 0;
	while (next_token() == 20 || next_token() == 21) { // PLUSOP || MINUSOP
		if (next_token() == 20) {
			match(PLUSOP);
			add_op(&op);
		}
		if (next_token() == 21) {
			match(MINUSOP);
			add_op(&op);
		}
		if (multiplication(&right_operand) == 0) errorFlag = 0;
		left_operand = action_gen_infix(left_operand, op, right_operand);
	}
	*operand = left_operand;
	return errorFlag;
}

int multiplication(expr_record *operand) {
	expr_record left_operand;
	expr_record right_operand;
	op_record op;
	int errorFlag = 1;
	// multiplication production
	if (unary(&left_operand) == 0) errorFlag = 0;
	while (next_token() == 22 || next_token() == 23) { // MULTOP || DIVOP
		if (next_token() == 22) {
			match(MULTOP);
			mult_op(&op);
		}
		if (next_token() == 23) {
			match(DIVOP);
			mult_op(&op);
		}
		if (unary(&right_operand) == 0) errorFlag = 0;
		left_operand = action_gen_infix(left_operand, op, right_operand);
	}
	*operand = left_operand;
	return errorFlag;
}

int unary(expr_record *operand) {
	expr_record t;
	char temp[50];
	Token next = next_token();
	int errorFlag = 1;
	// unary production
	switch(next) {
		case 24:
			match(NOTOP);
			if (unary(&t) == 0) errorFlag = 0;
			sprintf(temp, "%s%s", "!", t.data);
			strcpy(t.data, temp);
			*operand = t;
			break;
		case 21:
			match(MINUSOP);
			if (unary(&t) == 0) errorFlag = 0;
			sprintf(temp, "%s%s", "-", t.data);
			strcpy(t.data, temp);
			*operand = t;
			break;

		default:
			if (lprimary(&t) == 0) errorFlag = 0;
			*operand = t;	
	}
	return errorFlag;
}

int lprimary(expr_record *operand) {
	// lprimary production
	Token next = next_token();
	int errorFlag = 1;
	
	switch(next) {
		case 11:
			match(INTLITERAL);
			*operand = action_process_literal();
			break;
		case 10:
			*operand = ident();
			break;
		case 15:
			match(LPAREN);
			*operand = condition();
			match(RPAREN);
			break;
		case 12:
			match(FALSEOP);
			*operand = action_process_literal();
			break;
		case 13:
			match(TRUEOP);
			*operand = action_process_literal();
			break;
		case 14:
			match(NULLOP);
			*operand = action_process_literal();
			break;
		default:
			errorFlag = 0;
	}
	return errorFlag;
}


