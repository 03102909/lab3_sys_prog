%{
#include "ast.h"
#include <stdio.h>

int yylex();
void yyerror(const char* s);
ASTNode* root = nullptr;
%}

%union {
    int num;
    char* str;
    ASTNode* node;
}

%token <num> NUM
%token <str> ID
%token INT_TOK IF PLUS MULT LESS ASSIGN SEMI LBRACE RBRACE LPAREN RPAREN

%type <node> program stmt_list stmt expr

%left LESS
%left PLUS
%left MULT

%%
program: stmt_list { root = $1; }
       ;

stmt_list:
    stmt { $$ = new SeqNode(); ((SeqNode*)$$)->add($1); }
  | stmt_list stmt { ((SeqNode*)$1)->add($2); $$ = $1; }
  ;

stmt:
    INT_TOK ID ASSIGN expr SEMI { $$ = new AssignNode($2, $4, true); }
  | ID ASSIGN expr SEMI         { $$ = new AssignNode($1, $3, false); }
  | IF LPAREN expr RPAREN LBRACE stmt_list RBRACE { $$ = new IfNode($3, $6); }
  ;

expr:
    expr PLUS expr { $$ = new BinOpNode('+', $1, $3); }
  | expr MULT expr { $$ = new BinOpNode('*', $1, $3); }
  | expr LESS expr { $$ = new BinOpNode('<', $1, $3); }
  | NUM            { $$ = new NumNode($1); }
  | ID             { $$ = new IdNode($1); }
  ;
%%

void yyerror(const char* s) {
    fprintf(stderr, "Error: %s\n", s);
}