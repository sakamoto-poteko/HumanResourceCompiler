#include <spdlog/spdlog.h>

#include "IROps.h"
#include "IRProgramStructure.h"
#include "Operand.h"
#include "ThreeAddressCode.h"
#include "VerifySSAPass.h"
#include "hrl_global.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

int VerifySSAPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    UNUSED(metadata);
    UNUSED(program);

    std::set<unsigned int> variable_assigned;
    std::set<ControlFlowVertex> pass1, pass2;

    visit_basic_block(subroutine->get_start_block(), *subroutine->get_cfg(), pass1, [&](const BasicBlockPtr &basic_block) {
        verify_basic_block_assignments_and_uses(basic_block, variable_assigned);
    });

    visit_basic_block(subroutine->get_start_block(), *subroutine->get_cfg(), pass2, [&](const BasicBlockPtr &basic_block) {
        verify_basic_block_phi_incoming_branches(basic_block, variable_assigned);
    });

    return 0;
}

void VerifySSAPass::verify_basic_block_assignments_and_uses(const BasicBlockPtr &basic_block, std::set<unsigned int> &variable_assigned)
{
    for (const TACPtr &instruction : basic_block->get_instructions()) {
        // 1. it's assigned to a new variable
        const Operand &tgt = instruction->get_tgt();
        if (tgt.get_type() == Operand::OperandType::VariableId) {
            int tgt_var_id = tgt.get_register_id();
            if (tgt_var_id >= 0) {
                auto _var_asgn_it = variable_assigned.find(tgt_var_id);
                if (_var_asgn_it != variable_assigned.end()) {
                    spdlog::error("Variable '%{}' in block '{}' is defined already. This is likely the bug of SSA generator. Report this bug.", tgt_var_id, basic_block->get_label());
                    throw;
                } else {
                    variable_assigned.insert(tgt_var_id);
                }
            }
        }

        // 2. the sources are assigned
        auto check_operand = [&](const Operand &operand) {
            unsigned int var_id = operand.get_register_id();
            if (!variable_assigned.contains(var_id)) {
                spdlog::error("Variable '%{}' in block '{}' is used before assignment. This is likely the bug of SSA generator. Report this bug.", var_id, basic_block->get_label());
                throw;
            }
        };

        const Operand &src1 = instruction->get_src1();
        if (src1.get_type() == Operand::OperandType::VariableId && src1.get_register_id() >= 0) {
            check_operand(src1);
        }

        const Operand &src2 = instruction->get_src2();
        if (src2.get_type() == Operand::OperandType::VariableId && src2.get_register_id() >= 0) {
            check_operand(src2);
        }
    }
}

void VerifySSAPass::verify_basic_block_phi_incoming_branches(const BasicBlockPtr &basic_block, const std::set<unsigned int> &variable_assigned)
{
    for (const TACPtr &instruction : basic_block->get_instructions()) {
        if (instruction->get_op() == IROperation::PHI) {
            for (const auto &[basic_block, var_id] : instruction->get_phi_incomings()) {
                if (!variable_assigned.contains(var_id)) {
                    spdlog::error("Variable '%{}' in phi incoming of block '{}' is used before assignment. This is likely the bug of SSA generator. Report this bug.", var_id, basic_block->get_label());
                    throw;
                }
            }
        }
    }
}

CLOSE_IRGEN_NAMESPACE
// end
