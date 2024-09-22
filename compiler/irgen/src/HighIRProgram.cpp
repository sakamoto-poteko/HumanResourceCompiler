#include "HighIRProgram.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

HighIRProgram::HighIRProgram(
    const std::map<std::string, std::list<TACPtr>> &subroutine_ir,
    const label_instr_iter_bimap &label_map)
    : _subroutine_tacs(subroutine_ir)
    , _labels(label_map)
{
}

HighIRProgram::~HighIRProgram()
{
}

CLOSE_IRGEN_NAMESPACE
// end
