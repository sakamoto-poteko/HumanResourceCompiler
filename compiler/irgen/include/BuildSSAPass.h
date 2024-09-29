#ifndef BUILDSSAPASS_H
#define BUILDSSAPASS_H

#include <list>
#include <map>
#include <set>

#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

// Prereq: eliminate dead bb
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
    std::map<ControlFlowVertex, std::set<ControlFlowVertex>> build_dominance_frontiers(
        const ControlFlowGraph &cfg,
        const ControlFlowVertex &start_block);

    bool verify_dominance_frontiers(
        const ControlFlowGraph &cfg,
        const std::map<ControlFlowVertex, ControlFlowVertex> &dom_tree_map,
        const std::map<ControlFlowVertex, std::set<ControlFlowVertex>> &dominance_frontiers);

    void insert_phi_functions(
        const std::map<int, std::set<std::tuple<InstructionListIter, BasicBlockPtr>>> &def_map,
        const std::map<BasicBlockPtr, std::set<BasicBlockPtr>> &dominance_frontiers);

    void remove_single_branch_phi(const std::list<BasicBlockPtr> &basic_blocks);
};

CLOSE_IRGEN_NAMESPACE

#endif