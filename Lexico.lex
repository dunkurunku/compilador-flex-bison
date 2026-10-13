%option yylineno


%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TS.h"
#include "Semantico.tab.h"
#include "SemanticoGest.h"
%}

%%

"_"                 { return FINPROC; }

"init:"             { return INIT; }
"proc"              { return PROC;}
"if"                { return COND; }
"while"             { return CICLO; }
"repeat"            { return REPEAT; }
"until"             { return UNTIL; }
"else"              { return ELSE; }
"in"                { return IN; }
"out"               { return OUT; }
"="                 { return ASIGN; }

"*"                 { yylval.atrib = 0; yylval.tipo = NO_ASIGNADO; return BINOP; }
"/"                 { yylval.atrib = 1; yylval.tipo = NO_ASIGNADO; return BINOP; }
"%"                 { yylval.atrib = 2; yylval.tipo = NO_ASIGNADO; return BINOP; }
"**"                { yylval.atrib = 3; yylval.tipo = NO_ASIGNADO; return BINOP; }
"&&"                { yylval.atrib = 4; yylval.tipo = NO_ASIGNADO; return BINOP; }
"||"                { yylval.atrib = 5; yylval.tipo = NO_ASIGNADO; return BINOP; }
"^"                 { yylval.atrib = 6; yylval.tipo = NO_ASIGNADO; return BINOP; }
"<"                 { yylval.atrib = 7; yylval.tipo = BOOLEANO;  return BINOP; }
"<="                { yylval.atrib = 8; yylval.tipo = BOOLEANO;  return BINOP; }
">"                 { yylval.atrib = 9; yylval.tipo = BOOLEANO;  return BINOP; }
">="                { yylval.atrib = 10; yylval.tipo = BOOLEANO; return BINOP; }
"=="                { yylval.atrib = 11; yylval.tipo = BOOLEANO; return BINOP; }
"&"                 { yylval.atrib = 12; yylval.tipo = BOOLEANO; return BINOP; }
"|"                 { yylval.atrib = 13; yylval.tipo = BOOLEANO; return BINOP; }
"--"                { yylval.atrib = 14; yylval.tipo = NO_ASIGNADO; return BINOP; }
"!="                { yylval.atrib = 15; yylval.tipo = BOOLEANO; return BINOP; }


"!"                 { yylval.atrib = 0;  return UNOP; }
"#"                 { yylval.atrib = 1;  return UNOP; }
"?"                 { yylval.atrib = 2;  return UNOP; }

"@"                 { return OPARR; }
"++"                { return OPMASMAS; }

"+"                 { yylval.atrib = 0;  return OPSUMRESTA; }
"-"                 { yylval.atrib = 1;  return OPSUMRESTA; }


">>"                { return SHIFT_DER; }
"<<"                { return SHIFT_IZQ; }
"$"                 { return DOLLAR; }

"("                 { return PARIZQ; }
")"                 { return PARDER; }
"["                 { return CORCHIZQ; }
"]"                 { return CORCHDER; }
"{"                 { return LLAVIZQ; }
"}"                 { return LLAVDER; }
","                 { return COMA; }
";"                 { return PYC; }

"int"               { yylval.tipo = ENTERO;  return TIPO; }
"char"              { yylval.tipo = CARACTER;  return TIPO; }
"float"             { yylval.tipo = REAL;  return TIPO; }
"bool"              { yylval.tipo = BOOLEANO;  return TIPO; }
"string"            { yylval.tipo = STRING;  return TIPO; }
"list"              { return LIST; }

[0-9]+              { yylval.atrib = 0; yylval.tipo = ENTERO ; yylval.entero = atoi(yytext); return LITERAL; }
'.'                 { yylval.atrib = 1; yylval.tipo = CARACTER ; yylval.str = strdup(yytext); return LITERAL; }
[0-9]+\.[0-9]+      { yylval.atrib = 2; yylval.tipo = REAL ; yylval.flotante = atof(yytext); return LITERAL; }
"true"|"false"      { yylval.atrib = 3; yylval.tipo = BOOLEANO ; yylval.booleano = (strcmp(yytext, "true") == 0); return LITERAL; }
\"[^\"]*\"          { yylval.atrib = 4; yylval.tipo = STRING ; yylval.str = strdup(yytext); return LITERAL; }

[a-zA-Z][a-zA-Z0-9]* { yylval.lexema = strdup(yytext); return ID; }

"<var>"             { return VARIZQ; }
"<var_>"            { return VARDER; }

"//".*                              ;
"/*"([^*]|\*+[^*/])*\*+"/"          ;


[ \t\r\n]+    ;   // Ignorar espacios, tabs, retornos y saltos de línea

[\x80-\xFF]+        { yylval.atrib = -1; ; printf("[ERROR] Línea %d : Token no reconocido: '%s'", yylineno, yytext); num_errores_lexicos++;}   // captura caracteres no ASCII

.                   { yylval.atrib = -1; printf("[ERROR] Línea %d : Token no reconocido: '%s'\n", yylineno, yytext); num_errores_lexicos++;}

%%



int yywrap(void) {
    return 1;
}
