#ifndef LEXER_HELPER_H
#define LEXER_HELPER_H

#include "hrl_global.h"
#include "lexer_global.h"
#include <string>
#include <vector>

OPEN_LEXER_NAMESPACE

struct TokenMetadata
{
    enum Type
    {
        Newline,
        Comment,
    };

    Type type;
    StringPtr value;
};

struct CurrentToken
{
public:
    int integer;
    bool boolean;
    StringPtr identifier;
    std::vector<TokenMetadata> preceding_metadata;

    void clear() {
        integer = 0;
        boolean = false;
        identifier.reset();
        preceding_metadata.clear();
    }
};

extern CurrentToken __currentToken;

CLOSE_LEXER_NAMESPACE

#endif