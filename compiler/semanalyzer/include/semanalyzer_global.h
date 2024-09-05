#ifndef SEMANALYZER_GLOBAL_H
#define SEMANALYZER_GLOBAL_H

#define OPEN_SEMANALYZER_NAMESPACE \
    namespace hrl {                \
    namespace semanalyzer {

#define CLOSE_SEMANALYZER_NAMESPACE \
    }                               \
    }

#include <cstdio>

#include "hrl_global.h"

OPEN_SEMANALYZER_NAMESPACE

enum class SemAnalzyerASTNodeAttributeId : int {
    // range 1000-1999
    SYMBOL = 1000,
};

CLOSE_SEMANALYZER_NAMESPACE

#endif