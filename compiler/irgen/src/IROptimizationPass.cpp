#include "IROptimizationPass.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

int IROptimizationPass::run()
{
    int rc = 0;
    for (const auto &subroutine : _program->get_subroutines()) {
        rc = run_subroutine(subroutine, _program->get_metadata(), _program);
        if (rc != 0) {
            return rc;
        }
    }
    return 0;
}

CLOSE_IRGEN_NAMESPACE
// end
