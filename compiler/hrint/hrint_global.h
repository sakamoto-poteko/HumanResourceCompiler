#ifndef HRINT_GLOBAL_H
#define HRINT_GLOBAL_H

#include "hrl_global.h" // IWYU pragma: keep

#define OPEN_HRINT_NAMESPACE \
    namespace hrl {          \
    namespace hrint {
#define CLOSE_HRINT_NAMESPACE \
    }                         \
    }

OPEN_HRINT_NAMESPACE

CLOSE_HRINT_NAMESPACE

#endif