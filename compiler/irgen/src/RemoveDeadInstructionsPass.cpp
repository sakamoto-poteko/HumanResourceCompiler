#include "RemoveDeadInstructionsPass.h"
#include "IRGenOptions.h"
#include "IROps.h"
#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

int RemoveDeadInstructionsPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    UNUSED(metadata);
    UNUSED(program);

    const std::list<BasicBlockPtr> &basic_blocks = subroutine->get_basic_blocks();
    for (const BasicBlockPtr &basic_block : basic_blocks) {
        std::list<TACPtr> &instrs = basic_block->get_instructions();

        instrs.remove_if([&](const TACPtr &instr) {
            switch (instr->get_op()) {
            case IROperation::MOV:
            case IROperation::LOAD:
            case IROperation::STORE:
            case IROperation::LOADI:
            case IROperation::ADD:
            case IROperation::SUB:
            case IROperation::MUL:
            case IROperation::DIV:
            case IROperation::MOD:
            case IROperation::NEG:
            case IROperation::AND:
            case IROperation::OR:
            case IROperation::NOT:
            case IROperation::EQ:
            case IROperation::NE:
            case IROperation::LT:
            case IROperation::LE:
            case IROperation::GT:
            case IROperation::GE:
            case IROperation::JE:
            case IROperation::JNE:
            case IROperation::JGT:
            case IROperation::JLT:
            case IROperation::JGE:
            case IROperation::JLE:
            case IROperation::JZ:
            case IROperation::JNZ:
            case IROperation::JMP:
            case IROperation::CALL:
            case IROperation::RET:
            case IROperation::INPUT:
            case IROperation::OUTPUT:
            case IROperation::HALT:
            case IROperation::PHI:
                return false;
            case IROperation::ENTER:
                // Strip useless enter
                return _options.EliminateEnter >= IROptimizationFor::OptForSpeed && !subroutine->has_param();
            case IROperation::NOP:
                // Strip nop
                return _options.EliminateNop >= IROptimizationFor::OptForSpeed && true;
            }
        });

        // Find out the useless var defined
    }

    return 0;
}

CLOSE_IRGEN_NAMESPACE
// end
