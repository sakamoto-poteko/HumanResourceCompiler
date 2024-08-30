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
using ManagedToken = std::shared_ptr<Token>;

CLOSE_LEXER_NAMESPACE

#endif