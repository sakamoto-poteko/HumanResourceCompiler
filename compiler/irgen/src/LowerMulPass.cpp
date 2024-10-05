#include "LowerMulPass.h"
#include "hrl_global.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

LowerMulPass::~LowerMulPass()
{
}

int LowerMulPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    UNUSED(metadata);
    UNUSED(program);
}

CLOSE_IRGEN_NAMESPACE
// end
