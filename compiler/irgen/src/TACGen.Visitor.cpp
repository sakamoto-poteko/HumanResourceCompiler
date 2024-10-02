#include <cassert>
#include <list>
#include <memory>
#include <string>

#include <boost/range.hpp>

#include "ASTNode.h"
#include "HRBox.h"
#include "IROps.h"
#include "IRProgramStructure.h"
#include "Operand.h"
#include "Symbol.h"
#include "TACGen.h"
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

int TACGen::visit(const parser::IntegerASTNodePtr &node)
{
    BEGIN_VISIT();

    Operand result(take_var_id_numbering());
    create_instr(ThreeAddressCode::create_load_immediate(result, node->get_hrbox(), node));
    _node_var_id_result[node] = result;
    END_VISIT();
}

int TACGen::visit(const parser::BooleanASTNodePtr &node)
{
    BEGIN_VISIT();

    Operand result(take_var_id_numbering());
    create_instr(ThreeAddressCode::create_load_immediate(result, HRBox(node->get_value() ? 1 : 0), node));
    _node_var_id_result[node] = result;
    END_VISIT();
}

int TACGen::visit(const parser::VariableDeclarationASTNodePtr &node)
{
    BEGIN_VISIT();

    auto symbol = semanalyzer::Symbol::get_from(node);

    int var_id = take_var_id_numbering();
    if (_in_global_var_decl) {
        // it could be 0. -0 doesn't help distinguish between local and global
        // so we -1
        var_id = -var_id - 1;
    }
    Operand var(var_id);
    // add to the symbol map before traversing assignment
    _symbol_to_var_map[symbol] = var;

    auto asgn = node->get_assignment();
    if (asgn) {
        rc = traverse(asgn);
        RETURN_IF_FAIL_IN_VISIT(rc);

        // auto result = _node_var_id_result[asgn];
        // assert(result);

        // create_instr(ThreeAddressCode::create_data_movement(HighLevelIROps::MOV, var, result));
    } else {
        // it's a decl only
        // loadi v0, 0
        create_instr(ThreeAddressCode::create_load_immediate(var, HRBox(0), node));
    }

    _node_var_id_result[node] = var;
    END_VISIT();
}

int TACGen::visit(const parser::VariableAssignmentASTNodePtr &node)
{
    BEGIN_VISIT();

    auto expr = node->get_value();
    assert(expr);

    rc = traverse(expr);
    RETURN_IF_FAIL_IN_VISIT(rc);

    auto symbol = semanalyzer::Symbol::get_from(node);
    auto decl_operand = _symbol_to_var_map[symbol]; // this is created in visiting declaration node
    assert(decl_operand);

    auto expr_result = _node_var_id_result[expr];
    assert(expr_result);

    if (decl_operand.get_register_id() < 0) {
        // it's a global. for global, we store instead of mov
        create_instr(ThreeAddressCode::create_data_movement(IROperation::STORE, Operand(), decl_operand, expr_result, node));
    } else {
        create_instr(ThreeAddressCode::create_data_movement(IROperation::MOV, decl_operand, expr_result, Operand(), node));
    }

    _node_var_id_result[node] = _node_var_id_result[expr];
    END_VISIT();
}

int TACGen::visit(const parser::VariableAccessASTNodePtr &node)
{
    BEGIN_VISIT();

    auto symbol = semanalyzer::Symbol::get_from(node);
    auto result = _symbol_to_var_map[symbol];
    assert(result);

    // it's a global. we need to load first
    if (result.get_register_id() < 0) {
        Operand src(result);
        result = Operand(take_var_id_numbering());
        create_instr(ThreeAddressCode::create_data_movement(IROperation::LOAD, result, src, Operand(), node));
    }

    _node_var_id_result[node] = result;
    END_VISIT();
}

