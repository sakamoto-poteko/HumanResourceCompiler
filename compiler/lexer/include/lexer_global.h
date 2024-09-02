#ifndef LEXER_GLOBAL_H
#define LEXER_GLOBAL_H

#define OPEN_LEXER_NAMESPACE \
    namespace hrl {          \
    namespace lexer {
#define CLOSE_LEXER_NAMESPACE \
    }                         \
    }

#include <cstdio>

#include "hrl_global.h"

OPEN_LEXER_NAMESPACE

class Token;
class IdentifierToken;
class IntegerToken;
class BooleanToken;
using TokenPtr = std::shared_ptr<Token>;
using IdentifierTokenPtr = std::shared_ptr<IdentifierToken>;
using IntegerTokenPtr = std::shared_ptr<IntegerToken>;
using BooleanTokenPtr = std::shared_ptr<BooleanToken>;

CLOSE_LEXER_NAMESPACE

#endif