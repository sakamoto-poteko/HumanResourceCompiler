#ifndef LEXER_HELPER_H
#define LEXER_HELPER_H

#include "lexer_global.h"

OPEN_LEXER_NAMESPACE

struct CurrentToken
{
public:
    int integer;
    bool boolean;
    GCString identifier;
};

extern CurrentToken __currentToken;

CLOSE_LEXER_NAMESPACE

#endif