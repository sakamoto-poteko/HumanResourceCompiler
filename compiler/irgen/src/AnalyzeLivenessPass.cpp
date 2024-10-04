#include <algorithm>
#include <cstddef>
#include <iterator>
#include <set>

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptors.hpp>
#include <spdlog/spdlog.h>

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

int AnalyzeLivenessPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    UNUSED(metadata);
    UNUSED(program);

    const ControlFlowGraph &cfg = *subroutine->get_cfg();
    std::set<ControlFlowVertex> cfg_visited;
    std::vector<ControlFlowVertex> visit_order;
    traverse_cfg(subroutine->get_start_block(), cfg, cfg_visited, visit_order);
    // std::ranges::reverse(rpo);

    calculate_def_use(subroutine);
    calculate_in_out(cfg, visit_order);

    for (const BasicBlockPtr &bb : subroutine->get_basic_blocks()) {
        spdlog::trace(
            "[Liveness] In BB '{}', IN = %{{{}}}",
            bb->get_label(),
            boost::join(bb->get_in_variables() | boost::adaptors::transformed([](const unsigned int v) { return std::to_string(v); }), ","));

        spdlog::trace(
            "[Liveness] In BB '{}', OUT = %{{{}}}",
            bb->get_label(),
            boost::join(bb->get_out_variables() | boost::adaptors::transformed([](const unsigned int v) { return std::to_string(v); }), ","));

        spdlog::trace(
            "[Liveness] In BB '{}', DEF = %{{{}}}",
            bb->get_label(),
            boost::join(bb->get_def_variables() | boost::adaptors::transformed([](const unsigned int v) { return std::to_string(v); }), ","));

        spdlog::trace(
            "[Liveness] In BB '{}', USE = %{{{}}}",
            bb->get_label(),
            boost::join(bb->get_use_variables() | boost::adaptors::transformed([](const unsigned int v) { return std::to_string(v); }), ","));
    }

    return 0;
}

void AnalyzeLivenessPass::traverse_cfg(const ControlFlowVertex vertex, const ControlFlowGraph &cfg, std::set<ControlFlowVertex> &visited, std::vector<ControlFlowVertex> &result)
{
    if (visited.contains(vertex)) {
        return;
    }
    visited.insert(vertex);

    for (const ControlFlowEdge edge : boost::make_iterator_range(boost::out_edges(vertex, cfg))) {
        traverse_cfg(boost::target(edge, cfg), cfg, visited, result);
    }

    result.push_back(vertex);
}

