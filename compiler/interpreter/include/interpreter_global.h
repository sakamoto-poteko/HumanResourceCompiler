#ifndef INTERPRETER_GLOBAL_H
#define INTERPRETER_GLOBAL_H

#include "hrl_global.h" // IWYU pragma: keep

#define OPEN_INTERPRETER_NAMESPACE \
    namespace hrl {                \
    namespace interpreter {
#define CLOSE_INTERPRETER_NAMESPACE \
    }                               \
    }

OPEN_INTERPRETER_NAMESPACE

CLOSE_INTERPRETER_NAMESPACE

#endif