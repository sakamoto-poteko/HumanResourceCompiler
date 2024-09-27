#ifndef MERGECONDITIONALBRANCHPASS_H
#define MERGECONDITIONALBRANCHPASS_H

#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class MergeConditionalBranchPass : public IROptimizationPass {
public:
    MergeConditionalBranchPass(const ProgramPtr &program)
        : IROptimizationPass(program)
    {
    }

    ~MergeConditionalBranchPass() = default;

protected:
    int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) override;

private:
    int run_basic_block(const BasicBlockPtr &basic_block);
};

CLOSE_IRGEN_NAMESPACE
#endif