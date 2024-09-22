#include <cassert>
#include <iostream>
#include <iterator>
#include <list>
#include <string>

#include "Symbol.h"
#include "TACGen.h"
#include "TerminalColor.h"
#include "ThreeAddressCode.h"
#include "hrl_global.h"
#include "irgen_global.h"
#include "semanalyzer_global.h"

OPEN_IRGEN_NAMESPACE

#define BEGIN_VISIT() \
    enter_node(node); \
    int rc = 0;       \
    UNUSED(rc);

#define END_VISIT() \
    leave_node();   \
    return rc;

#define RETURN_IF_FAIL_IN_VISIT(rc) \
    if (rc != 0) {                  \
        leave_node();               \
        return rc;                  \
    }

int TACGen::run()
{
    int rc = visit(_root);
    return rc;
}

int TACGen::visit(const parser::IntegerASTNodePtr &node)
{
    BEGIN_VISIT();

    Operand imm(node->get_value(), true);
    Operand result(take_var_id_numbering());
    create_instr(ThreeAddressCode::createLoadImmediate(result, imm));
    _node_var_id_result[node] = result;
    END_VISIT();
}

int TACGen::visit(const parser::BooleanASTNodePtr &node)
{
    BEGIN_VISIT();

    Operand imm(node->get_value() ? 1 : 0, true);
    Operand result(take_var_id_numbering());
    create_instr(ThreeAddressCode::createLoadImmediate(result, imm));
    _node_var_id_result[node] = result;
    END_VISIT();
}

int TACGen::visit(const parser::VariableDeclarationASTNodePtr &node)
{
    BEGIN_VISIT();

    auto symbol = semanalyzer::Symbol::get_from(node);

    auto asgn = node->get_assignment();
    if (asgn) {
        rc = traverse(asgn);
        RETURN_IF_FAIL_IN_VISIT(rc);

        auto result = _node_var_id_result[asgn];
        assert(result);

        _symbol_to_var_map[symbol] = result;
        _node_var_id_result[node] = result;
    } else {
        // it's a decl only
        // loadi v0, 0

        Operand var(take_var_id_numbering());
        create_instr(ThreeAddressCode::createLoadImmediate(var, 0));

        _symbol_to_var_map[symbol] = var;
        _node_var_id_result[node] = var;
    }

    END_VISIT();
}

int TACGen::visit(const parser::VariableAssignmentASTNodePtr &node)
{
    BEGIN_VISIT();

    auto expr = node->get_value();
    assert(expr);

    rc = traverse(expr);
    RETURN_IF_FAIL_IN_VISIT(rc);

    _node_var_id_result[node] = _node_var_id_result[expr];
    END_VISIT();
}

int TACGen::visit(const parser::VariableAccessASTNodePtr &node)
{
    BEGIN_VISIT();

    auto symbol = semanalyzer::Symbol::get_from(node);
    auto result = _symbol_to_var_map[symbol];
    assert(result);

    _node_var_id_result[node] = result;
    END_VISIT();
}

int TACGen::visit(const parser::FloorBoxInitStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    auto asgn = node->get_assignment();
    assert(asgn);
    rc = traverse(asgn);
    RETURN_IF_FAIL_IN_VISIT(rc);
    // setting no result
    // _node_var_id_result[node] = _node_var_id_result[asgn];

    END_VISIT();
}

int TACGen::visit(const parser::FloorAssignmentASTNodePtr &node)
{
    BEGIN_VISIT();

    // floor assignment always has two parts around `=`
    auto value_node = node->get_value();
    assert(value_node);
    rc = traverse(value_node);
    RETURN_IF_FAIL_IN_VISIT(rc);
    auto value = _node_var_id_result[value_node];
    assert(value);

    auto index_node = node->get_floor_number();
    assert(index_node);
    rc = traverse(index_node);
    RETURN_IF_FAIL_IN_VISIT(rc);
    auto index = _node_var_id_result[index_node];
    assert(index);

    Operand var(take_var_id_numbering());
    create_instr(ThreeAddressCode::createDataMovement(HighLevelIROps::LOAD, var, index));
    _node_var_id_result[node] = var;

    END_VISIT();
}

