#include "RenumberVariableIdPass.h"
#include "hrl_global.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

RenumberVariableIdPass::~RenumberVariableIdPass()
{
}

int RenumberVariableIdPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    UNUSED(metadata);
    UNUSED(program);

    renumber_registers(subroutine);
    return 0;
}

void RenumberVariableIdPass::renumber_registers(const SubroutinePtr &subroutine)
{
    const std::list<BasicBlockPtr> &basic_blocks = subroutine->get_basic_blocks();
    unsigned int current_number = 0;
    std::map<unsigned int, unsigned int> old_to_new_id_map;

    auto get_new_reg_id = [&current_number, &old_to_new_id_map](unsigned int old_id) {
        auto it = old_to_new_id_map.find(old_id);
        if (it == old_to_new_id_map.end()) {
            unsigned int new_id = current_number++;
            old_to_new_id_map[old_id] = new_id;
            return new_id;
        } else {
            return it->second;
        }
    };

    for (const BasicBlockPtr &basic_block : basic_blocks) {
        for (TACPtr &instr : basic_block->get_instructions()) {
            Operand tgt = instr->get_tgt();
            Operand src1 = instr->get_src1();
            Operand src2 = instr->get_src2();
            bool mutated = false;

            // if assert fails, consider skip phi nodes here
            if (src1.get_type() == Operand::OperandType::VariableId && src1.get_register_id() >= 0) {
                src1 = Operand(get_new_reg_id(src1.get_register_id()));
                mutated = true;
            }

            if (src2.get_type() == Operand::OperandType::VariableId && src2.get_register_id() >= 0) {
                src2 = Operand(get_new_reg_id(src2.get_register_id()));
                mutated = true;
            }

            if (tgt.get_type() == Operand::OperandType::VariableId && tgt.get_register_id() >= 0) {
                tgt = Operand(get_new_reg_id(tgt.get_register_id()));
                mutated = true;
            }

            if (mutated) {
                TACPtr new_instr = ThreeAddressCode::create(instr->get_op(), tgt, src1, src2, instr->get_ast_node());
                new_instr->set_phi_incomings(instr->get_phi_incomings());
                instr = new_instr;
            }
        }

        // process phi name lastly
        for (const BasicBlockPtr &basic_block : basic_blocks) {
            for (TACPtr &instr : basic_block->get_instructions()) {
                if (instr->get_op() == IROperation::PHI) {
                    for (const auto &[income_basic_block, var_id] : instr->get_phi_incomings()) {
                        instr->set_phi_incoming(income_basic_block, get_new_reg_id(var_id));
                    }
                }
            }
        }
    }
}

CLOSE_IRGEN_NAMESPACE
// end
