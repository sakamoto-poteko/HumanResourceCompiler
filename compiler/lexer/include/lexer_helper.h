#ifndef LEXER_HELPER_H
#define LEXER_HELPER_H

#include <vector>

#include "hrl_global.h"
#include "lexer_global.h"

OPEN_LEXER_NAMESPACE

struct TokenMetadata {
    enum Type {
        Newline,
        Comment,
    };

    Type type = Newline;
    StringPtr value = nullptr;
};

struct CurrentToken {
public:
    int integer;
    bool boolean;
    StringPtr identifier;
    std::vector<TokenMetadata> preceding_metadata;
    bool is_char;

    void clear()
    {
        integer = 0;
        boolean = false;
        identifier.reset();
        preceding_metadata.clear();
        is_char = false;
    }
};

extern CurrentToken __currentToken;

CLOSE_LEXER_NAMESPACE

#endif