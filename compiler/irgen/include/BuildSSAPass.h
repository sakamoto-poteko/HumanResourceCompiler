#ifndef BUILDSSAPASS_H
#define BUILDSSAPASS_H

#include <map>
#include <set>
#include <string>

#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

// Depends: CFG, Liveness
class BuildSSAPass : public IROptimizationPass {
public:
    BuildSSAPass(const ProgramPtr &program)
        : IROptimizationPass(program)
    {
    }

    ~BuildSSAPass();

    std::string get_additional_metadata_text(unsigned int task_index, const std::string &path) override;

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
        const std::map<unsigned int, std::set<BasicBlockPtr>> &def_map,
        const std::map<BasicBlockPtr, std::set<BasicBlockPtr>> &dominance_frontiers);

    void rename_and_populate_phi(
        const std::map<unsigned int, std::set<BasicBlockPtr>> &def_map,
        const std::map<ControlFlowVertex, std::set<ControlFlowVertex>> &strict_dom_tree_children,
        const ControlFlowGraph &cfg,
        ControlFlowVertex entry);

    // [End Group]

    std::map<SubroutinePtr, ControlFlowGraph> _dominance_trees;
    std::string generate_dominance_tree_graphviz();
};

CLOSE_IRGEN_NAMESPACE

#endif