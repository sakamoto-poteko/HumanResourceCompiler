#ifndef BUILD_CONTROL_FLOW_GRAPH_PASS_H
#define BUILD_CONTROL_FLOW_GRAPH_PASS_H

#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class BuildControlFlowGraphPass : public IROptimizationPass {
public:
    BuildControlFlowGraphPass(const ProgramPtr &program, const IRGenOptions &options)
        : IROptimizationPass(program, options)
    {
    }

    ~BuildControlFlowGraphPass();

    int run() override;
    int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) override;

protected:
private:
};

CLOSE_IRGEN_NAMESPACE

#endif