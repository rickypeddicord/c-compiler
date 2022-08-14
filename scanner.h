#ifndef SCANNER_H_
#define SCANNER_H_


typedef enum Token
{
	BEGIN,
	END,
	READ,
	WRITE,
	IF,
	THEN,
	ELSE,
	ENDIF,
	WHILE,
	ENDWHILE,
	ID,
	INTLITERAL,
	FALSEOP,
	TRUEOP,
	NULLOP,
	LPAREN,
	RPAREN,
	SEMICOLON,
	COMMA,
	ASSIGNOP,
	PLUSOP,
	MINUSOP,
	MULTOP,
	DIVOP,
	NOTOP,
	LESSOP,
	LESSEQUALOP,
	GREATEROP,
	GREATEREQUALOP,
	EQUALOP,
	NOTEQUALOP,
	SCANEOF,
	ERROR
} Token;

char * getLineBuffer();
int getNewLine();
Token scanner(char *buffer, FILE *in_file, FILE *out_file, FILE *list_file, int print);
void clear_buffer(char *);
Token check_reserved(char *);
int lexical_error(char *, int flag, FILE *);
char* token_ident(Token, char *);
void add_char(char *, char, int);
int isComment();

#endif
