%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int yylex(void);
void yyerror(const char *s);
%}

/* Keywords as tokens */
%token GLOB PROC FUNC MAIN LOCAL VAR_KW
%token HALT PRINT RETURN WHILE DO UNTIL IF ELSE
%token NEG NOT EQ GT OR AND PLUS MINUS MULT DIV

/* Terminals for literals and values */
%token USERNAME NUMBER STRING
%token '=' '(' ')' '{' '}' ';'

%%

/* Start rule */
SPL_PROG
    : GLOB '{' VARIABLES '}' PROC '{' PROCDEFS '}' FUNC '{' FUNCDEFS '}' MAIN '{' MAINPROG '}'
    ;

VARIABLES
    : /* empty */
    | USERNAME VARIABLES
    ;

NAME
    : USERNAME
    ;

PROCDEFS
    : /* empty */
    | PDEF PROCDEFS
    ;

PDEF
    : NAME '(' PARAM ')' '{' BODY '}'
    ;

FDEF
    : NAME '(' PARAM ')' '{' BODY ';' RETURN ATOM '}'
    ;

FUNCDEFS
    : /* empty */
    | FDEF FUNCDEFS
    ;

BODY
    : LOCAL '{' MAXTHREE '}' ALGO
    ;

PARAM
    : MAXTHREE
    ;

MAXTHREE
    : /* empty */
    | USERNAME
    | USERNAME USERNAME
    | USERNAME USERNAME USERNAME
    ;

MAINPROG
    : VAR_KW '{' VARIABLES '}' ALGO
    ;

ATOM
    : USERNAME
    | NUMBER
    ;

ALGO
    : INSTR
    | INSTR ';' ALGO
    ;

INSTR
    : HALT
    | PRINT OUTPUT
    | NAME '(' INPUT ')'          /* procedure call */
    | ASSIGN
    | LOOP
    | BRANCH
    ;

ASSIGN
    : USERNAME '=' NAME '(' INPUT ')'  /* function call */
    | USERNAME '=' TERM
    ;

LOOP
    : WHILE TERM '{' ALGO '}'
    | DO '{' ALGO '}' UNTIL TERM
    ;

BRANCH
    : IF TERM '{' ALGO '}'
    | IF TERM '{' ALGO '}' ELSE '{' ALGO '}'
    ;

OUTPUT
    : ATOM
    | STRING
    ;

INPUT
    : /* empty */
    | ATOM
    | ATOM ATOM
    | ATOM ATOM ATOM
    ;

TERM
    : ATOM
    | '(' UNOP TERM ')'
    | '(' TERM BINOP TERM ')'
    ;

UNOP
    : NEG
    | NOT
    ;

BINOP
    : EQ
    | GT
    | OR
    | AND
    | PLUS
    | MINUS
    | MULT
    | DIV
    ;
%%

/* Error handler */
void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}


int main(void) {
    return yyparse();
}