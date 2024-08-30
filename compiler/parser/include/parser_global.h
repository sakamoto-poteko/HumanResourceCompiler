#ifndef PARSER_GLOBAL_H
#define PARSER_GLOBAL_H

#define OPEN_PARSER_NAMESPACE \
    namespace hrl {           \
    namespace parser {
#define CLOSE_PARSER_NAMESPACE \
    }                          \
    }

#include <cstdio>

#include "hrl_global.h"

OPEN_PARSER_NAMESPACE


CLOSE_PARSER_NAMESPACE

#endif