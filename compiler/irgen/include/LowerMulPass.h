#ifndef LOWERMULPASS_H
#define LOWERMULPASS_H

#include "IRGenOptions.h"
#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class LowerMulPass : public IROptimizationPass {
public:
    LowerMulPass(const ProgramPtr program, const IRGenOptions &options)
        : IROptimizationPass(program, options)
    {
    }

    ~LowerMulPass();

protected:
    int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) override;

private:
};

CLOSE_IRGEN_NAMESPACE

#endif