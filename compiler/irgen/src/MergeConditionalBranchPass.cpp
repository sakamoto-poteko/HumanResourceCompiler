#include "MergeConditionalBranchPass.h"
#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

MergeConditionalBranchPass::~MergeConditionalBranchPass()
{
}

int MergeConditionalBranchPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    for (const auto &bb : subroutine->get_basic_blocks()) {
        int rc = run_basic_block(bb);
        if (rc != 0) {
            return rc;
        }
    }

    return 0;
}

int MergeConditionalBranchPass::run_basic_block(const BasicBlockPtr &basic_block)
{
    std::list<TACPtr> instrs = basic_block->get_instructions();
    auto current = instrs.begin();
    auto next = std::next(current);

    while (next != instrs.end()) {
        // if current is comparison expr and next is jz/jnz

        ++current;
        ++next;
    }
}

CLOSE_IRGEN_NAMESPACE
// end
