#ifndef IRGEN_GLOBAL_H
#define IRGEN_GLOBAL_H

#define OPEN_IRGEN_NAMESPACE \
    namespace hrl {          \
    namespace irgen {
#define CLOSE_IRGEN_NAMESPACE \
    }                         \
    }

OPEN_IRGEN_NAMESPACE

constexpr int DEFAULT_FLOOR_MAX = 64;

CLOSE_IRGEN_NAMESPACE

#endif