int TACGen::visit(const parser::FloorAccessASTNodePtr &node)
{
    BEGIN_VISIT();

    auto index_node = node->get_index_expr();
    assert(index_node);
    rc = traverse(index_node);
    RETURN_IF_FAIL_IN_VISIT(rc);
    auto index = _node_var_id_result[index_node];
    assert(index);

    Operand var(take_var_id_numbering());
    create_instr(ThreeAddressCode::createDataMovement(HighLevelIROps::LOAD, var, index));
    _node_var_id_result[node] = var;

    END_VISIT();
}

int TACGen::visit(const parser::NegativeExpressionASTNodePtr &node)
{
    BEGIN_VISIT();

    rc = traverse(node->get_operand());
    RETURN_IF_FAIL_IN_VISIT(rc);
    auto op = _node_var_id_result[node->get_operand()];
    assert(op);

    Operand var(take_var_id_numbering());
    create_instr(ThreeAddressCode::createArithmetic(HighLevelIROps::NEG, var, op));
    _node_var_id_result[node] = var;

    END_VISIT();
}

int TACGen::visit(const parser::NotExpressionASTNodePtr &node)
{
    BEGIN_VISIT();

    rc = traverse(node->get_operand());
    RETURN_IF_FAIL_IN_VISIT(rc);
    auto op = _node_var_id_result[node->get_operand()];
    assert(op);

    Operand var(take_var_id_numbering());
    create_instr(ThreeAddressCode::createLogical(HighLevelIROps::NOT, var, op));
    _node_var_id_result[node] = var;

    END_VISIT();
}

int TACGen::visit(const parser::IncrementExpressionASTNodePtr &node)
{
    BEGIN_VISIT();

    auto symbol = semanalyzer::Symbol::get_from(node);
    assert(symbol);
    auto original = _symbol_to_var_map[symbol];
    assert(original);

    Operand var(take_var_id_numbering());
    create_instr(ThreeAddressCode::createArithmetic(HighLevelIROps::ADD, var, original, Operand(1, true)));
    _node_var_id_result[node] = var;
    _symbol_to_var_map[symbol] = var;

    END_VISIT();
}

int TACGen::visit(const parser::DecrementExpressionASTNodePtr &node)
{
    BEGIN_VISIT();

    auto symbol = semanalyzer::Symbol::get_from(node);
    assert(symbol);
    auto original = _symbol_to_var_map[symbol];
    assert(original);

    Operand var(take_var_id_numbering());
    create_instr(ThreeAddressCode::createArithmetic(HighLevelIROps::SUB, var, original, Operand(1, true)));
    _node_var_id_result[node] = var;
    _symbol_to_var_map[symbol] = var;

    END_VISIT();
}

template <HighLevelIROps op>
int TACGen::visit_binary_expression(const parser::AbstractBinaryExpressionASTNodePtr &node)
{
    BEGIN_VISIT();

    auto left_node = node->get_left();
    auto right_node = node->get_right();
    assert(left_node);
    assert(right_node);

    rc = traverse_multiple(left_node, right_node);
    RETURN_IF_FAIL_IN_VISIT(rc);

    auto left = _node_var_id_result[left_node];
    auto right = _node_var_id_result[right_node];
    assert(left);
    assert(right);

    Operand tgt(take_var_id_numbering());
    create_binary_instr<op>(tgt, left, right);
    _node_var_id_result[node] = tgt;

    END_VISIT();
}

int TACGen::visit(const parser::AddExpressionASTNodePtr &node)
{
    return visit_binary_expression<HighLevelIROps::ADD>(node);
}

int TACGen::visit(const parser::SubExpressionASTNodePtr &node)
{
    return visit_binary_expression<HighLevelIROps::SUB>(node);
}

