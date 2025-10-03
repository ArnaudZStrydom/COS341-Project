%{
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "spl_lexer.h"
#include "ast.h"

extern int yylex();
void yyerror(const char *s);
AstNode* ast_root = nullptr;
extern int current_line_number;
%}

%code requires {
    #include <string>
    #include <vector>
    class AstNode;
    class StatementNode;
    class ExpressionNode;
    class VarNode;
    class BodyNode;
    class MainProgNode;
    template<typename T> class AstNodeList;
    class ProcDefNode;
    class FuncDefNode;
    class FuncCallNode;
}

%union {
    std::string* sval;
    AstNode* node;
}

%token <sval> GLOB PROC FUNC MAIN LOCAL VAR RETURN HALT PRINT
%token <sval> WHILE DO UNTIL IF ELSE NEG NOT EQ OR AND
%token <sval> PLUS MINUS MULT DIV
%token <sval> LBRACE RBRACE LPAREN RPAREN SEMICOLON ASSIGN GT
%token <sval> IDENTIFIER NUMBER STRING

%type <node> spl_prog variables var name procdefs pdef funcdefs fdef
%type <node> body param maxthree mainprog atom algo instr_list instr
%type <node> assign loop branch output input term
%type <node> assign_rhs
%start spl_prog

%%

spl_prog:
    GLOB LBRACE variables RBRACE
    PROC LBRACE procdefs RBRACE
    FUNC LBRACE funcdefs RBRACE
    MAIN LBRACE mainprog RBRACE
    {
        ast_root = new ProgramNode(
            static_cast<AstNodeList<VarNode>*>($3),    
            static_cast<AstNodeList<ProcDefNode>*>($7),  
            static_cast<AstNodeList<FuncDefNode>*>($11), 
            static_cast<MainProgNode*>($15)             
        );
    }

variables:
    /* empty */ { $$ = new AstNodeList<VarNode>(); }
    | variables var {
        auto* list = static_cast<AstNodeList<VarNode>*>($1);
        list->elements.push_back(static_cast<VarNode*>($2)); // Use push_back to append
        $$ = list;
    }

var:
    IDENTIFIER { $$ = new VarNode(*$1); delete $1; }
    ;

name:
    IDENTIFIER { $$ = new VarNode(*$1); delete $1; }
    ;

procdefs:
    /* empty */ { $$ = new AstNodeList<ProcDefNode>(); }
    | procdefs pdef {
        auto* list = static_cast<AstNodeList<ProcDefNode>*>($1);
        list->elements.push_back(static_cast<ProcDefNode*>($2));
        $$ = list;
    }
    ;

pdef:
    name LPAREN param RPAREN LBRACE body RBRACE
    {
        VarNode* name_node = static_cast<VarNode*>($1);
        $$ = new ProcDefNode(name_node->name, static_cast<AstNodeList<VarNode>*>($3), static_cast<BodyNode*>($6));
        delete name_node;
    }
    ;

funcdefs:
    /* empty */ { $$ = new AstNodeList<FuncDefNode>(); }
    | funcdefs fdef {
        auto* list = static_cast<AstNodeList<FuncDefNode>*>($1);
        list->elements.push_back(static_cast<FuncDefNode*>($2));
        $$ = list;
    }
    ;

fdef:
    name LPAREN param RPAREN LBRACE body RBRACE 
    {
        VarNode* name_node = static_cast<VarNode*>($1);
        $$ = new FuncDefNode(name_node->name, static_cast<AstNodeList<VarNode>*>($3), static_cast<BodyNode*>($6));
        delete name_node;
    }
    ;

body:
    LOCAL LBRACE maxthree RBRACE algo
    { $$ = new BodyNode(static_cast<AstNodeList<VarNode>*>($3), static_cast<AstNodeList<StatementNode>*>($5)); }
    ;

param:
    maxthree { $$ = $1; }
    ;

maxthree:
    /* empty */ { $$ = new AstNodeList<VarNode>(); }
    | var { auto* list = new AstNodeList<VarNode>(); list->elements.push_back(static_cast<VarNode*>($1)); $$ = list; }
    | var var { auto* list = new AstNodeList<VarNode>(); list->elements.push_back(static_cast<VarNode*>($1)); list->elements.push_back(static_cast<VarNode*>($2)); $$ = list; }
    | var var var { auto* list = new AstNodeList<VarNode>(); list->elements.push_back(static_cast<VarNode*>($1)); list->elements.push_back(static_cast<VarNode*>($2)); list->elements.push_back(static_cast<VarNode*>($3)); $$ = list; }
    ;

mainprog:
    VAR LBRACE variables RBRACE algo
    { $$ = new MainProgNode(static_cast<AstNodeList<VarNode>*>($3), static_cast<AstNodeList<StatementNode>*>($5)); }
    ;

atom:
    var { $$ = $1; }
    | NUMBER { $$ = new NumberNode(*$1); delete $1; }
    ;
    
algo:
    instr_list { $$ = $1; } 
    ;

