%{
#include "ast.h"
#include <stdio.h>

int yylex();
void yyerror(const char* s);
ASTNode* root = nullptr;
%}

%union {
    int num;
    double dnum;
    char* str;
    ASTNode* node;
}

%token <num> NUM BOOL_LIT
%token <dnum> FLOAT_NUM
%token <str> ID
%token INT_TOK DOUBLE_TOK BOOL_TOK IF ELSE WHILE FOR
%token AND OR NOT EQ NEQ LE GE PLUS MINUS MULT DIV LESS GREATER
%token ASSIGN SEMI LBRACE RBRACE LPAREN RPAREN

%type <node> program stmt_list stmt assign_stmt expr
%type <str> type_spec

%left OR
%left AND
%left EQ NEQ
%left LESS GREATER LE GE
%left PLUS MINUS
%left MULT DIV
%right NOT

%%
program: stmt_list { root = $1; }
       ;

stmt_list:
    stmt { $$ = new SeqNode(); ((SeqNode*)$$)->add($1); }
  | stmt_list stmt { ((SeqNode*)$1)->add($2); $$ = $1; }
  ;

type_spec:
    INT_TOK    { $$ = (char*)"int"; }
  | DOUBLE_TOK { $$ = (char*)"double"; }
  | BOOL_TOK   { $$ = (char*)"bool"; }
  ;

assign_stmt:
    type_spec ID ASSIGN expr { $$ = new AssignNode($2, $4, $1); }
  | ID ASSIGN expr           { $$ = new AssignNode($1, $3, ""); }
  ;

stmt:
    assign_stmt SEMI { $$ = $1; }
  | IF LPAREN expr RPAREN LBRACE stmt_list RBRACE { $$ = new IfNode($3, $6); }
  | IF LPAREN expr RPAREN LBRACE stmt_list RBRACE ELSE LBRACE stmt_list RBRACE { $$ = new IfNode($3, $6, $10); }
  | WHILE LPAREN expr RPAREN LBRACE stmt_list RBRACE { $$ = new WhileNode($3, $6); }
  | FOR LPAREN assign_stmt SEMI expr SEMI assign_stmt RPAREN LBRACE stmt_list RBRACE { $$ = new ForNode($3, $5, $7, $10); }
  ;

expr:
    expr OR expr      { $$ = new BinOpNode("||", $1, $3); }
  | expr AND expr     { $$ = new BinOpNode("&&", $1, $3); }
  | expr EQ expr      { $$ = new BinOpNode("==", $1, $3); }
  | expr NEQ expr     { $$ = new BinOpNode("!=", $1, $3); }
  | expr LESS expr    { $$ = new BinOpNode("<", $1, $3); }
  | expr GREATER expr { $$ = new BinOpNode(">", $1, $3); }
  | expr LE expr      { $$ = new BinOpNode("<=", $1, $3); }
  | expr GE expr      { $$ = new BinOpNode(">=", $1, $3); }
  | expr PLUS expr    { $$ = new BinOpNode("+", $1, $3); }
  | expr MINUS expr   { $$ = new BinOpNode("-", $1, $3); }
  | expr MULT expr    { $$ = new BinOpNode("*", $1, $3); }
  | expr DIV expr     { $$ = new BinOpNode("/", $1, $3); }
  | NOT expr          { $$ = new UnaryOpNode("!", $2); }
  | LPAREN expr RPAREN { $$ = new ParenNode($2); }
  | NUM               { $$ = new NumNode($1); }
  | FLOAT_NUM         { $$ = new NumNode($1); }
  | BOOL_LIT          { $$ = new NumNode((bool)($1 != 0)); }
  | ID                { $$ = new IdNode($1); }
  ;
%%

void yyerror(const char* s) {
    fprintf(stderr, "Error: %s\n", s);
}
