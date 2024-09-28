#ifndef BUILDSSAPASS_H
#define BUILDSSAPASS_H

#include <map>
#include <set>

#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "irgen_global.h"

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
    std::map<ControlFlowVertex, std::set<ControlFlowVertex>> build_dominance_frontiers(const ControlFlowGraphPtr &graph, const ControlFlowVertex &start_block);
    bool verify_dominance_frontiers(
        const ControlFlowGraph &cfg,
        const std::map<ControlFlowVertex, ControlFlowVertex> &dom_tree_map,
        const std::map<ControlFlowVertex, std::set<ControlFlowVertex>> &dominance_frontiers);
};

CLOSE_IRGEN_NAMESPACE

#endif