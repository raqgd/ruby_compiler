%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"           /* Definiciones y funciones AST */
#include "sym_tab.h"       /* Tabla de símbolos */
#include "mips_generator.h"/* Generación de código MIPS */

#include "parser.tab.h"    /* Incluye tokens y tipos de Bison para retornar tokens */

FILE *salida;               /* Archivo de salida para debug/log */
int unterminated_comment = 0; /* Control de comentarios multilínea no cerrados */

int yywrap(void) {
    return 1;
}

%}

%option yylineno

%x COMMENT

%%

"=begin"               { unterminated_comment = 1; BEGIN(COMMENT); }
<COMMENT>"=end"        { unterminated_comment = 0; BEGIN(INITIAL); }
<COMMENT>[^=]+         { /* Ignorar contenido dentro del comentario */ }
<COMMENT>"="[^e]+      { /* Para no confundir con '=end' */ }
<COMMENT>"=e"[^n]+     { /* Igual que arriba */ }
<COMMENT>"=en"[^d]+    { /* Igual que arriba */ }

<<EOF>>                { 
                          if (unterminated_comment) {
                              fprintf(stderr, "Error: comentario multilínea '=begin' no cerrado con '=end' al final del archivo (línea %d)\n", yylineno);
                              exit(1);
                          }
                      }

"#".*                 { /* Ignorar comentarios de línea */ }

"if"                  { 
    fprintf(salida, "KEYWORD: %s, LINEA: %d\n", yytext, yylineno);
    return IF;
}
"else"                { 
    fprintf(salida, "KEYWORD: %s, LINEA: %d\n", yytext, yylineno);
    return ELSE;
}
"while"               { 
    fprintf(salida, "KEYWORD: %s, LINEA: %d\n", yytext, yylineno);
    return WHILE;
}
"print"               { 
    fprintf(salida, "FUNCTION: %s, LINEA: %d\n", yytext, yylineno);
    return PRINT;
}

"def"                 { 
    fprintf(salida, "KEYWORD: %s, LINEA: %d\n", yytext, yylineno);
    return DEF;
}

"return"              { 
    fprintf(salida, "KEYWORD: %s, LINEA: %d\n", yytext, yylineno);
    return RETURN;
}

"true"                { 
    fprintf(salida, "LITERAL: %s, LINEA: %d\n", yytext, yylineno);
    yylval.boolean = 1;
    return BOOL;
}

"false"               { 
    fprintf(salida, "LITERAL: %s, LINEA: %d\n", yytext, yylineno);
    yylval.boolean = 0;
    return BOOL;
}

\"([^\\\"]|\\.)*\"    { 
    fprintf(salida, "STRING: %s, LINEA: %d\n", yytext, yylineno);
    char* temp = strdup(yytext + 1);  
    temp[strlen(temp) - 1] = '\0';   
    yylval.str = temp;
    return STRING;
}

\'([^\\\']|\\.)*\'    { 
    fprintf(salida, "STRING: %s, LINEA: %d\n", yytext, yylineno);
    char* temp = strdup(yytext + 1); 
    temp[strlen(temp) - 1] = '\0';  
    yylval.str = temp;
    return STRING;
}

[a-zA-Z_][a-zA-Z0-9_]*    { 
    fprintf(salida, "IDENTIFIER: %s, LINEA: %d\n", yytext, yylineno);
    yylval.str = strdup(yytext);
    return ID;
}

"<"                   { 
    fprintf(salida, "OPERATOR_LT: %s, LINEA: %d\n", yytext, yylineno);
    return LT;
}

">"                   { 
    fprintf(salida, "OPERATOR_GT: %s, LINEA: %d\n", yytext, yylineno);
    return GT;
}

"+"                   { 
    fprintf(salida, "OPERATOR_PLUS: %s, LINEA: %d\n", yytext, yylineno);
    return PLUS;
}

"-"                   { 
    fprintf(salida, "OPERATOR_MINUS: %s, LINEA: %d\n", yytext, yylineno);
    return MINUS;
}

"*"                   { 
    fprintf(salida, "OPERATOR_MUL: %s, LINEA: %d\n", yytext, yylineno);
    return MUL;
}

"/"                   { 
    fprintf(salida, "OPERATOR_DIV: %s, LINEA: %d\n", yytext, yylineno);
    return DIV;
}

"="                   { 
    fprintf(salida, "ASSIGN: %s, LINEA: %d\n", yytext, yylineno);
    return ASSIGN;
}

"=="                  { 
    fprintf(salida, "OPERATOR_EQ: %s, LINEA: %d\n", yytext, yylineno);
    return EQ;
}

"!="                  { 
    fprintf(salida, "OPERATOR_NEQ: %s, LINEA: %d\n", yytext, yylineno);
    return NEQ;
}

"<="                  { 
    fprintf(salida, "OPERATOR_LE: %s, LINEA: %d\n", yytext, yylineno);
    return LE;
}

">="                  { 
    fprintf(salida, "OPERATOR_GE: %s, LINEA: %d\n", yytext, yylineno);
    return GE;
}

[0-9]+\.[0-9]+        { 
    fprintf(salida, "FLOAT: %s, LINEA: %d\n", yytext, yylineno);
    yylval.flt = atof(yytext);
    return FLOAT;
}

[0-9]+                { 
    fprintf(salida, "NUM: %s, LINEA: %d\n", yytext, yylineno);
    yylval.integer = atoi(yytext);
    return NUM;
}

[ \t\n]+              { /* Ignorar espacios en blanco */ }

.                     { 
    fprintf(salida, "CARACTER NO RECONOCIDO: %s, LINEA: %d\n", yytext, yylineno);
    fprintf(stderr, "Advertencia: Caracter no reconocido '%s' en linea %d\n", yytext, yylineno);
}

%%