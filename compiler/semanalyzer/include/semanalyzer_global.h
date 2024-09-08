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

enum SemAnalzyerASTNodeAttributeId : int {
    // range 1000-1999
    ATTR_SEMANALYZER_BEGIN = 1000,
    ATTR_SEMANALYZER_SYMBOL = 1000,
    ATTR_SEMANALYZER_SCOPE_INFO = 1001,
    ATTR_SEMANALYZER_CONST_FOLDING_VALUE = 1002,
    ATTR_SEMANALYZER_END = 1999,
};

CLOSE_SEMANALYZER_NAMESPACE

#endif