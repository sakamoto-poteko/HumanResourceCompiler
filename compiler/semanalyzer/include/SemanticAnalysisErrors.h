#ifndef SEMANTICANALYSISERRORS_H
#define SEMANTICANALYSISERRORS_H

#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

enum SemanticAnalysisError : int {
    E_SEMA_SYM_REDEF = 3001,
    E_SEMA_SYM_UNDEFINED = 3002,
    E_SEMA_INT_OVERFLOW = 3003,
    E_SEMA_DIV_MOD_0 = 3004,
    E_SEMA_SUBROUTINE_SIGNATURE_MISMATCH = 3005,
    W_SEMA_VAR_SHADOW_OUTER = 3006,
    E_SEMA_VAR_USE_BEFORE_INIT = 3007,
    W_SEMA_DEAD_CODE = 3008,
    E_SEMA_INVALID_LOOP_CONTROL_CONTEXT = 3009,
    E_SEMA_INVALID_RETURN_CONTEXT = 3010,
    E_SEMA_NOT_ALL_PATH_RETURN_VALUE = 3011,
    E_SEMA_EXPECT_RETURN_VALUE_BUT_NO = 3012,
    E_SEMA_EXPECT_RETURN_NO_VAL_BUT_GIVEN = 3013,
    W_SEMA_VAR_DEFINED_BUT_UNUSED = 3014,
};

CLOSE_SEMANALYZER_NAMESPACE

#endif