int TACGen::visit(const parser::MulExpressionASTNodePtr &node)
{
    return visit_binary_expression<HighLevelIROps::MUL>(node);
}

int TACGen::visit(const parser::DivExpressionASTNodePtr &node)
{
    return visit_binary_expression<HighLevelIROps::DIV>(node);
}

int TACGen::visit(const parser::ModExpressionASTNodePtr &node)
{
    return visit_binary_expression<HighLevelIROps::MOD>(node);
}

int TACGen::visit(const parser::EqualExpressionASTNodePtr &node)
{
    return visit_binary_expression<HighLevelIROps::EQ>(node);
}

int TACGen::visit(const parser::NotEqualExpressionASTNodePtr &node)
{
    return visit_binary_expression<HighLevelIROps::NE>(node);
}

int TACGen::visit(const parser::GreaterThanExpressionASTNodePtr &node)
{
    return visit_binary_expression<HighLevelIROps::GT>(node);
}

int TACGen::visit(const parser::GreaterEqualExpressionASTNodePtr &node)
{
    return visit_binary_expression<HighLevelIROps::GE>(node);
}

int TACGen::visit(const parser::LessThanExpressionASTNodePtr &node)
{
    return visit_binary_expression<HighLevelIROps::LT>(node);
}

int TACGen::visit(const parser::LessEqualExpressionASTNodePtr &node)
{
    return visit_binary_expression<HighLevelIROps::LE>(node);
}

int TACGen::visit(const parser::AndExpressionASTNodePtr &node)
{
    return visit_binary_expression<HighLevelIROps::AND>(node);
}

int TACGen::visit(const parser::OrExpressionASTNodePtr &node)
{
    return visit_binary_expression<HighLevelIROps::OR>(node);
}

int TACGen::visit(const parser::InvocationExpressionASTNodePtr &node)
{
    BEGIN_VISIT();

    Operand result(take_var_id_numbering());
    auto param_node = node->get_argument();
    if (param_node) {
        rc = traverse(param_node);
        RETURN_IF_FAIL_IN_VISIT(rc);
        Operand param = _node_var_id_result[param_node];
        ThreeAddressCode::createCall(*node->get_func_name(), param, result);
    } else {
        ThreeAddressCode::createCall(*node->get_func_name(), result);
    }
    _node_var_id_result[node] = result;

    END_VISIT();
}

int TACGen::visit(const parser::EmptyStatementASTNodePtr &node)
{
    BEGIN_VISIT();
    END_VISIT();
}

int TACGen::visit(const parser::IfStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    std::string
        if_block_id(take_block_label("if")),
        then_block_id(take_block_label("then")),
        else_block_id(take_block_label("else")),
        end_if_id(take_block_label("fi"));

    // condition
    auto if_block_start(create_noop());
    auto cond_node = node->get_condition();
    assert(cond_node);
    rc = traverse(cond_node);
    RETURN_IF_FAIL_IN_VISIT(rc);
    Operand cond = _node_var_id_result[cond_node];
    assert(cond);
    // if cond true (1), proceed to then block
    // if cond false (0), proceed to else block
    create_jz(cond, else_block_id);

    // then
    auto then_block_start(create_noop());
    auto then_node = node->get_then_branch();
    if (then_node) {
        rc = traverse(then_node);
        RETURN_IF_FAIL_IN_VISIT(rc);
        create_jmp(end_if_id);
    }

    // else
    auto else_block_start(create_noop());
    auto else_node = node->get_else_branch();
    if (else_node) {
        rc = traverse(else_node);
        RETURN_IF_FAIL_IN_VISIT(rc);
    }

    auto end_if_start(create_noop());

    _labels.insert({ if_block_id, if_block_start });
    _labels.insert({ then_block_id, then_block_start });
    _labels.insert({ else_block_id, else_block_start });
    _labels.insert({ end_if_id, end_if_start });

    END_VISIT();
}

