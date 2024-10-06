#include <algorithm>
#include <iterator>
#include <ranges>

#include <spdlog/spdlog.h>

#include "IRGenOptions.h"
#include "IROps.h"
#include "RemoveDeadInstructionsPass.h"
#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

int RemoveDeadInstructionsPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    UNUSED(metadata);
    UNUSED(program);

    // Dead code elimination:
    // Pg.24 of https://www.cs.princeton.edu/courses/archive/spring15/cos320/lectures/13-SSA.pdf

    bool enable_dead_assignment_elimination = subroutine->is_ssa();
    if (!enable_dead_assignment_elimination) {
        spdlog::debug("[RmDeadInstr] IR for subroutine '{}' is not SSA. Skipping dead assignment elimination.", subroutine->get_func_name());
    }

    std::set<unsigned int> defined_vars;
    std::set<unsigned int> used_vars;

    const std::list<BasicBlockPtr> &basic_blocks = subroutine->get_basic_blocks();
    for (const BasicBlockPtr &basic_block : basic_blocks) {
        std::list<TACPtr> &instrs = basic_block->get_instructions();

        instrs.remove_if([&](const TACPtr &instr) {
            const IROperation op = instr->get_op();

            if (enable_dead_assignment_elimination) {
                if (!IROperationMetadata::has_side_effect(op) && instr->get_tgt().is_local_register()) {
                    defined_vars.insert(instr->get_tgt().get_register_id());
                }

                if (instr->get_src1().is_local_register()) {
                    used_vars.insert(instr->get_src1().get_register_id());
                }

                if (instr->get_src2().is_local_register()) {
                    used_vars.insert(instr->get_src2().get_register_id());
                }

                if (op == IROperation::PHI) {
                    auto phi_incoming_uses = instr->get_phi_incomings() | std::views::transform([](const auto &phi_pair) {
                        auto &[var_id, _] = phi_pair.second;
                        return var_id;
                    });
                    used_vars.insert(phi_incoming_uses.begin(), phi_incoming_uses.end());
                }
            }

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
        // Find out the useless var defined
        std::set<unsigned int> unused_var;
        std::ranges::set_difference(defined_vars, used_vars, std::inserter(unused_var, unused_var.begin()));

        for (const BasicBlockPtr &basic_block : basic_blocks) {
            // remove unused var
            basic_block->get_instructions().remove_if([&](const TACPtr &instruction) {
                bool dead = instruction->get_tgt().is_local_register() && unused_var.contains(instruction->get_tgt().get_register_id());
                if (dead) {
                    spdlog::trace("[RmDeadInstr] '{}' is never used. Removing this instruction.", instruction->to_string(true));
                    assert(!IROperationMetadata::has_side_effect(instruction->get_op()));
                    return true;
                } else {
                    return false;
                }
            });
        }
    }

    return 0;
}

CLOSE_IRGEN_NAMESPACE
// end
