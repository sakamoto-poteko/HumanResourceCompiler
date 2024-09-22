#ifndef HRC_GLOBAL_H
#define HRC_GLOBAL_H

#include "hrl_global.h" // IWYU pragma: keep

#define OPEN_HRC_NAMESPACE \
    namespace hrl {        \
    namespace hrc {
#define CLOSE_HRC_NAMESPACE \
    }                       \
    }

OPEN_HRC_NAMESPACE

CLOSE_HRC_NAMESPACE

#endif