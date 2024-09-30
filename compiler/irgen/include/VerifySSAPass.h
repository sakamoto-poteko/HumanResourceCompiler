#ifndef VERIFYSSAPASS_H
#define VERIFYSSAPASS_H

#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class VerifySSAPass : public IROptimizationPass {
public:
    VerifySSAPass(const ProgramPtr &program)
        : IROptimizationPass(program)
    {
    }

    ~VerifySSAPass();

protected:
    int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) override;

private:
};

CLOSE_IRGEN_NAMESPACE

#endif