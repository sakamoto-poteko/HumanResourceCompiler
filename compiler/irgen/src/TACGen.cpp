#include "TACGen.h"
#include "ParseTreeNode.h"
#include "Symbol.h"
#include "ThreeAddressCode.h"
#include "irgen_global.h"
#include <cassert>

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
}

int TACGen::visit(const parser::IntegerASTNodePtr &node)
{
    BEGIN_VISIT();

    Operand result(node->get_value(), true);

    _node_var_id_result[node] = result;
    END_VISIT();
}

int TACGen::visit(const parser::BooleanASTNodePtr &node)
{
    BEGIN_VISIT();

    Operand result(node->get_value() ? 1 : 0, true);

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
        _current_subroutine_tac.emplace_back(ThreeAddressCode::createLoadImmediate(var, 0));

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
    _current_subroutine_tac.emplace_back(ThreeAddressCode::createDataMovement(HighLevelIROps::LOAD, var, index));
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
    _current_subroutine_tac.emplace_back(ThreeAddressCode::createDataMovement(HighLevelIROps::LOAD, var, index));
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
    _current_subroutine_tac.emplace_back(ThreeAddressCode::createArithmetic(HighLevelIROps::NEG, var, op));
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
    _current_subroutine_tac.emplace_back(ThreeAddressCode::createLogical(HighLevelIROps::NOT, var, op));
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
    _current_subroutine_tac.emplace_back(ThreeAddressCode::createArithmetic(HighLevelIROps::ADD, var, original, Operand(1, true)));
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
    _current_subroutine_tac.emplace_back(ThreeAddressCode::createArithmetic(HighLevelIROps::SUB, var, original, Operand(1, true)));
    _node_var_id_result[node] = var;
    _symbol_to_var_map[symbol] = var;

    END_VISIT();
}

template <HighLevelIROps op>
int TACGen::visit_binary_expression(const parser::AbstractBinaryExpressionASTNodePtr &node)
{
    BEGIN_VISIT();

    auto l = node->get_left();
    auto r = node->get_right();
    assert(l);
    assert(r);

    rc = traverse_multiple(l, r);
    RETURN_IF_FAIL_IN_VISIT(rc);

    auto lresult = _node_var_id_result[l];
    auto rresult = _node_var_id_result[r];
    assert(lresult);
    assert(rresult);

    Operand var(take_var_id_numbering());

    if constexpr (op >= HighLevelIROps::ADD && op <= HighLevelIROps::MOD) {
        _current_subroutine_tac.emplace_back(ThreeAddressCode::createArithmetic(op, var, lresult, rresult));
    } else if constexpr (op >= HighLevelIROps::EQ && op <= HighLevelIROps::GE) {
        _current_subroutine_tac.emplace_back(ThreeAddressCode::createComparison(op, var, lresult, rresult));
    } else if constexpr (op == HighLevelIROps::AND || op == HighLevelIROps::OR) {
        _current_subroutine_tac.emplace_back(ThreeAddressCode::createLogical(op, var, lresult, rresult));
    } else {
        static_assert(false, "invalid op passed in");
    }

    _node_var_id_result[node] = var;

    END_VISIT();
}

int TACGen::visit(const parser::AddExpressionASTNodePtr &node)
{
    return visit_binary_expression<HighLevelIROps::ADD>(node);
}

int TACGen::visit(const parser::SubExpressionASTNodePtr &node)
{
    return visit_binary_expression<HighLevelIROps::ADD>(node);
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
}

int TACGen::visit(const parser::WhileStatementASTNodePtr &node)
{
}

int TACGen::visit(const parser::ForStatementASTNodePtr &node)
{
}

int TACGen::visit(const parser::ReturnStatementASTNodePtr &node)
{
}

int TACGen::visit(const parser::BreakStatementASTNodePtr &node)
{
}

int TACGen::visit(const parser::ContinueStatementASTNodePtr &node)
{
}

int TACGen::visit(const parser::StatementBlockASTNodePtr &node)
{
}

int TACGen::visit(const parser::SubprocDefinitionASTNodePtr &node)
{
}

int TACGen::visit(const parser::FunctionDefinitionASTNodePtr &node)
{
}

int TACGen::visit(const parser::CompilationUnitASTNodePtr &node)
{
    BEGIN_VISIT();

    rc = traverse_multiple(node->get_floor_inits(), node->get_var_decls());
    RETURN_IF_FAIL_IN_VISIT(rc);

    rc = traverse(node->get_subroutines());
    RETURN_IF_FAIL_IN_VISIT(rc);

    END_VISIT();
}

CLOSE_IRGEN_NAMESPACE
// end