instr_list:
    instr {
        auto* list = new AstNodeList<StatementNode>();
        list->elements.push_back(static_cast<StatementNode*>($1));
        $$ = list;
    }
    | instr_list SEMICOLON instr { 
        auto* list = static_cast<AstNodeList<StatementNode>*>($1);
        list->elements.push_back(static_cast<StatementNode*>($3)); // Index is now $3
        $$ = list;
    }

instr:
    HALT { $$ = new HaltNode(); }
    | PRINT output { $$ = new PrintNode(static_cast<ExpressionNode*>($2)); }
    | name LPAREN input RPAREN {
        VarNode* name_node = static_cast<VarNode*>($1);
        $$ = new ProcCallNode(name_node->name, static_cast<AstNodeList<ExpressionNode>*>($3));
        delete name_node;
    }
    | assign { $$ = $1; }
    | loop { $$ = $1; }
    | branch { $$ = $1; }
    | RETURN atom { $$ = new ReturnNode(static_cast<ExpressionNode*>($2)); }
    ;

assign:
    var ASSIGN assign_rhs 
    { $$ = new AssignNode(static_cast<VarNode*>($1), static_cast<ExpressionNode*>($3)); }
    ;

assign_rhs:
    name LPAREN input RPAREN
    {
        VarNode* name_node = static_cast<VarNode*>($1);
        $$ = new FuncCallNode(name_node->name, static_cast<AstNodeList<ExpressionNode>*>($3));
        delete name_node;
    }
  | term { $$ = $1; }
    ;
loop:
    WHILE term LBRACE algo RBRACE { $$ = new WhileNode(static_cast<ExpressionNode*>($2), static_cast<AstNodeList<StatementNode>*>($4)); }
    | DO LBRACE algo RBRACE UNTIL term { $$ = new DoUntilNode(static_cast<AstNodeList<StatementNode>*>($3), static_cast<ExpressionNode*>($6)); }
    ;

branch:
    IF term LBRACE algo RBRACE { $$ = new IfNode(static_cast<ExpressionNode*>($2), static_cast<AstNodeList<StatementNode>*>($4)); }
    | IF term LBRACE algo RBRACE ELSE LBRACE algo RBRACE { $$ = new IfElseNode(static_cast<ExpressionNode*>($2), static_cast<AstNodeList<StatementNode>*>($4), static_cast<AstNodeList<StatementNode>*>($8)); }
    ;

output:
    atom { $$ = $1; }
    | STRING { $$ = new StringNode(*$1); delete $1; }
    ;

input:
    /* empty */ { $$ = new AstNodeList<ExpressionNode>(); }
    | atom { auto* list = new AstNodeList<ExpressionNode>(); list->elements.push_back(static_cast<ExpressionNode*>($1)); $$ = list; }
    | atom atom { auto* list = new AstNodeList<ExpressionNode>(); list->elements.push_back(static_cast<ExpressionNode*>($1)); list->elements.push_back(static_cast<ExpressionNode*>($2)); $$ = list; }
    | atom atom atom { auto* list = new AstNodeList<ExpressionNode>(); list->elements.push_back(static_cast<ExpressionNode*>($1)); list->elements.push_back(static_cast<ExpressionNode*>($2)); list->elements.push_back(static_cast<ExpressionNode*>($3)); $$ = list; }
    ;

term:
    atom { $$ = $1; }
    | LPAREN NEG term RPAREN { $$ = new UnaryOpNode("neg", static_cast<ExpressionNode*>($3)); }
    | LPAREN NOT term RPAREN { $$ = new UnaryOpNode("not", static_cast<ExpressionNode*>($3)); }
    | LPAREN term EQ term RPAREN { $$ = new BinaryOpNode(static_cast<ExpressionNode*>($2), "eq", static_cast<ExpressionNode*>($4)); }
    | LPAREN term GT term RPAREN { $$ = new BinaryOpNode(static_cast<ExpressionNode*>($2), ">", static_cast<ExpressionNode*>($4)); }
    | LPAREN term OR term RPAREN { $$ = new BinaryOpNode(static_cast<ExpressionNode*>($2), "or", static_cast<ExpressionNode*>($4)); }
    | LPAREN term AND term RPAREN { $$ = new BinaryOpNode(static_cast<ExpressionNode*>($2), "and", static_cast<ExpressionNode*>($4)); }
    | LPAREN term PLUS term RPAREN { $$ = new BinaryOpNode(static_cast<ExpressionNode*>($2), "plus", static_cast<ExpressionNode*>($4)); }
    | LPAREN term MINUS term RPAREN { $$ = new BinaryOpNode(static_cast<ExpressionNode*>($2), "minus", static_cast<ExpressionNode*>($4)); }
    | LPAREN term MULT term RPAREN { $$ = new BinaryOpNode(static_cast<ExpressionNode*>($2), "mult", static_cast<ExpressionNode*>($4)); }
    | LPAREN term DIV term RPAREN { $$ = new BinaryOpNode(static_cast<ExpressionNode*>($2), "div", static_cast<ExpressionNode*>($4)); }
    ;

%%

void yyerror(const char *s) {
    std::cerr << "Syntax Error on line " << current_line_number << ": " << s << std::endl;
}