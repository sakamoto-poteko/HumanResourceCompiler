#include <cassert>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <string>

#include <boost/range.hpp>

#include "IROps.h"
#include "IRProgramStructure.h"
#include "TACGen.h"
#include "TerminalColor.h"
#include "ThreeAddressCode.h"
#include "irgen_global.h"
#include "semanalyzer_global.h"

OPEN_IRGEN_NAMESPACE

int TACGen::run()
{
    int rc = visit(_root);
    return rc;
}

int TACGen::take_var_id_numbering()
{
    return _current_subroutine_var_id++;
}

std::string TACGen::take_block_label()
{
    std::string result(_current_subroutine_name + ".B" + std::to_string(_current_block_label_id));
    ++_current_block_label_id;
    return result;
}

std::string TACGen::take_block_label(const std::string &msg)
{
    std::string result(_current_subroutine_name + ".B" + std::to_string(_current_block_label_id) + "_" + msg);
    ++_current_block_label_id;
    return result;
}

std::list<TACPtr>::iterator TACGen::create_noop(const parser::ASTNodePtr &node)
{
    _current_subroutine_tac.emplace_back(ThreeAddressCode::create_special(HighLevelIROps::NOP, node));
    return std::prev(_current_subroutine_tac.end());
}

std::list<TACPtr>::iterator TACGen::create_jmp(const std::string &label, const parser::ASTNodePtr &node)
{
    _current_subroutine_tac.emplace_back(ThreeAddressCode::create_branching(Operand(label), node));
    return std::prev(_current_subroutine_tac.end());
}

std::list<TACPtr>::iterator TACGen::create_jnz(const Operand &operand, const std::string &label, const parser::ASTNodePtr &node)
{
    _current_subroutine_tac.emplace_back(ThreeAddressCode::create_branching(HighLevelIROps::JNZ, Operand(label), operand, node));
    return std::prev(_current_subroutine_tac.end());
}

std::list<TACPtr>::iterator TACGen::create_jz(const Operand &operand, const std::string &label, const parser::ASTNodePtr &node)
{
    _current_subroutine_tac.emplace_back(ThreeAddressCode::create_branching(HighLevelIROps::JZ, Operand(label), operand, node));
    return std::prev(_current_subroutine_tac.end());
}

std::list<TACPtr>::iterator TACGen::create_instr(const TACPtr &instr)
{
    _current_subroutine_tac.push_back(instr);
    return std::prev(_current_subroutine_tac.end());
}

void TACGen::print()
{
    std::cout << "@floor_max = " << get_max_floor() << std::endl;

    print_subroutine(semanalyzer::GLOBAL_SCOPE_ID, _subroutine_tacs[semanalyzer::GLOBAL_SCOPE_ID]);

    for (auto &[name, tacs] : _subroutine_tacs) {
        if (name != semanalyzer::GLOBAL_SCOPE_ID) {
            print_subroutine(name, tacs);
        }
    }
}

void TACGen::print_subroutine(const std::string &name, std::list<TACPtr> &tacs)
{
    std::cout << __tc.C_DARK_PINK << "def " << name << ":" << __tc.C_RESET << std::endl;

    for (std::list<TACPtr>::iterator it = tacs.begin(); it != tacs.end(); ++it) {
        auto lbl_it = _labels.right.find(it);
        if (lbl_it != _labels.right.end()) {
            std::cout << __tc.C_DARK_BLUE << lbl_it->second << ":" << __tc.C_RESET << std::endl;
        }
        std::cout << "    " << (*it)->to_string() << std::endl;
    }

    std::cout << std::endl;
}

int TACGen::get_max_floor()
{
    return _root->get_floor_max().value_or(DEFAULT_FLOOR_MAX);
}

CLOSE_IRGEN_NAMESPACE
// end
