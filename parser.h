#ifndef PARSER_H_
#define PARSER_H_

#include "scanner.h"

typedef enum expr_type {
	IDEXPR,
	LITERALEXPR,
	TEMPEXPR
} expr_type;

typedef struct expr_record {
	char data[200];
	expr_type kind;
} expr_record;

typedef struct op_record {
	char oper[10];
} op_record;

int lookup(char *string);
void enter(char *string);
void check_id(char *string);
void generate(char *s1, char *s2, char *s3, char *s4, char *s5, char *s6, char *s7, char *s8);
char *get_temp();
void action_start();
void action_assign(expr_record target, expr_record source);
void action_read_id(expr_record record);
void action_write_expr(expr_record record);
void action_if_statement(expr_record record);
void action_else_statement();
void action_end_statement();
void action_while_statement(expr_record record);
expr_record action_process_id();
expr_record action_gen_infix(expr_record left_operand, op_record op, expr_record right_operand);
expr_record action_process_literal();
op_record action_process_op();
void action_finish();
void add_op(op_record *op);
void mult_op(op_record *op);
void rel_op(op_record *op);
int system_goal(char *buffer, FILE *in_file, FILE *out_file, FILE *list_file, FILE *temp_file);
int match(Token);
Token next_token();
int program();
int statement_list();
int statement();
expr_record ident();
int id_list();
int IFTail();
int expr_list();
int expression(expr_record *result);
int term(expr_record *operand);
int factor(expr_record *operand);
expr_record condition();
int addition(expr_record *operand);
int multiplication(expr_record *operand);
int unary(expr_record *operand);
int lprimary(expr_record *operand);




#endif