int TACGen::visit(const parser::WhileStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    std::string
        while_block_id(take_block_label("while")),
        loop_block_id(take_block_label("loop")),
        end_while_block_id(take_block_label("elihw"));

    _loop_break_dest.push(end_while_block_id);
    _loop_continue_dest.push(while_block_id);

    // condition
    auto while_block_start(create_noop());
    auto cond_node = node->get_condition();
    assert(cond_node);
    rc = traverse(cond_node);
    RETURN_IF_FAIL_IN_VISIT(rc);
    Operand cond = _node_var_id_result[cond_node];
    assert(cond);
    // true: proceed
    // false (0): jump to end while
    create_jz(cond, end_while_block_id);

    // loop
    auto loop_block_start(create_noop());

    rc = traverse(node->get_body());
    RETURN_IF_FAIL_IN_VISIT(rc);
    create_jmp(while_block_id);

    // end loop
    auto end_while_block_start(create_noop());
    _labels.insert({ while_block_id, while_block_start });
    _labels.insert({ loop_block_id, loop_block_start });
    _labels.insert({ end_while_block_id, end_while_block_start });

    _loop_break_dest.pop();
    _loop_continue_dest.pop();

    END_VISIT();
}

int TACGen::visit(const parser::ForStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    std::string
        for_block_id(take_block_label("for")),
        cond_block_id(take_block_label("cond")),
        loop_block_id(take_block_label("loop")),
        update_block_id(take_block_label("update")),
        end_for_block_id(take_block_label("rof"));

    _loop_break_dest.push(end_for_block_id);
    _loop_continue_dest.push(update_block_id);

    // init
    auto for_block_start(create_noop());
    rc = traverse(node->get_init());
    RETURN_IF_FAIL_IN_VISIT(rc);

    // cond
    auto cond_block_start(create_noop());
    auto cond_node = node->get_condition();
    assert(cond_node);
    rc = traverse(cond_node);
    RETURN_IF_FAIL_IN_VISIT(rc);
    Operand cond = _node_var_id_result[cond_node];
    assert(cond);
    // true: proceed
    // false (0): jump to end for
    create_jz(cond, end_for_block_id);

    // loop
    auto loop_block_start(create_noop());
    rc = traverse(node->get_body());
    RETURN_IF_FAIL_IN_VISIT(rc);

    // update
    auto update_block_start(create_noop());
    rc = traverse(node->get_update());
    RETURN_IF_FAIL_IN_VISIT(rc);
    create_jmp(for_block_id);

    // end loop
    auto end_for_block_start(create_noop());
    _labels.insert({ for_block_id, for_block_start });
    _labels.insert({ cond_block_id, cond_block_start });
    _labels.insert({ loop_block_id, loop_block_start });
    _labels.insert({ update_block_id, update_block_start });
    _labels.insert({ end_for_block_id, end_for_block_start });

    _loop_break_dest.pop();
    _loop_continue_dest.pop();

    END_VISIT();
}

int TACGen::visit(const parser::ReturnStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    auto expr_node = node->get_expression();
    if (expr_node) {
        rc = traverse(expr_node);
        RETURN_IF_FAIL_IN_VISIT(rc);
        auto expr = _node_var_id_result[expr_node];
        create_instr(ThreeAddressCode::createReturn(expr));
    } else {
        create_instr(ThreeAddressCode::createReturn());
    }

    END_VISIT();
}

int TACGen::visit(const parser::BreakStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    assert(!_loop_break_dest.empty());
    create_jmp(_loop_break_dest.top());

    END_VISIT();
}

int TACGen::visit(const parser::ContinueStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    assert(!_loop_continue_dest.empty());
    create_jmp(_loop_continue_dest.top());

    END_VISIT();
}

int TACGen::visit(const parser::StatementBlockASTNodePtr &node)
{
    BEGIN_VISIT();

    rc = traverse(node->get_statements());
    RETURN_IF_FAIL_IN_VISIT(rc);

    END_VISIT();
}

int TACGen::visit(const parser::SubprocDefinitionASTNodePtr &node)
{
    return visit_subroutine(node);
}

