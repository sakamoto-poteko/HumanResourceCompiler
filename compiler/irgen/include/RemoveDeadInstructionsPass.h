#ifndef REMOVE_DEAD_INSTRUCTION_PASS_H
#define REMOVE_DEAD_INSTRUCTION_PASS_H

#include "IROptimizationPass.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class RemoveDeadInstructionsPass : public IROptimizationPass {
public:
    RemoveDeadInstructionsPass(const ProgramPtr &program, const IRGenOptions &options)
        : IROptimizationPass(program, options)
    {
    }

    ~RemoveDeadInstructionsPass() = default;

protected:
    int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) override;

private:
};

CLOSE_IRGEN_NAMESPACE
#endif