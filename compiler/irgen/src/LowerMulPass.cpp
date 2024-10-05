#include "LowerMulPass.h"
#include "IROps.h"
#include "IRProgramStructure.h"
#include "ThreeAddressCode.h"
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

    bool opt_for_speed = false;

    switch (_options.ModulusLowering) {
    case IROptimizationFor::NoOpt:
        return 0;
    case IROptimizationFor::OptForSpeed:
        opt_for_speed = true;
        break;
    case IROptimizationFor::OptForCodeSize:
        opt_for_speed = false;
        break;
    }

    subroutine->get_max_reg_id();
    for (const BasicBlockPtr &basic_block : subroutine->get_basic_blocks()) {
        for (TACPtr &instr : basic_block->get_instructions()) {
            if (instr->get_op() == IROperation::MUL) {
                if (opt_for_speed) {

                } else {
                }
            }
        }
    }
}

CLOSE_IRGEN_NAMESPACE
// end
