%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mips_generator.h"

int yylex();
void yyerror(const char *s);

int label_count = 0; // para generar etiquetas únicas
%}

/* ------------------ TIPOS DE DATOS ------------------ */
%union {
    int integer;
    int boolean;
    char* str;
}

/* ------------------ TOKENS CON TIPO ------------------ */
%token <integer> NUM
%token <boolean> BOOL
%token <str> ID
%token IF ELSE WHILE
%token EQ NEQ LT GT LE GE
%token PLUS MINUS MUL DIV
%token ASSIGN

/* ------------------ TIPOS PARA NO TERMINALES ------------------ */
%type <integer> expr
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
    /* vacío */
  | program stmt '\n'
  | program stmt
  ;

stmt:
    IF expr stmt %prec LOWER_THAN_ELSE {
        int lbl = label_count++;
        mips_if_start(lbl);
        // stmt ya generó código
        mips_if_end(lbl);
    }
  | IF expr stmt ELSE stmt {
        int lbl = label_count++;
        mips_if_else_start(lbl);
        // stmt ya generó código
        mips_if_else_mid(lbl);
        // segundo stmt
        mips_if_else_end(lbl);
    }
  | WHILE expr stmt {
        int lbl = label_count++;
        mips_while_start(lbl);
        // expr genera cond
        // stmt ejecuta cuerpo
        mips_while_end(lbl);
    }
  | expr {
        mips_expr_result($1);
    }
  ;

expr:
    NUM { $$ = $1; mips_push_int($1); }
  | BOOL { $$ = $1; mips_push_int($1); }
  | identifier {
        printf("Uso de variable: %s\n", $1);
        mips_use_var($1);
        free($1);
        $$ = 0;
    }
  | expr PLUS expr { $$ = $1 + $3; mips_add(); }
  | expr MINUS expr { $$ = $1 - $3; mips_sub(); }
  | expr MUL expr { $$ = $1 * $3; mips_mul(); }
  | expr DIV expr {
        if ($3 == 0) yyerror("división por cero");
        else {
            $$ = $1 / $3;
            mips_div();
        }
    }
  | expr EQ expr { $$ = $1 == $3; mips_eq(); }
  | expr NEQ expr { $$ = $1 != $3; mips_neq(); }
  | expr LT expr { $$ = $1 < $3; mips_lt(); }
  | expr GT expr { $$ = $1 > $3; mips_gt(); }
  | expr LE expr { $$ = $1 <= $3; mips_le(); }
  | expr GE expr { $$ = $1 >= $3; mips_ge(); }
  | identifier ASSIGN expr {
        printf("Asignación a %s = %d\n", $1, $3);
        mips_assign($1);
        free($1);
        $$ = $3;
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
    init_mips();
    yyparse();
    end_mips();
    return 0;
}