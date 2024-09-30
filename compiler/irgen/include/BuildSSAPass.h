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
// We're really supposed to perform IN/OUT analysis here when inserting phi
// But I'm too lazy. Removing single branch and zero branch phi seems to be fine here.
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
    // [Group] These are SSA-building steps
    // pair<imm_dom_tree, strict_dom_map>
    std::pair<std::map<ControlFlowVertex, ControlFlowVertex>, std::map<ControlFlowVertex, std::set<ControlFlowVertex>>> build_dominance_tree(
        const ControlFlowGraph &cfg,
        const ControlFlowVertex &start_block);

    std::map<ControlFlowVertex, std::set<ControlFlowVertex>> build_dominance_frontiers(
        const ControlFlowGraph &cfg,
        const ControlFlowVertex &start_block,
        std::map<ControlFlowVertex, ControlFlowVertex> immediate_dom_tree_map,
        std::map<ControlFlowVertex, std::set<ControlFlowVertex>> strict_dom_tree_children);

    bool verify_dominance_frontiers(
        const ControlFlowGraph &cfg,
        const std::map<ControlFlowVertex, ControlFlowVertex> &dom_tree_map,
        const std::map<ControlFlowVertex, std::set<ControlFlowVertex>> &dominance_frontiers);

    void insert_phi_functions(
        const std::map<int, std::set<std::tuple<InstructionListIter, BasicBlockPtr>>> &def_map,
        const std::map<BasicBlockPtr, std::set<BasicBlockPtr>> &dominance_frontiers);

    void populate_phi_function(
        const std::map<int, std::set<std::tuple<InstructionListIter, BasicBlockPtr>>> &def_map,
        // std::map<ControlFlowVertex, std::set<ControlFlowVertex>> strict_dom_tree_children,
        const ControlFlowGraph &cfg,
        const ControlFlowVertex &start_block);

    void remove_redundant_phi(const std::list<BasicBlockPtr> &basic_blocks);

    void rename_registers(const SubroutinePtr &subroutine, const std::map<ControlFlowVertex, ControlFlowVertex> &dom_tree_map);

    void renumber_registers(const SubroutinePtr &subroutine);
    // [End Group]
};

CLOSE_IRGEN_NAMESPACE

#endif