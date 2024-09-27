#ifndef CONTROLFLOWGRAPHBUILDER_H
#define CONTROLFLOWGRAPHBUILDER_H

#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class ControlFlowGraphBuilder : public IROptimizationPass {
public:
    ControlFlowGraphBuilder(const ProgramPtr &program)
        : IROptimizationPass(program)
    {
    }

    ~ControlFlowGraphBuilder();

    int run() override;
    int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) override;

private:
};

CLOSE_IRGEN_NAMESPACE

#endif