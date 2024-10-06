#include <memory>
#include <vector>

#include <spdlog/spdlog.h>

#include "IROps.h"
#include "MergeConditionalBranchPass.h"
#include "Operand.h"
#include "ThreeAddressCode.h"
#include "hrl_global.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

int MergeConditionalBranchPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    UNUSED(metadata);
    UNUSED(program);

    for (const auto &bb : subroutine->get_basic_blocks()) {
        int rc = run_basic_block(bb);
        if (rc != 0) {
            return rc;
        }
    }

    return 0;
}

int MergeConditionalBranchPass::run_basic_block(const BasicBlockPtr &basic_block)
{
    std::list<TACPtr> &instrs = basic_block->get_instructions();
    auto current = instrs.begin();
    auto next = std::next(current);

    std::vector<InstructionListIter> comparison_instr; // the comparison instr after merging
    while (next != instrs.end()) {
        auto cur_instr = *current;
        auto next_instr = *next;
        // if current is comparison expr and next is jz/jnz

        auto next_op = next_instr->get_op();
        bool is_jz = false, is_jnz = false;
        if (next_op == IROperation::JZ) {
            is_jz = true;
        } else if (next_op == IROperation::JNZ) {
            is_jnz = true;
        }

        auto cur_op = cur_instr->get_op();

        if (IROperationMetadata::is_comparison(cur_op)) {
            IROperation merged_op = IROperation::HALT;

            if (is_jz) {
                // jz is jump the invert of condition
                // e.g.: eq c, a, b; jz not_equal, not_equal is supposed to be reached when a != b
                switch (cur_op) {
                case IROperation::EQ:
                    merged_op = IROperation::JNE;
                    break;
                case IROperation::NE:
                    merged_op = IROperation::JE;
                    break;
                case IROperation::LT:
                    merged_op = IROperation::JGE;
                    break;
                case IROperation::LE:
                    merged_op = IROperation::JGT;
                    break;
                case IROperation::GT:
                    merged_op = IROperation::JLE;
                    break;
                case IROperation::GE:
                    merged_op = IROperation::JLT;
                    break;
                default:
                    spdlog::critical("BUG: incorrect op {} when merging cond branch. {}", static_cast<int>(cur_op), __PRETTY_FUNCTION__);
                    throw;
                }
            } else if (is_jnz) {
                // jnz is jump the condition
                // e.g.: eq c, a, b; jnz equals, equals is supposed to be reached when a == b
                switch (cur_op) {
                case IROperation::EQ:
                    merged_op = IROperation::JE;
                    break;
                case IROperation::NE:
                    merged_op = IROperation::JNE;
                    break;
                case IROperation::LT:
                    merged_op = IROperation::JLT;
                    break;
                case IROperation::LE:
                    merged_op = IROperation::JLE;
                    break;
                case IROperation::GT:
                    merged_op = IROperation::JGT;
                    break;
                case IROperation::GE:
                    merged_op = IROperation::JGE;
                    break;
                default:
                    spdlog::critical("BUG: incorrect op {} when merging cond branch. {}", static_cast<int>(cur_op), __PRETTY_FUNCTION__);
                    throw;
                }
            }

            // perform merge here
            if (is_jz || is_jnz) {
                auto src1 = cur_instr->get_src1();
                auto src2 = cur_instr->get_src2();
                auto tgt = next_instr->get_tgt();

                comparison_instr.push_back(current);
                *next = ThreeAddressCode::create_branching(merged_op, tgt, src1, src2, cur_instr->get_ast_node());
            }
        }

        ++current;
        ++next;
    }

    for (const auto &merged_comparison_instr_iter : comparison_instr) {
        instrs.erase(merged_comparison_instr_iter);
    }

    return 0;
}

CLOSE_IRGEN_NAMESPACE
// end
