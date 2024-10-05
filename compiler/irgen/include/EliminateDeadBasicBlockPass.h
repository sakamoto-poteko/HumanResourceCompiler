#ifndef DEADBASICBLOCKELIMINATOR_H
#define DEADBASICBLOCKELIMINATOR_H

#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class EliminateDeadBasicBlockPass : public IROptimizationPass {
public:
    EliminateDeadBasicBlockPass(const ProgramPtr &program, const IRGenOptions &options)
        : IROptimizationPass(program, options)
    {
    }

    ~EliminateDeadBasicBlockPass();

protected:
    int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) override;

private:
};

CLOSE_IRGEN_NAMESPACE

#endif