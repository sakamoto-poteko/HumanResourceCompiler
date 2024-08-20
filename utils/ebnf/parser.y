%{
#include <cstdio>
#include <cstdlib>

#include <string>
#include <memory>

#include "ASTNode.h"

extern char *yytext;
extern int yylex();
extern int yycolno;
extern int yylineno;
void yyerror(const char *s);
std::shared_ptr<SyntaxNode> root; // This will hold the root of the AST
%}

%union {
    char* str;
    ASTNode* node;
    ExpressionNode* expression;
    TermNode* term;
    FactorNode* factor;
    OptionalNode *optional;
    RepeatedNode *repeated;
    GroupedNode *grouped;
}
%start syntax

%token <str> ID LITERAL

%token EQUALS SEMICOLON PIPE COMMA LBRACKET RBRACKET LPAREN RPAREN LBRACE RBRACE

%type <node> syntax production
%type <expression> expression
%type <term> term
%type <factor> factor
%type <optional> optional
%type <repeated> repeated
%type <grouped> grouped

%%
syntax:
    /* empty */
    { root = std::make_shared<SyntaxNode>(); }
    | syntax production
    {
        root->productions.push_back(ASTNodePtr($2));
    }
    ;

production:
    ID EQUALS expression SEMICOLON
    {
        $$ = new ProductionNode(std::string($1), ASTNodePtr($3));
    }
    ;

expression:
    term
    {
        $$ = new ExpressionNode();
        $$->addTerm(ASTNodePtr($1));
    }
    | expression PIPE term
    {
        $1->addTerm(ASTNodePtr($3));
        $$ = $1;
    }
    ;

term:
    factor
    {
        $$ = new TermNode();
        $$->addFactor(ASTNodePtr($1));
    }
    | term COMMA factor
    {
        $1->addFactor(ASTNodePtr($3));
        $$ = $1;
    }
    ;

factor:
    ID
    {
        $$ = new FactorNode(std::string($1));
        std::free($1);
    }
    | LITERAL
    {
        $$ = new FactorNode(std::string($1));
        std::free($1);
    }
    | optional
    {
        $$ = new FactorNode(ASTNodePtr($1));
    }
    | repeated
    {
        $$ = new FactorNode(ASTNodePtr($1));
    }
    | grouped
    {
        $$ = new FactorNode(ASTNodePtr($1));
    }
    ;

optional:
    LBRACKET expression RBRACKET
    {
        $$ = new OptionalNode(ASTNodePtr($2));
    }
    ;

repeated:
    LBRACE expression RBRACE
    {
        $$ = new RepeatedNode(ASTNodePtr($2));
    }
    ;

grouped:
    LPAREN expression RPAREN
    {
        $$ = new GroupedNode(ASTNodePtr($2));
    }
    ;
%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s at line %d, column %d\n", s, yylineno, yycolno);
}
