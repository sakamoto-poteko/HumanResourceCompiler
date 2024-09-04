%{
#include <cstdio>
#include <cstdlib>

#include <string>
#include <memory>

#include "ParseTreeNode.h"

extern char *yytext;
extern int yylex();
extern int yycolno;
extern int yylineno;
void yyerror(const char *s);
std::shared_ptr<SyntaxNode> root; // This will hold the root of the ParseTree
%}

%locations

%union {
    char* str;
    SyntaxNode* syntax;
    ProductionNode* production;
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

%type <syntax> syntax 
%type <production> production
%type <expression> expression
%type <term> term
%type <factor> factor
%type <optional> optional
%type <repeated> repeated
%type <grouped> grouped

%%
syntax:
    production
    { root = std::make_shared<SyntaxNode>(@1.first_line, @1.first_column); root->productions.push_back(ProductionNodePtr($1)); }
    | syntax production
    {
        root->productions.push_back(ProductionNodePtr($2));
    }
    ;

production:
    ID EQUALS expression SEMICOLON
    {
        $$ = new ProductionNode(std::string($1), ExpressionNodePtr($3), @1.first_line, @1.first_column);
    }
    ;

expression:
    term
    {
        $$ = new ExpressionNode(@1.first_line, @1.first_column);
        $$->addTerm(TermNodePtr($1));
    }
    | expression PIPE term
    {
        $1->addTerm(TermNodePtr($3));
        $$ = $1;
    }
    ;

term:
    factor
    {
        $$ = new TermNode(@1.first_line, @1.first_column);
        $$->addFactor(FactorNodePtr($1));
    }
    | term COMMA factor
    {
        $1->addFactor(FactorNodePtr($3));
        $$ = $1;
    }
    | term factor
    {
        $1->addFactor(FactorNodePtr($2));
        $$ = $1;
    }    
    | epsilon
    {
        $$ = new TermNode(@1.first_line, @1.first_column);
        $$->addFactor(std::make_shared<FactorNode>(std::make_shared<EpsilonNode>(@1.first_line, @1.first_column), @1.first_line, @1.first_column));
    }
    ;

factor:
    ID
    {
        auto node = std::make_shared<IdentifierNode>(std::string($1), @1.first_line, @1.first_column);
        std::free($1);
        $$ = new FactorNode(node, @1.first_line, @1.first_column);
    }
    | LITERAL
    {
        auto node = std::make_shared<LiteralNode>(std::string($1), @1.first_line, @1.first_column);
        std::free($1);
        $$ = new FactorNode(node, @1.first_line, @1.first_column);
    }
    | optional
    {
        $$ = new FactorNode(ParseTreeNodePtr($1), @1.first_line, @1.first_column);
    }
    | repeated
    {
        $$ = new FactorNode(ParseTreeNodePtr($1), @1.first_line, @1.first_column);
    }
    | grouped
    {
        $$ = new FactorNode(ParseTreeNodePtr($1), @1.first_line, @1.first_column);
    }
    ;

epsilon:
    ;

optional:
    LBRACKET expression RBRACKET
    {
        $$ = new OptionalNode(ParseTreeNodePtr($2), @2.first_line, @2.first_column);
    }
    ;

repeated:
    LBRACE expression RBRACE
    {
        $$ = new RepeatedNode(ParseTreeNodePtr($2), @2.first_line, @2.first_column);
    }
    ;

grouped:
    LPAREN expression RPAREN
    {
        $$ = new GroupedNode(ParseTreeNodePtr($2), @2.first_line, @2.first_column);
    }
    ;
%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s at line %d, column %d\n", s, yylineno, yycolno);
}
