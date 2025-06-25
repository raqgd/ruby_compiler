%{
#include "parser.tab.h"
#include <stdlib.h>
#include <string.h>
%}

digit   [0-9]
id      [a-zA-Z_][a-zA-Z0-9_]*
ws      [ \t\r]+

%%

"if"            return IF;
"else"          return ELSE;
"while"         return WHILE;

"=="            return EQ;
"!="            return NEQ;
"<="            return LE;
">="            return GE;
"<"             return LT;
">"             return GT;

"+"             return PLUS;
"-"             return MINUS;
"*"             return MUL;
"/"             return DIV;

"="             return ASSIGN;

"true"          { yylval.boolean = 1; return BOOL; }
"false"         { yylval.boolean = 0; return BOOL; }

{digit}+        { yylval.integer = atoi(yytext); return NUM; }
{id}            { yylval.str = strdup(yytext); return ID; }

"#".*           ; // Comentario de una línea
"=begin"([^=]|[^b]|[^e]|[^g]|[^i]|[^n])*"=end"  ; // Comentario multilinea

\n              return '\n';
{ws}            ; // Ignorar espacios
.               return yytext[0];

%%

int yywrap() { return 1; }