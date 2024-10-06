#include <algorithm>
#include <ranges>
#include <vector>

#include <spdlog/spdlog.h>

#include "IROps.h"
#include "IRProgramStructure.h"
#include "Operand.h"
#include "PropagateCopyPass.h"
#include "ThreeAddressCode.h"
#include "hrl_global.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

PropagateCopyPass::~PropagateCopyPass()
{
}

int PropagateCopyPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    UNUSED(metadata);
    UNUSED(program);

    // map<dest, src>
    std::map<unsigned int, std::tuple<unsigned int, BasicBlockPtr>> copied_map;
    visit_dominance_node(subroutine->get_dominance_root(), *subroutine->get_dominance_tree(), copied_map);
    assert(copied_map.empty());

    return 0;
}

void PropagateCopyPass::visit_dominance_node(const BBGraphVertex vertex, const BBGraph &dom_tree, std::map<unsigned int, std::tuple<unsigned int, BasicBlockPtr>> &prev_copied_variable)
{
    std::map<unsigned int, std::tuple<unsigned int, BasicBlockPtr>> copied_variable(prev_copied_variable);

    const BasicBlockPtr &basic_block = dom_tree[vertex];
    for (TACPtr &instr : basic_block->get_instructions()) {
        const Operand &tgt = instr->get_tgt();
        const Operand &src1 = instr->get_src1();
        const Operand &src2 = instr->get_src2();

        if (instr->get_op() == IROperation::MOV && tgt.is_local_register() && src1.is_local_register()) {
            auto src1_prev_copied_it = copied_variable.find(src1.get_register_id());
            // src1 is previously copied to. use the root operand of copy
            if (src1_prev_copied_it != copied_variable.end()) {
                copied_variable.insert_or_assign(tgt.get_register_id(), src1_prev_copied_it->second);
            } else {
                copied_variable[tgt.get_register_id()] = { src1.get_register_id(), basic_block };
            }
        } else if (instr->get_op() != IROperation::PHI) {
            Operand new_src1 = src1, new_src2 = src2;
            bool mutated = false;
            if (src1.is_local_register()) {
                unsigned int src1_id = src1.get_register_id();
                if (auto it = copied_variable.find(src1_id); it != copied_variable.end()) {
                    new_src1 = Operand(std::get<0>(it->second));
                    mutated = true;
                }
            }

            if (src2.is_local_register()) {
                unsigned int src2_id = src2.get_register_id();
                if (auto it = copied_variable.find(src2_id); it != copied_variable.end()) {
                    new_src2 = Operand(std::get<0>(it->second));
                    mutated = true;
                }
            }

            if (mutated) {
                TACPtr new_instr = ThreeAddressCode::create(instr->get_op(), instr->get_tgt(), new_src1, new_src2, instr->get_ast_node());
                spdlog::trace("[PropCopy] Using previously defined register. {} => {}", instr->to_string(true), new_instr->to_string(true));
                instr = new_instr;
            }
        } else if (instr->get_op() == IROperation::PHI) {
            /*
            // Substitute operands based on the environment
            substitutedOperands = []
            for operand in instr.operands:
                if operand in localEnv and localEnv[operand] != operand:
                    substitutedOperands.append(localEnv[operand])
                else:
                    substitutedOperands.append(operand)
            */
            std::vector<unsigned int> phi_incoming_new;
            for (auto &[predecessor, phi_meta] : instr->get_phi_incomings()) {
                auto &[incoming_var_id, var_def_block] = phi_meta;
                if (auto var_in_copied_it = copied_variable.find(incoming_var_id); var_in_copied_it != copied_variable.end() && std::get<0>(var_in_copied_it->second) != incoming_var_id) {
                    // incoming var is found in copied variables
                    // let's get its copy root and def block
                    auto &[prev_moved_var_id, prev_moved_var_def_block] = var_in_copied_it->second;
                    incoming_var_id = prev_moved_var_id;
                    var_def_block = prev_moved_var_def_block;
                    phi_incoming_new.push_back(prev_moved_var_id);
                } else {
                    // this variable is not copied. leave as-is
                    phi_incoming_new.push_back(incoming_var_id);
                }
            }

            /*
            // Check if all operands are identical after substitution
            if allOperandsEqual(substitutedOperands):
                commonOperand = substitutedOperands[0]
                // Replace φ-function result with the common operand
                replace instr with "x = commonOperand"
                // Update the environment to map x to the common operand
                localEnv[x] = commonOperand
            else:
                // If operands differ, map x to itself to indicate no substitution
                localEnv[x] = x
            */
            assert(!phi_incoming_new.empty() && "Phi shouldn't be empty");
            if (std::ranges::all_of(phi_incoming_new, [&phi_incoming_new](unsigned int v) { return v == phi_incoming_new.front(); })) {
                TACPtr new_instr = ThreeAddressCode::create_data_movement(IROperation::MOV, instr->get_tgt(), Operand(phi_incoming_new.front()), Operand(), instr->get_ast_node());
                spdlog::trace("[PropCopy] Phi simplified with all same branches. {} => {}", instr->to_string(true), new_instr->to_string(true));
                instr = new_instr;
            }
        } else {
            // spdlog::trace("[PropCopy] Skipping MOV with global variable: {}", instr->to_string(true));
            spdlog::critical("Unreachable path. {}", __PRETTY_FUNCTION__);
            throw;
        }
    }

    for (BBGraphEdge out_edge : boost::make_iterator_range(boost::out_edges(vertex, dom_tree))) {
        visit_dominance_node(boost::target(out_edge, dom_tree), dom_tree, copied_variable);
    }
}

CLOSE_IRGEN_NAMESPACE
// end
