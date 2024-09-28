#ifndef BUILDSSAPASS_H
#define BUILDSSAPASS_H

#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "irgen_global.h"
#include <vector>

OPEN_IRGEN_NAMESPACE

class BuildSSAPass : public IROptimizationPass {
public:
    BuildSSAPass(const ProgramPtr &program)
        : IROptimizationPass(program)
    {
    }

    ~BuildSSAPass();

protected:
    int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) override;

private:
    std::vector<std::vector<ControlFlowVertex>> build_dominance_frontiers(const ControlFlowGraphPtr &graph, const ControlFlowVertex &start_block);
};

CLOSE_IRGEN_NAMESPACE

#endif