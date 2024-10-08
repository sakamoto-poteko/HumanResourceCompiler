%{

#include <cstdio>

#include "ASTNode.h"
#include "parser.h"

unsigned int yycolno = 0;
bool last_token_newline = false;

// YY_USER_ACTION is executed before the rule codes
// The code is weird but that's it.
// The actual start col of row is yycolno - yyleng, after yylex()
#define YY_USER_ACTION \
if (last_token_newline) { yycolno = 0; last_token_newline = false; } \
yycolno += yyleng;  \
for (std::size_t i = 0; i < yyleng; ++i) { if (yytext[i] == '\n') { last_token_newline = true; break; } } \
yylloc.first_line = yylineno; \
yylloc.first_column = yycolno - yyleng + 1; 

#define YY_NO_UNPUT
#define YY_NO_INPUT
%}

%option nounistd
%option never-interactive
%option noyywrap
%option yylineno

%%
[a-zA-Z_][a-zA-Z_0-9]*	{ yylval.str = strdup(yytext); return ID;        }
\'[^\']*\'|\"[^\"]*\"	{ yylval.str = strdup(yytext); return LITERAL;   }
"="	                    { return EQUALS;    }
":"	                    { return EQUALS;    }
"->"                    { return EQUALS;    }
"::="                   { return EQUALS;    }
";"	                    { return SEMICOLON; }
"["	                    { return LBRACKET;  }
"]"	                    { return RBRACKET;  }
"("	                    { return LPAREN;    }
")"	                    { return RPAREN;    }
"{"	                    { return LBRACE;    }
"}"	                    { return RBRACE;    }
"|"	                    { return PIPE;      }
","	                    { return COMMA;     }
[ \t\r\n]+	            { }
\(\*[^*]*\*\)           { /* comments */    }
\/\/[^\n]*              { /* comments */    }
\#[^\n]*                { /* comments */    }
%%