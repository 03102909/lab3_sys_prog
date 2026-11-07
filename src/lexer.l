%{
#include "ast.h"
#include "parser.tab.h"
#include <string.h>
%}

%option noyywrap

%%
"int"            { return INT_TOK; }
"double"         { return DOUBLE_TOK; }
"bool"           { return BOOL_TOK; }
"true"           { yylval.num = 1; return BOOL_LIT; }
"false"          { yylval.num = 0; return BOOL_LIT; }
"if"             { return IF; }
"else"           { return ELSE; }
"while"          { return WHILE; }
"for"            { return FOR; }
[a-zA-Z]+        { yylval.str = strdup(yytext); return ID; }
[0-9]+\.[0-9]+   { yylval.dnum = atof(yytext); return FLOAT_NUM; }
[0-9]+           { yylval.num = atoi(yytext); return NUM; }
"&&"             { return AND; }
"||"             { return OR; }
"=="             { return EQ; }
"!="             { return NEQ; }
"<="             { return LE; }
">="             { return GE; }
"!"              { return NOT; }
"+"              { return PLUS; }
"-"              { return MINUS; }
"*"              { return MULT; }
"/"              { return DIV; }
"<"              { return LESS; }
">"              { return GREATER; }
"="              { return ASSIGN; }
";"              { return SEMI; }
"{"              { return LBRACE; }
"}"              { return RBRACE; }
"("              { return LPAREN; }
")"              { return RPAREN; }
[ \t\n\r]        { /* ignore whitespace */ }
.                { fprintf(stderr, "Unknown character: %s\n", yytext); }
%%
