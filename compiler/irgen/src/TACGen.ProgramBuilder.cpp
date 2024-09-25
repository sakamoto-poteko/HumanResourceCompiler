#include <cassert>
#include <list>
#include <memory>
#include <ranges>
#include <string>

#include <boost/range.hpp>

#include "ASTNode.h"
#include "IROps.h"
#include "IRProgramStructure.h"
#include "Symbol.h"
#include "TACGen.h"
#include "ThreeAddressCode.h"
#include "hrl_global.h"
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
        if (instr_has_lbl || seen_control_flow) {
            // push the old block into the list
            basic_blocks.push_back(std::make_shared<BasicBlock>(std::move(current_label), std::move(current_basic_block)));
            current_basic_block.clear();
            // set the new label. either there's a label, or we need to make a new
            if (instr_has_lbl) {
                current_label = label_it->second;
            } else {
                // XB is a random name. I just found it's easier to see
                current_label = subroutine_name + ".XB" + std::to_string(subroutine_block_id);
                subroutine_block_id++;
            }
            seen_control_flow = false;
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

int TACGen::build_ir_program()
{
    ProgramMetadata metadata;

    std::list<SubroutinePtr> subroutines;

    for (auto &[subroutine_name, tacs] : _subroutine_tacs) {
        // 1. get all BB
        std::list<BasicBlockPtr> basic_blocks = build_subroutine_split_tacs_to_basic_blocks(subroutine_name, tacs);
        // 2. link BB
        ControlFlowGraph cfg = build_subroutine_link_cfg_from_basic_blocks(basic_blocks);
        semanalyzer::SymbolPtr function_symbol;
        std::string defined_scope;
        bool ok = _symbol_table->lookup_symbol(semanalyzer::GLOBAL_SCOPE_ID, subroutine_name, false, function_symbol, defined_scope);
        SubroutinePtr subroutine = std::make_shared<Subroutine>(
            subroutine_name,
            function_symbol->has_param(),
            function_symbol->has_return(),
            basic_blocks,
            cfg);
        subroutines.push_back(subroutine);
    }
}

CLOSE_IRGEN_NAMESPACE
// end
