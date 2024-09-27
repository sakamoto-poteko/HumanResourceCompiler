#include <iterator>
#include <list>
#include <memory>
#include <string>

#include <boost/range.hpp>

#include "IROps.h"
#include "TACGen.h"
#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

int TACGen::run()
{
    int rc = visit(_root);
    if (rc != 0) {
        return rc;
    }
    rc = build_ir_program();
    if (rc != 0) {
        return rc;
    }
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
    _current_subroutine_tac.emplace_back(ThreeAddressCode::create_special(IROperation::NOP, node));
    return std::prev(_current_subroutine_tac.end());
}

std::list<TACPtr>::iterator TACGen::create_jmp(const std::string &label, const parser::ASTNodePtr &node)
{
    _current_subroutine_tac.emplace_back(ThreeAddressCode::create_branching(Operand(label), node));
    return std::prev(_current_subroutine_tac.end());
}

std::list<TACPtr>::iterator TACGen::create_jnz(const Operand &operand, const std::string &label, const parser::ASTNodePtr &node)
{
    _current_subroutine_tac.emplace_back(ThreeAddressCode::create_branching(IROperation::JNZ, Operand(label), operand, node));
    return std::prev(_current_subroutine_tac.end());
}

std::list<TACPtr>::iterator TACGen::create_jz(const Operand &operand, const std::string &label, const parser::ASTNodePtr &node)
{
    _current_subroutine_tac.emplace_back(ThreeAddressCode::create_branching(IROperation::JZ, Operand(label), operand, node));
    return std::prev(_current_subroutine_tac.end());
}

std::list<TACPtr>::iterator TACGen::create_instr(const TACPtr &instr)
{
    _current_subroutine_tac.push_back(instr);
    return std::prev(_current_subroutine_tac.end());
}

CLOSE_IRGEN_NAMESPACE
// end