int TACGen::visit(const parser::FunctionDefinitionASTNodePtr &node)
{
    return visit_subroutine(node);
}

int TACGen::visit_subroutine(const parser::AbstractSubroutineASTNodePtr &node)
{
    BEGIN_VISIT();

    _current_subroutine_var_id = 0;
    _current_block_label_id = 0;
    _current_subroutine_name = *node->get_name();

    auto param_node = node->get_parameter();
    Operand param(take_var_id_numbering());
    auto func_start = create_instr(ThreeAddressCode::createEnter(param));
    if (param_node) {
        auto symbol = semanalyzer::Symbol::get_from(param_node);
        assert(symbol);
        _symbol_to_var_map[symbol] = param;
    }

    rc = traverse(node->get_body());
    RETURN_IF_FAIL_IN_VISIT(rc);

    // it's ensured that all path returned
    auto func_end = create_instr(ThreeAddressCode::createSpecial(HighLevelIROps::HALT));

    _labels.insert({ *node->get_name(), func_start });
    _labels.insert({ *node->get_name() + ".end_no_return", func_end });

    auto &list = _subroutine_tacs[*node->get_name()];
    list.swap(_current_subroutine_tac);
    _current_subroutine_tac.clear();

    END_VISIT();
}

int TACGen::visit(const parser::CompilationUnitASTNodePtr &node)
{
    BEGIN_VISIT();

    _current_subroutine_var_id = 0;
    _current_block_label_id = 0;
    _current_subroutine_name = semanalyzer::GLOBAL_SCOPE_ID;

    rc = traverse_multiple(node->get_floor_inits(), node->get_var_decls());
    RETURN_IF_FAIL_IN_VISIT(rc);

    create_jmp("start");

    auto &list = _subroutine_tacs[semanalyzer::GLOBAL_SCOPE_ID];
    list.swap(_current_subroutine_tac);
    _current_subroutine_tac.clear();

    // subroutine visits will clear var id and block label numbering

    rc = traverse(node->get_subroutines());
    RETURN_IF_FAIL_IN_VISIT(rc);

    END_VISIT();
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

std::list<TACPtr>::iterator TACGen::create_noop()
{
    _current_subroutine_tac.emplace_back(ThreeAddressCode::createSpecial(HighLevelIROps::NOP));
    return std::prev(_current_subroutine_tac.end());
}

std::list<TACPtr>::iterator TACGen::create_jmp(const std::string &label)
{
    _current_subroutine_tac.emplace_back(ThreeAddressCode::createBranching(Operand(label)));
    return std::prev(_current_subroutine_tac.end());
}

std::list<TACPtr>::iterator TACGen::create_jnz(const Operand &operand, const std::string &label)
{
    _current_subroutine_tac.emplace_back(ThreeAddressCode::createBranching(HighLevelIROps::JNZ, Operand(label), operand));
    return std::prev(_current_subroutine_tac.end());
}

std::list<TACPtr>::iterator TACGen::create_jz(const Operand &operand, const std::string &label)
{
    _current_subroutine_tac.emplace_back(ThreeAddressCode::createBranching(HighLevelIROps::JZ, Operand(label), operand));
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
    std::cout << __tc.C_HIGHLIGHT << "def " << name << ":" << __tc.C_RESET << std::endl;

    for (std::list<TACPtr>::iterator it = tacs.begin(); it != tacs.end(); ++it) {
        auto lbl_it = _labels.right.find(it);
        if (lbl_it != _labels.right.end()) {
            std::cout << __tc.C_DARK_BLUE << lbl_it->second << ":" << __tc.C_RESET << std::endl;
        }
        std::cout << "    " << (*it)->to_string() << std::endl;
    }

    std::cout << std::endl
              << std::endl;
}

int TACGen::get_max_floor()
{
    return _root->get_floor_max().value_or(DEFAULT_FLOOR_MAX);
}

CLOSE_IRGEN_NAMESPACE
// end
