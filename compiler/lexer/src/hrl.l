%{

#include <cstdio>
#include "lexer_global.h"
#include "lexer_helper.h"

#include "HRLToken.h"

using namespace std;
using namespace hrl::lexer;

int yycolno = 0;
bool last_token_newline = false;

// YY_USER_ACTION is executed before the rule codes
// The code is weird but that's it.
// The actual start col of row is yycolno - yyleng, after yylex()
#define YY_USER_ACTION \
if (last_token_newline) { yycolno = 0; last_token_newline = false; } \
yycolno += yyleng;  \
for (int i = 0; i < yyleng; ++i) { if (yytext[i] == '\n') { last_token_newline = true; break; } }

%}

%option nounistd
%option never-interactive
%option noyywrap
%option yylineno

%%
"//".*                          { __currentToken.preceding_metadata.emplace_back(TokenMetadata {.type=TokenMetadata::Comment, .value = std::make_shared<std::string>(yytext)}); return COMMENT; }

"import"                        { return IMPORT;  }
"return"                        { return RETURN;  }
"let"                           { return LET;     }
"init"                          { return INIT;    }
"floor"                         { return FLOOR;   }
"floor_max"                     { return FLOOR_MAX;}
"function"                      { return FUNCTION;}
"sub"                           { return SUBWORD;}

"if"                            { return IF;    }
"else"                          { return ELSE;  }
"while"                         { return WHILE; }
"for"                           { return FOR;   }

">="                            { return GE; }
"<="                            { return LE; }
"=="                            { return EE; }
"!="                            { return NE; }
">"                             { return GT; }
"<"                             { return LT; }

"("                             { return OPEN_PAREN;   }
")"                             { return CLOSE_PAREN;  }
"{"                             { return OPEN_BRACE;   }
"}"                             { return CLOSE_BRACE;  }
"["                             { return OPEN_BRACKET; }
"]"                             { return CLOSE_BRACKET;}
","                             { return COMMA;        }

"&&"                            { return AND;          }
"||"                            { return OR;           }

"++"                            { return ADDADD;       }
"--"                            { return SUBSUB;       }
"!"                             { return NOT;          }
"+"                             { return ADD;          }
"-"                             { return SUB;          }
"*"                             { return MUL;          }
"/"                             { return DIV;          }
"%"                             { return MOD;          }
"="                             { return EQ;           }
";"                             { return T;            }

"true"                          { __currentToken.boolean = true; return BOOLEAN; }
"false"                         { __currentToken.boolean = false; return BOOLEAN; }
[0-9]+                          { __currentToken.integer = atoi(yytext); return INTEGER; }

[A-Za-z_][A-Za-z0-9_]*          { __currentToken.identifier = std::make_shared<std::string>(yytext); return IDENTIFIER; }
[ \t]+                          { }
[\n\r]                          { __currentToken.preceding_metadata.emplace_back(TokenMetadata {.type=TokenMetadata::Newline}); return NEWLINE; }

.                               { return ERROR; }

%%