int TACGen::visit(const parser::FloorBoxInitStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    // floor_box_init_statement = INIT, FLOOR, OPEN_BRACKET, INTEGER, CLOSE_BRACKET, EQ, INTEGER, T;
    // floor init statement always has a non-null assignment part, and the assignment is always an integer
    auto asgn = node->get_assignment();
    assert(asgn);
    auto flr_id_node = asgn->get_floor_number();
    assert(flr_id_node);
    auto flr_value_node = asgn->get_value();
    assert(flr_value_node);

    assert(flr_id_node->get_node_type() == parser::ASTNodeType::Integer);
    assert(flr_value_node->get_node_type() == parser::ASTNodeType::Integer);

    auto flr_id = std::static_pointer_cast<parser::IntegerASTNode>(flr_id_node);
    auto flr_value = std::static_pointer_cast<parser::IntegerASTNode>(flr_value_node);

    if (flr_value->get_is_char()) {
        _floor_inits.insert_or_assign(flr_id->get_value(), std::move(HRBox(static_cast<int>(flr_value->get_value()))));
    } else {
        _floor_inits.insert_or_assign(flr_id->get_value(), std::move(HRBox(static_cast<char>(flr_value->get_value()))));
    }

    // setting no result
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
    create_instr(ThreeAddressCode::create_data_movement(IROperation::STORE, Operand(), index, value, node));
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
    create_instr(ThreeAddressCode::create_data_movement(IROperation::LOAD, var, index, Operand(), node));
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
    create_instr(ThreeAddressCode::create_arithmetic(IROperation::NEG, var, op, node));
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
    create_instr(ThreeAddressCode::create_logical(IROperation::NOT, var, op, node));
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

    Operand one_var(take_var_id_numbering());
    create_instr(ThreeAddressCode::create_load_immediate(one_var, HRBox(1)));

    Operand var(take_var_id_numbering());
    create_instr(ThreeAddressCode::create_arithmetic(IROperation::ADD, var, original, one_var, node));

    if (original.get_register_id() < 0) {
        // it's a global. for global, we store instead of mov
        create_instr(ThreeAddressCode::create_data_movement(IROperation::STORE, Operand(), original, var, node));
    } else {
        create_instr(ThreeAddressCode::create_data_movement(IROperation::MOV, original, var, Operand(), node));
    }

    _node_var_id_result[node] = original;

    END_VISIT();
}

int TACGen::visit(const parser::DecrementExpressionASTNodePtr &node)
{
    BEGIN_VISIT();

    auto symbol = semanalyzer::Symbol::get_from(node);
    assert(symbol);
    Operand original = _symbol_to_var_map[symbol];
    assert(original);

    Operand one_var(take_var_id_numbering());
    create_instr(ThreeAddressCode::create_load_immediate(one_var, HRBox(1)));

    Operand var(take_var_id_numbering());
    create_instr(ThreeAddressCode::create_arithmetic(IROperation::SUB, var, original, one_var, node));

    if (original.get_register_id() < 0) {
        // it's a global. for global, we store instead of mov
        create_instr(ThreeAddressCode::create_data_movement(IROperation::STORE, Operand(), original, var, node));
    } else {
        create_instr(ThreeAddressCode::create_data_movement(IROperation::MOV, original, var, Operand(), node));
    }

    _node_var_id_result[node] = original;

    END_VISIT();
}

template <IROperation op>
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
    create_binary_instr<op>(tgt, left, right, node);
    _node_var_id_result[node] = tgt;

    END_VISIT();
}

int TACGen::visit(const parser::AddExpressionASTNodePtr &node)
{
    return visit_binary_expression<IROperation::ADD>(node);
}

int TACGen::visit(const parser::SubExpressionASTNodePtr &node)
{
    return visit_binary_expression<IROperation::SUB>(node);
}

int TACGen::visit(const parser::MulExpressionASTNodePtr &node)
{
    return visit_binary_expression<IROperation::MUL>(node);
}

