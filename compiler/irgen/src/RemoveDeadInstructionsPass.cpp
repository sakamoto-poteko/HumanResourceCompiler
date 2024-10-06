#include <iterator>
#include <list>
#include <ranges>

#include <spdlog/spdlog.h>

#include "IRGenOptions.h"
#include "IROps.h"
#include "IRProgramStructure.h"
#include "RemoveDeadInstructionsPass.h"
#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

void RemoveDeadInstructionsPass::dce_dom_tree_visit(const BBGraphVertex vertex, const BBGraph &dom_tree, std::set<unsigned int> &live_variables)
{
    const BasicBlockPtr &basic_block = dom_tree[vertex];
    std::list<TACPtr> &instructions = basic_block->get_instructions();

    // Traverse children
    for (const BBGraphEdge out_edge : boost::make_iterator_range(boost::out_edges(vertex, dom_tree))) {
        dce_dom_tree_visit(boost::target(out_edge, dom_tree), dom_tree, live_variables);
    }

    // Mark other variables reversely, and remove unneeded instruction
    for (auto instr_rit = instructions.rbegin(); instr_rit != instructions.rend();) {
        const TACPtr &instruction = *instr_rit;

        if (IROperationMetadata::has_side_effect(instruction->get_op()) && instruction->get_tgt().is_local_register()) {
            live_variables.insert(instruction->get_tgt().get_register_id());
        }

        if (instruction->get_src1().is_local_register()) {
            live_variables.insert(instruction->get_src1().get_register_id());
        }

        if (instruction->get_src2().is_local_register()) {
            live_variables.insert(instruction->get_src2().get_register_id());
        }

        if (instruction->get_tgt().is_local_register() && !live_variables.contains(instruction->get_tgt().get_register_id())) {
            spdlog::trace("[RmDeadInstr] Removing '{}'.", instruction->to_string(true));
            assert(!IROperationMetadata::has_side_effect(instruction->get_op()));
            instr_rit = std::list<TACPtr>::reverse_iterator(instructions.erase(std::next(instr_rit).base()));
        } else {
            ++instr_rit;
        }
    }
}

int RemoveDeadInstructionsPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    UNUSED(metadata);
    UNUSED(program);

    // Dead code elimination:
    // Pg.24 of https://www.cs.princeton.edu/courses/archive/spring15/cos320/lectures/13-SSA.pdf

    // Opt for speed, opt for code size, either ways it's eliminate
    bool enable_dead_assignment_elimination = subroutine->is_ssa() && _options.EliminateDeadAssignment != IROptimizationFor::NoOpt;
    if (!subroutine->is_ssa()) {
        spdlog::debug("[RmDeadInstr] IR for subroutine '{}' is not SSA. Skipping dead assignment elimination.", subroutine->get_func_name());
    }
    if (enable_dead_assignment_elimination) {
        spdlog::debug("[RmDeadInstr] Enabled dead assignment elimination for subroutine '{}'.", subroutine->get_func_name());
    }

    const std::list<BasicBlockPtr> &basic_blocks = subroutine->get_basic_blocks();
    for (const BasicBlockPtr &basic_block : basic_blocks) {
        std::list<TACPtr> &instrs = basic_block->get_instructions();

        instrs.remove_if([&](const TACPtr &instr) {
            const IROperation op = instr->get_op();

            // Pass 1: eliminate
            if (op == IROperation::ENTER) {
                // Strip useless enter
                return _options.EliminateEnter >= IROptimizationFor::OptForSpeed && !subroutine->has_param();
            }

            if (op == IROperation::NOP) {
                return _options.EliminateNop >= IROptimizationFor::OptForSpeed && true;
            }

            return false;
        });
    } // end pass 1 loop for BB

    if (enable_dead_assignment_elimination) {
        std::set<unsigned int> live_variables;

        // Mark phi incoming as live first, as they are out of dominance tree order
        for (const BasicBlockPtr &basic_block : subroutine->get_basic_blocks()) {
            for (const TACPtr &instruction : basic_block->get_instructions()) {
                if (instruction->get_op() == IROperation::PHI) {
                    auto phi_incoming_uses = instruction->get_phi_incomings() | std::views::transform([](const auto &phi_pair) {
                        return std::get<0>(phi_pair.second);
                    });
                    live_variables.insert(phi_incoming_uses.begin(), phi_incoming_uses.end());
                }
            }
        }

        dce_dom_tree_visit(subroutine->get_dominance_root(), *subroutine->get_dominance_tree(), live_variables);
    }

    return 0;
}

CLOSE_IRGEN_NAMESPACE
// end
