%{

#include <cstdio>



int yycolno = 0;


%}

%option nounistd
%option never-interactive
%option noyywrap

%%
"true"                          {  return 257; }
"false"                         {  return 257; }
[0-9]+                          { return 257; }

[A-Za-z_][A-Za-z0-9_]*          { return 257; }
[ \t\n\r]+                      { }

.                               { return -1; }

%%