void AnalyzeLivenessPass::calculate_in_out(const ControlFlowGraph &cfg, const std::vector<ControlFlowVertex> &vertices)
{
    for (const ControlFlowVertex vert : vertices) {
        cfg[vert]->get_in_variables().clear();
        cfg[vert]->get_out_variables().clear();
    }

    /*
    // Iterative Fixed-Point Computation using RPO
    // Input: Control Flow Graph (CFG) with blocks
    // Each block has a GEN and KILL set
    // Output: IN[block] and OUT[block] for each block in the CFG

    initialize IN[block] = ∅ for all blocks
    initialize OUT[block] = ∅ for all blocks

    // Iterate until no changes occur in any IN or OUT set
    do
        changed = false

        // For each block in reverse order (from end to start)
        for each block in CFG do
            // Save the old IN set for comparison
            old_IN = IN[block]

            // Calculate the OUT set as the union of IN sets of successor blocks
            OUT[block] = ∅
            for each successor in successors(block) do
                OUT[block] = OUT[block] ∪ IN[successor]
            end for

            // Calculate the new IN set using the equation:
            IN[block] = GEN[block] ∪ (OUT[block] - KILL[block])

            // If IN[block] has changed, set changed to true
            if IN[block] ≠ old_IN then
                changed = true
            end if
        end for

    // Repeat until no changes in IN and OUT sets
    while changed

    // After the loop, IN and OUT sets are stabilized
    */

    bool changed = true;
    while (changed) {
        changed = false;

        for (const ControlFlowVertex vert : vertices) {
            const BasicBlockPtr &block = cfg[vert];

            const std::set<unsigned int> &block_use = block->get_use_variables();
            const std::set<unsigned int> &block_def = block->get_def_variables();
            std::set<unsigned int> old_IN = block->get_in_variables();
            std::set<unsigned int> old_OUT = block->get_out_variables();
            std::set<unsigned int> new_IN;
            std::set<unsigned int> new_OUT;

            // Compute OUT[B] as union of IN sets of successors
            for (const ControlFlowEdge out_edge : boost::make_iterator_range(boost::out_edges(vert, cfg))) {
                // OUT[block] = OUT[block] ∪ IN[successor]
                const BasicBlockPtr &successor = cfg[boost::target(out_edge, cfg)];
                const std::set<unsigned int> &successor_IN = successor->get_in_variables();
                new_OUT.insert(successor_IN.begin(), successor_IN.end());
            }

            // Compute IN[B] as USE[B] ∪ (OUT[B] - DEF[B])
            // USE[B]
            new_IN.insert(block_use.begin(), block_use.end());

            // OUT[B] - DEF[B]
            std::set<unsigned int> set_diff;
            std::ranges::set_difference(new_OUT, block_def, std::inserter(set_diff, set_diff.begin()));

            // USE[B] ∪ (OUT[B] - DEF[B])
            new_IN.insert(set_diff.begin(), set_diff.end());

            std::set<unsigned int> in_diff, out_diff;
            std::ranges::set_symmetric_difference(old_IN, new_IN, std::inserter(in_diff, in_diff.begin()));
            std::ranges::set_symmetric_difference(old_OUT, new_OUT, std::inserter(out_diff, out_diff.begin()));

            block->get_in_variables().swap(new_IN);
            block->get_out_variables().swap(new_OUT);

            if (!in_diff.empty() || !out_diff.empty()) {
                changed = true;
            }
        }
    }
}

void AnalyzeLivenessPass::calculate_def_use(const SubroutinePtr &subroutine)
{
    std::map<unsigned int, std::set<BasicBlockPtr>> subroutine_def_map;
    std::map<unsigned int, std::set<BasicBlockPtr>> subroutine_use_map;

    for (const BasicBlockPtr &basic_block : subroutine->get_basic_blocks()) {
        std::set<unsigned int> blk_def_set, blk_use_set;

        for (const TACPtr &instruction : basic_block->get_instructions()) {
            const Operand &tgt = instruction->get_tgt();
            const Operand &src1 = instruction->get_src1();
            const Operand &src2 = instruction->get_src2();

            if (src1.get_type() == Operand::OperandType::VariableId && src1.get_register_id() >= 0) {
                if (unsigned int reg = src1.get_register_id(); !blk_def_set.contains(reg)) {
                    subroutine_use_map[reg].insert(basic_block);
                    blk_use_set.insert(reg);
                }
            }

            if (src2.get_type() == Operand::OperandType::VariableId && src2.get_register_id() >= 0) {
                if (unsigned int reg = src2.get_register_id(); !blk_def_set.contains(reg)) {
                    subroutine_use_map[reg].insert(basic_block);
                    blk_use_set.insert(reg);
                }
            }

            if (tgt.get_type() == Operand::OperandType::VariableId && tgt.get_register_id() >= 0) {
                subroutine_def_map[tgt.get_register_id()].insert(basic_block);
                blk_def_set.insert(tgt.get_register_id());
            }
        }

        basic_block->get_def_variables().swap(blk_def_set);
        basic_block->get_use_variables().swap(blk_use_set);
    }

    subroutine->get_def_variables().swap(subroutine_def_map);
    subroutine->get_use_variables().swap(subroutine_use_map);
}

CLOSE_IRGEN_NAMESPACE
// end
