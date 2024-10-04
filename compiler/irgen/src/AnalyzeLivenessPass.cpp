#include <algorithm>
#include <cstddef>

#include <boost/range/adaptors.hpp>
#include <iterator>
#include <set>

#include "AnalyzeLivenessPass.h"
#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "Operand.h"
#include "ThreeAddressCode.h"
#include "hrl_global.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

AnalyzeLivenessPass::AnalyzeLivenessPass(const ProgramPtr &program)
    : IROptimizationPass(program)
{
}

AnalyzeLivenessPass::~AnalyzeLivenessPass()
{
}

int AnalyzeLivenessPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    UNUSED(metadata);
    UNUSED(program);

    const ControlFlowGraph &cfg = *subroutine->get_cfg();
    std::set<ControlFlowVertex> rpo_visited;
    std::vector<ControlFlowVertex> rpo;
    traverse_cfg(subroutine->get_start_block(), cfg, rpo_visited, rpo);
    std::reverse(rpo.begin(), rpo.end());

    calculate_def_use(subroutine->get_basic_blocks());
    calculate_in_out(cfg, rpo);
}

void AnalyzeLivenessPass::traverse_cfg(ControlFlowVertex vertex, const ControlFlowGraph &cfg, std::set<ControlFlowVertex> &visited, std::vector<ControlFlowVertex> &result)
{
    if (visited.contains(vertex)) {
        return;
    }
    visited.insert(vertex);

    for (ControlFlowEdge edge : boost::make_iterator_range(boost::out_edges(vertex, cfg))) {
        traverse_cfg(boost::target(edge, cfg), cfg, visited, result);
    }

    result.push_back(vertex);
}

void AnalyzeLivenessPass::calculate_in_out(const ControlFlowGraph &cfg, const std::vector<ControlFlowVertex> &rpo)
{
    for (ControlFlowVertex vert : rpo) {
        cfg[vert]->get_in_variables().clear();
        cfg[vert]->get_out_variables().clear();
    }

    /*
    // Iterative Fixed-Point Computation using RPO
    changed = true
    while changed:
        changed = false
        // Process blocks in RPO
        for block in rpo:
            old_IN = IN[block]
            old_OUT = OUT[block]

            // Compute OUT[B] as union of IN sets of successors
            new_OUT = empty set
            for successor in CFG.successors(block):
                new_OUT = new_OUT ∪ IN[successor]
            OUT[block] = new_OUT

            // Compute IN[B] as USE[B] ∪ (OUT[B] - DEF[B])
            new_IN = USE[block] ∪ (OUT[block] - DEF[block])
            IN[block] = new_IN

            // Check if IN or OUT has changed
            if old_IN ≠ IN[block] or old_OUT ≠ OUT[block]:
                changed = true
    */

    bool changed = true;
    while (changed) {
        changed = false;

        // Process blocks in RPO
        for (ControlFlowVertex vert : rpo) {
            const BasicBlockPtr &block = cfg[vert];

            std::set<unsigned int> old_IN = block->get_in_variables();
            std::set<unsigned int> old_OUT = block->get_out_variables();

            // Compute OUT[B] as union of IN sets of successors
            std::set<unsigned int> new_OUT;
            for (ControlFlowEdge out_edge : boost::make_iterator_range(boost::out_edges(vert, cfg))) {
                // OUT[block] = union(IN[s] for s in CFG.successors(block))
                const BasicBlockPtr &successor = cfg[boost::target(out_edge, cfg)];
                std::size_t new_OUT_prev_size = new_OUT.size();
                new_OUT.insert(successor->get_in_variables().begin(), successor->get_in_variables().end());
            }
            block->get_out_variables().swap(new_OUT);

            // Compute IN[B] as USE[B] ∪ (OUT[B] - DEF[B])

            // this is new out
            const std::set<unsigned int> &block_out = block->get_out_variables();
            const auto &block_use = block->get_use_variables();
            const auto &block_def = block->get_use_variables();

            std::set<unsigned int> new_IN;
            // USE[B]
            new_IN.insert(boost::adaptors::keys(block_use).begin(), boost::adaptors::keys(block_use).end());

            // OUT[B] - DEF[B]
            std::set<unsigned int> set_diff;
            std::set_difference(
                block_out.begin(), block_out.end(),
                boost::adaptors::keys(block_def).begin(), boost::adaptors::keys(block_def).end(),
                std::inserter(set_diff, set_diff.begin()));

            // USE[B] ∪ (OUT[B] - DEF[B])
            new_IN.insert(set_diff.begin(), set_diff.end());
            block->get_in_variables().swap(new_IN);

            std::set<unsigned int> in_diff, out_diff;
            std::set_symmetric_difference(
                old_IN.begin(), old_IN.end(),
                block->get_in_variables().begin(), block->get_in_variables().end(),
                std::inserter(in_diff, in_diff.begin()));
            std::set_symmetric_difference(
                old_OUT.begin(), old_OUT.end(),
                block->get_out_variables().begin(), block->get_out_variables().end(),
                std::inserter(out_diff, out_diff.begin()));
            if (!in_diff.empty() || !out_diff.empty()) {
                changed = true;
            }
        }
    }
}

void AnalyzeLivenessPass::calculate_def_use(const std::list<BasicBlockPtr> &basic_blocks)
{
    for (const BasicBlockPtr &basic_block : basic_blocks) {
        std::map<unsigned int, std::set<BasicBlockPtr>> def_map;
        std::map<unsigned int, std::set<BasicBlockPtr>> use_map;

        for (const TACPtr &instruction : basic_block->get_instructions()) {
            const Operand &tgt = instruction->get_tgt();
            const Operand &src1 = instruction->get_src1();
            const Operand &src2 = instruction->get_src2();

            if (tgt.get_type() == Operand::OperandType::VariableId && tgt.get_register_id() >= 0) {
                def_map[tgt.get_register_id()].insert(basic_block);
            }

            if (src1.get_type() == Operand::OperandType::VariableId && src1.get_register_id() >= 0) {
                use_map[src1.get_register_id()].insert(basic_block);
            }

            if (src1.get_type() == Operand::OperandType::VariableId && src1.get_register_id() >= 0) {
                use_map[src1.get_register_id()].insert(basic_block);
            }
        }

        basic_block->get_def_variables().swap(def_map);
        basic_block->get_use_variables().swap(use_map);
    }
}

CLOSE_IRGEN_NAMESPACE
// end
