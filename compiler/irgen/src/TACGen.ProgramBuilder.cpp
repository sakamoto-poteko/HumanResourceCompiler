#include <cassert>
#include <iterator>
#include <list>
#include <memory>
#include <string>

#include <boost/range.hpp>
#include <spdlog/spdlog.h>

#include "IROps.h"
#include "IRProgramStructure.h"
#include "Symbol.h"
#include "TACGen.h"
#include "ThreeAddressCode.h"
#include "irgen_global.h"
#include "semanalyzer_global.h"

OPEN_IRGEN_NAMESPACE

std::list<BasicBlockPtr> TACGen::build_subroutine_split_tacs_to_basic_blocks(const std::string &subroutine_name, std::list<TACPtr> &tacs)
{
    int subroutine_block_id = 0;
    std::list<BasicBlockPtr> basic_blocks;

    std::list<TACPtr> current_basic_block;
    std::string current_label;
    bool seen_control_flow = false;

    // build basic blocks first
    for (std::list<TACPtr>::iterator tac_it = tacs.begin(); tac_it != tacs.end(); ++tac_it) {
        auto label_it = _labels.right.find(tac_it);
        // is there a label for this tac? or is there a control flow instr seen?
        // if yes, we need to start a new block
        bool instr_has_lbl = label_it != _labels.right.end();
        if ((instr_has_lbl) || seen_control_flow) {
            // if we're not the first instr, push the old block into the list
            if (!current_basic_block.empty()) {
                basic_blocks.push_back(std::make_shared<BasicBlock>(std::move(current_label), std::move(current_basic_block)));
            }
            current_basic_block.clear();
            current_label.clear();
            seen_control_flow = false;
        }

        // lbl set?
        if (current_label.empty()) {
            // set the new label. either there's a label, or we need to make a new
            if (instr_has_lbl) {
                current_label = label_it->second;
            } else {
                // XB is a random name. I just found it's easier to see
                current_label = subroutine_name + ".XB" + std::to_string(subroutine_block_id);
                subroutine_block_id++;
            }
        }
        // add the instr
        current_basic_block.push_back(*tac_it);

        // mark if this one is control flow
        if (is_control_transfer_operation((*tac_it)->get_op())) {
            seen_control_flow = true;
        }
    }

    if (!current_basic_block.empty()) {
        basic_blocks.push_back(std::make_shared<BasicBlock>(std::move(current_label), std::move(current_basic_block)));
    }

// correctness check:
#ifndef NDEBUG
    int count = 0;
    for (const auto &bb : basic_blocks) {
        count += bb->get_instructions().size();
    }
    assert(count == tacs.size());
#endif // !NDEBUG

    return basic_blocks;
}

ControlFlowGraph TACGen::build_subroutine_link_cfg_from_basic_blocks(std::list<BasicBlockPtr> &basic_blocks)
{
    ControlFlowGraph cfg;
    std::map<std::string, ControlFlowVertex> label_to_block;
    // 1st pass: map label to bb vert
    // 2nd pass: build cfg
    for (auto bb_it = basic_blocks.begin(); bb_it != basic_blocks.end(); ++bb_it) {
        auto vert = cfg.add_vertex(*bb_it);
        label_to_block[(*bb_it)->get_label()] = vert;
    }

    for (auto bb_it = basic_blocks.begin(); bb_it != basic_blocks.end(); ++bb_it) {
        const auto &bb = *bb_it;
        ControlFlowVertex bb_vert = label_to_block[bb->get_label()];

        bool connect_next = false;
        bool connect_target = false;
        std::string target;

        // is the bb empty? if it is, we're connecting to next bb.
        if (bb->get_instructions().empty()) {
            connect_next = true;
            connect_target = false;
        } else {
            // bb is not empty. we'll need to check the final instr
            TACPtr flow_xfer_instr = *std::prev(bb->get_instructions().end());
            switch (flow_xfer_instr->get_op()) {
                // conditional branch
            case HighLevelIROps::JE:
            case HighLevelIROps::JNE:
            case HighLevelIROps::JGT:
            case HighLevelIROps::JLT:
            case HighLevelIROps::JGE:
            case HighLevelIROps::JLE:
            case HighLevelIROps::JZ:
            case HighLevelIROps::JNZ:
                connect_next = true;
                connect_target = true;
                target = flow_xfer_instr->get_tgt().get_label();
                break;

                // branch
            case HighLevelIROps::JMP:
                connect_next = false;
                connect_target = true;
                target = flow_xfer_instr->get_tgt().get_label();
                break;

            case HighLevelIROps::RET:
            case HighLevelIROps::HALT:
                connect_next = false;
                connect_target = false;
                break;

                // nothing yet. not drawing func graph yet.
            case HighLevelIROps::CALL:
            default:
                connect_next = true;
                connect_target = false;
                break;
            }
        }

        if (connect_next) {
            auto next_bb_it = std::next(bb_it);
            if (next_bb_it != basic_blocks.end()) {
                cfg.add_edge(bb_vert, label_to_block[(*next_bb_it)->get_label()]);
            }
        }

        if (connect_target) {
            auto tgt_it = label_to_block.find(target);
            if (tgt_it == label_to_block.end()) {
                spdlog::critical("BUG: target label '{}' does not exist in this block. {}", target, __PRETTY_FUNCTION__);
                throw;
            }
            cfg.add_edge(bb_vert, tgt_it->second);
        }
    }

    return cfg;
}

int TACGen::build_ir_program()
{
    ProgramMetadata metadata;
    std::list<SubroutinePtr> subroutines;

    for (auto &[subroutine_name, tacs] : _subroutine_tacs) {
        // 1. get all BB
        std::list<BasicBlockPtr> basic_blocks = build_subroutine_split_tacs_to_basic_blocks(subroutine_name, tacs);
        // 2. link BB
        ControlFlowGraph cfg = build_subroutine_link_cfg_from_basic_blocks(basic_blocks);
        bool has_param = false;
        bool has_return = false;

        if (subroutine_name == semanalyzer::GLOBAL_SCOPE_ID) {
            // this is global var declaration and floor initialization
            has_param = false;
            has_return = false;
        } else {
            semanalyzer::SymbolPtr function_symbol;
            std::string defined_scope;
            bool ok = _symbol_table->lookup_symbol(semanalyzer::GLOBAL_SCOPE_ID, subroutine_name, false, function_symbol, defined_scope);
            assert(ok);
            assert(function_symbol);
            has_param = function_symbol->has_param();
            has_return = function_symbol->has_return();
        }

        SubroutinePtr subroutine = std::make_shared<Subroutine>(
            subroutine_name,
            has_param,
            has_return,
            basic_blocks,
            cfg);
        subroutines.push_back(subroutine);
    }

    _built_program = std::make_shared<Program>(subroutines, metadata);
    return 0;
}

CLOSE_IRGEN_NAMESPACE
// end
