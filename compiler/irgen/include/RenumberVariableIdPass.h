#ifndef RENUMBERVARIABLEIDPASS_H
#define RENUMBERVARIABLEIDPASS_H

#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class RenumberVariableIdPass : public IROptimizationPass {
public:
    RenumberVariableIdPass(const ProgramPtr &program)
        : IROptimizationPass(program)
    {
    }

    ~RenumberVariableIdPass();

protected:
    int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) override;

private:
    void renumber_registers(const SubroutinePtr &subroutine);
};

CLOSE_IRGEN_NAMESPACE

#endif