%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"           /*---Definiciones del AST y funciones---*/
#include "sym_tab.h"       /*---Tabla de símbolos---*/
#include "mips_generator.h" /*---Generador de código MIPS---*/

int yylex(void);
void yyerror(const char *s);

AST* root;  /* Raíz del Árbol de Sintaxis Abstracta */

int label_count = 0; /*---Contador para generar etiquetas únicas en MIPS---*/
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
%token IF ELSE WHILE PRINT DEF RETURN
%token EQ NEQ LT GT LE GE
%token PLUS MINUS MUL DIV
%token ASSIGN

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
  | stmts stmt {
        $$ = ast_append_stmt($1, $2);
    }
;

stmt:
    IF expr stmt %prec LOWER_THAN_ELSE {
        $$ = ast_create_if_node($2, $3, NULL);
    }
  | IF expr stmt ELSE stmt {
        $$ = ast_create_if_node($2, $3, $5);
    }
  | WHILE expr stmt {
        $$ = ast_create_while_node($2, $3);
    }
  | expr ';' {
        $$ = ast_create_expr_stmt($1);
    }
;

expr:
     NUM {
        $$ = ast_create_num_node($1);
    }
  | BOOL {
        $$ = ast_create_bool_node($1);
    }
  | identifier {
        if (!symbol_exists($1)) {
            yyerror("Variable usada sin declarar");
            YYABORT;
        }
        $$ = ast_create_id_node($1);
        free($1);
    }
  | expr PLUS expr {
        $$ = ast_create_binop_node(PLUS, $1, $3);
    }
  | expr MINUS expr {
        $$ = ast_create_binop_node(MINUS, $1, $3);
    }
  | expr MUL expr {
        $$ = ast_create_binop_node(MUL, $1, $3);
    }
  | expr DIV expr {
        if (is_zero($3)) {
            yyerror("División por cero");
            YYABORT;
        }
        $$ = ast_create_binop_node(DIV, $1, $3);
    }
  | expr EQ expr {
        $$ = ast_create_binop_node(EQ, $1, $3);
    }
  | expr NEQ expr {
        $$ = ast_create_binop_node(NEQ, $1, $3);
    }
  | expr LT expr {
        $$ = ast_create_binop_node(LT, $1, $3);
    }
  | expr GT expr {
        $$ = ast_create_binop_node(GT, $1, $3);
    }
  | expr LE expr {
        $$ = ast_create_binop_node(LE, $1, $3);
    }
  | expr GE expr {
        $$ = ast_create_binop_node(GE, $1, $3);
    }
  | identifier ASSIGN expr {
        if (!symbol_exists($1)) {
            yyerror("Variable usada sin declarar");
            YYABORT;
        }
        $$ = ast_create_assign_node($1, $3);
        free($1);
    }
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