int TACGen::visit(const parser::DivExpressionASTNodePtr &node)
{
    return visit_binary_expression<IROperation::DIV>(node);
}

int TACGen::visit(const parser::ModExpressionASTNodePtr &node)
{
    return visit_binary_expression<IROperation::MOD>(node);
}

int TACGen::visit(const parser::EqualExpressionASTNodePtr &node)
{
    return visit_binary_expression<IROperation::EQ>(node);
}

int TACGen::visit(const parser::NotEqualExpressionASTNodePtr &node)
{
    return visit_binary_expression<IROperation::NE>(node);
}

int TACGen::visit(const parser::GreaterThanExpressionASTNodePtr &node)
{
    return visit_binary_expression<IROperation::GT>(node);
}

int TACGen::visit(const parser::GreaterEqualExpressionASTNodePtr &node)
{
    return visit_binary_expression<IROperation::GE>(node);
}

int TACGen::visit(const parser::LessThanExpressionASTNodePtr &node)
{
    return visit_binary_expression<IROperation::LT>(node);
}

int TACGen::visit(const parser::LessEqualExpressionASTNodePtr &node)
{
    return visit_binary_expression<IROperation::LE>(node);
}

int TACGen::visit(const parser::AndExpressionASTNodePtr &node)
{
    return visit_binary_expression<IROperation::AND>(node);
}

int TACGen::visit(const parser::OrExpressionASTNodePtr &node)
{
    return visit_binary_expression<IROperation::OR>(node);
}

int TACGen::visit(const parser::InvocationExpressionASTNodePtr &node)
{
    BEGIN_VISIT();

    const std::string &func_name = *node->get_func_name();

    Operand result(take_var_id_numbering());
    auto param_node = node->get_argument();
    if (param_node) {
        rc = traverse(param_node);
        RETURN_IF_FAIL_IN_VISIT(rc);
        Operand param = _node_var_id_result[param_node];

        if (func_name == "outbox") {
            create_instr(ThreeAddressCode::create_io(IROperation::OUTPUT, param, node));
            _node_var_id_result[node] = Operand(HRBox(0));
        } else {
            create_instr(ThreeAddressCode::create_call(Operand(func_name), param, result, node));
            _node_var_id_result[node] = result;
        }
    } else {
        if (func_name == "inbox") {
            create_instr(ThreeAddressCode::create_io(IROperation::INPUT, result, node));
            _node_var_id_result[node] = result;
        } else {
            create_instr(ThreeAddressCode::create_call(Operand(func_name), result, node));
            _node_var_id_result[node] = result;
        }
    }

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
    auto if_block_start(create_noop(node));
    auto cond_node = node->get_condition();
    assert(cond_node);
    rc = traverse(cond_node);
    RETURN_IF_FAIL_IN_VISIT(rc);
    Operand cond = _node_var_id_result[cond_node];
    assert(cond);
    // if cond true (1), proceed to then block
    // if cond false (0), proceed to else block
    create_jz(cond, else_block_id, node);

    // then
    auto then_block_start(create_noop(node));
    auto then_node = node->get_then_branch();
    if (then_node) {
        rc = traverse(then_node);
        RETURN_IF_FAIL_IN_VISIT(rc);
        create_jmp(end_if_id, node);
    }

    // else
    auto else_block_start(create_noop(node));
    auto else_node = node->get_else_branch();
    if (else_node) {
        rc = traverse(else_node);
        RETURN_IF_FAIL_IN_VISIT(rc);
    }

    auto end_if_start(create_noop(node));

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
    auto while_block_start(create_noop(node));
    auto cond_node = node->get_condition();
    assert(cond_node);
    rc = traverse(cond_node);
    RETURN_IF_FAIL_IN_VISIT(rc);
    Operand cond = _node_var_id_result[cond_node];
    assert(cond);
    // true: proceed
    // false (0): jump to end while
    create_jz(cond, end_while_block_id, node);

    // loop
    auto loop_block_start(create_noop(node));

    rc = traverse(node->get_body());
    RETURN_IF_FAIL_IN_VISIT(rc);
    create_jmp(while_block_id, node);

    // end loop
    auto end_while_block_start(create_noop(node));
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
        init_block_id(take_block_label("for")),
        cond_block_id(take_block_label("cond")),
        loop_block_id(take_block_label("loop")),
        update_block_id(take_block_label("update")),
        end_for_block_id(take_block_label("rof"));

    _loop_break_dest.push(end_for_block_id);
    _loop_continue_dest.push(update_block_id);

    // init
    auto for_block_start(create_noop(node));
    rc = traverse(node->get_init());
    RETURN_IF_FAIL_IN_VISIT(rc);

    // cond
    auto cond_block_start(create_noop(node));
    auto cond_node = node->get_condition();
    assert(cond_node);
    rc = traverse(cond_node);
    RETURN_IF_FAIL_IN_VISIT(rc);
    Operand cond = _node_var_id_result[cond_node];
    assert(cond);
    // true: proceed
    // false (0): jump to end for
    create_jz(cond, end_for_block_id, node);

    // loop
    auto loop_block_start(create_noop(node));
    rc = traverse(node->get_body());
    RETURN_IF_FAIL_IN_VISIT(rc);

    // update
    auto update_block_start(create_noop(node));
    rc = traverse(node->get_update());
    RETURN_IF_FAIL_IN_VISIT(rc);
    create_jmp(cond_block_id, node);

    // end loop
    auto end_for_block_start(create_noop(node));
    _labels.insert({ init_block_id, for_block_start });
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
        create_instr(ThreeAddressCode::create_return(expr, node));
    } else {
        create_instr(ThreeAddressCode::create_return(node));
    }

    END_VISIT();
}

