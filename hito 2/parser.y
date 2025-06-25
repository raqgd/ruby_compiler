%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "sym_tab.h"
#include "mips_generator.h"

int yylex(void);
void yyerror(const char *s);

AST* root;

int label_count = 0;
%}

/* ------------------ TIPOS DE DATOS ------------------ */
%union {
    int integer;
    int boolean;
    char* str;
    float flt;
    AST* node;
}

/* ------------------ TOKENS CON TIPO ------------------ */
%token <integer> NUM
%token <boolean> BOOL
%token <str> STRING ID
%token <flt> FLOAT
%token IF ELSE WHILE DEF RETURN
%token THEN DO
%token END
%token EQ NEQ LT GT LE GE
%token PLUS MINUS MUL DIV
%token ASSIGN
%token NEWLINE

/* ------------------ TIPOS PARA NO TERMINALES ------------------ */
%type <node> program stmts stmt expr
%type <str> identifier

/* ------------------ PRECEDENCIA DE OPERADORES ------------------ */
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%left ASSIGN
%left EQ NEQ LT GT LE GE
%left PLUS MINUS
%left MUL DIV

%%

program:
    stmts {
        root = $1;
    }
;

stmts:
    /* vacío */ { $$ = NULL; }
  | stmts stmt { $$ = ast_append_stmt($1, $2); }
  | stmt       { $$ = $1; }
;

stmt:
    identifier ASSIGN expr NEWLINE {
        if (!symbol_exists($1)) {
            symtable_declare($1, $3->type);
        }
        $$ = ast_create_assign_node($1, $3);
        free($1);
    }
  | expr NEWLINE {
        $$ = ast_create_expr_stmt($1);
    }
  | IF expr THEN stmts ELSE stmts END {
        $$ = ast_create_if_node($2, $4, $6);
    }
  | IF expr THEN stmts END %prec LOWER_THAN_ELSE {
        $$ = ast_create_if_node($2, $4, NULL);
    }
  | WHILE expr DO stmts END {
        $$ = ast_create_while_node($2, $4);
    }
;

expr:
    NUM              { $$ = ast_create_num_node($1); }
  | BOOL             { $$ = ast_create_bool_node($1); }
  | STRING           { $$ = ast_create_str_node($1); }
  | identifier       { $$ = ast_create_id_node($1); free($1); }
  | expr PLUS expr   { $$ = ast_create_binop_node(PLUS, $1, $3); }
  | expr MINUS expr  { $$ = ast_create_binop_node(MINUS, $1, $3); }
  | expr MUL expr    { $$ = ast_create_binop_node(MUL, $1, $3); }
  | expr DIV expr    {
                        if (is_zero($3)) {
                            yyerror("División por cero");
                            YYABORT;
                        }
                        $$ = ast_create_binop_node(DIV, $1, $3);
                    }
  | expr EQ expr     { $$ = ast_create_binop_node(EQ, $1, $3); }
  | expr NEQ expr    { $$ = ast_create_binop_node(NEQ, $1, $3); }
  | expr LT expr     { $$ = ast_create_binop_node(LT, $1, $3); }
  | expr GT expr     { $$ = ast_create_binop_node(GT, $1, $3); }
  | expr LE expr     { $$ = ast_create_binop_node(LE, $1, $3); }
  | expr GE expr     { $$ = ast_create_binop_node(GE, $1, $3); }
;

identifier:
    ID { $$ = $1; }
;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main() {
    symtable_init();

    if (yyparse() == 0) {
        generate_mips_from_ast(root);
    }

    symtable_destroy();
    ast_destroy(root);

    return 0;
}