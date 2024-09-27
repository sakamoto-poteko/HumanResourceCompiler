#include "StripUselessInstructionPass.h"
#include "IROps.h"
#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

int StripUselessInstructionPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    const auto &bbs = subroutine->get_basic_blocks();
    for (const auto &bb : bbs) {
        auto &instrs = bb->get_instructions();

        if (!subroutine->has_param()) {
            instrs.remove_if([](const TACPtr &instr) { return instr->get_op() == IROperation::ENTER; });
        }

        instrs.remove_if([](const TACPtr &instr) { return instr->get_op() == IROperation::NOP; });
    }
    return 0;
}

CLOSE_IRGEN_NAMESPACE
// end