int TACGen::visit(const parser::BreakStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    assert(!_loop_break_dest.empty());
    create_jmp(_loop_break_dest.top(), node);

    END_VISIT();
}

int TACGen::visit(const parser::ContinueStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    assert(!_loop_continue_dest.empty());
    create_jmp(_loop_continue_dest.top(), node);

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
    auto func_start = create_instr(ThreeAddressCode::create_enter(param, node));
    if (param_node) {
        auto symbol = semanalyzer::Symbol::get_from(param_node);
        assert(symbol);
        _symbol_to_var_map[symbol] = param;
    }

    rc = traverse(node->get_body());
    RETURN_IF_FAIL_IN_VISIT(rc);

    // it's ensured that all path returned
    InstructionListIter func_end;

    if (node->get_node_type() == parser::ASTNodeType::FunctionDefinition) {
        Operand var(take_var_id_numbering());
        func_end = create_instr(ThreeAddressCode::create_load_immediate(var, HRBox(0), node));
        create_instr(ThreeAddressCode::create_return(var));
    } else {
        // it's a subproc
        func_end = create_instr(ThreeAddressCode::create_return(node));
    }

    _labels.insert({ *node->get_name(), func_start });
    _labels.insert({ *node->get_name() + ".end", func_end });

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

    rc = traverse(node->get_floor_inits());
    RETURN_IF_FAIL_IN_VISIT(rc);

    _in_global_var_decl = true;
    rc = traverse(node->get_var_decls());
    RETURN_IF_FAIL_IN_VISIT(rc);
    _in_global_var_decl = false;

    Operand start_result(take_var_id_numbering());
    create_instr(ThreeAddressCode::create_call(Operand("start"), start_result, node));

    auto &list = _subroutine_tacs[semanalyzer::GLOBAL_SCOPE_ID];
    list.swap(_current_subroutine_tac);
    _current_subroutine_tac.clear();

    // subroutine visits will clear var id and block label numbering

    rc = traverse(node->get_subroutines());
    RETURN_IF_FAIL_IN_VISIT(rc);

    END_VISIT();
}

CLOSE_IRGEN_NAMESPACE
// end
