#ifndef STRIPEMPTYBASICBLOCKPASS_H
#define STRIPEMPTYBASICBLOCKPASS_H

#include "IROptimizationPass.h"
#include "IRProgramStructure.h"

OPEN_IRGEN_NAMESPACE

class StripEmptyBasicBlockPass : public IROptimizationPass {
public:
    StripEmptyBasicBlockPass(const ProgramPtr &program)
        : IROptimizationPass(program)
    {
    }

    ~StripEmptyBasicBlockPass() = default;

protected:
    int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) override;

private:
};

CLOSE_IRGEN_NAMESPACE

#endif