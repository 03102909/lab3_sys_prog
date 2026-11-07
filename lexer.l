%{
#include "ast.h"
#include "parser.tab.h"
#include <string.h>
%}

%option noyywrap

%%
"int"       { return INT_TOK; }
"if"        { return IF; }
[a-zA-Z]+   { yylval.str = strdup(yytext); return ID; }
[0-9]+      { yylval.num = atoi(yytext); return NUM; }
"+"         { return PLUS; }
"*"         { return MULT; }
"<"         { return LESS; }
"="         { return ASSIGN; }
";"         { return SEMI; }
"{"         { return LBRACE; }
"}"         { return RBRACE; }
"("         { return LPAREN; }
")"         { return RPAREN; }
[ \t\n]     { /* ignore whitespace */ }
.           { printf("Unknown character: %s\n", yytext); }
%%