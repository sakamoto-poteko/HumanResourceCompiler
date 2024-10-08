#include <cassert>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

#include "ASTBuilder.h"
#include "ASTNode.h"
#include "ASTNodeForward.h"
#include "ParseTreeNode.h"
#include "hrl_global.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

#define SET_RESULT(NodeType, ...)                                                                                                                        \
    _result_stack.push(std::static_pointer_cast<ASTNode>(std::make_shared<NodeType>(node->lineno(), node->colno(), -1, -1 __VA_OPT__(, ) __VA_ARGS__))); \
    return

ASTBuilder::ASTBuilder(CompilationUnitPTNodePtr root)
    : _root(std::move(root))
{
    // Constructor implementation (if any)
}

void ASTBuilder::visit(IdentifierPTNodePtr node)
{
    // Will be called when evaluating expression
    // In this case it's a variable access node
    // If it's not the case, the visit_and_cast will raise, since it's a bad cast.
    SET_RESULT(VariableAccessASTNode, node->get_value());
}

void ASTBuilder::visit(IntegerLiteralPTNodePtr node)
{
    SET_RESULT(IntegerASTNode, node->get_value(), node->get_is_char());
}

void ASTBuilder::visit(BooleanLiteralPTNodePtr node)
{
    SET_RESULT(BooleanASTNode, node->get_value());
}

void ASTBuilder::visit(BinaryOperatorPTNodePtr node)
{
    UNUSED(node);
    assert(false);
    // passed through. won't be called
}

void ASTBuilder::visit(VariableDeclarationPTNodePtr node)
{
    StringPtr var_name = node->get_var_name()->get_value();

    if (node->get_expr()) {
        auto value_expr = visit_and_cast<AbstractExpressionASTNodePtr>(node->get_expr());

        VariableAssignmentASTNodePtr assignment = std::make_shared<VariableAssignmentASTNode>(
            node->get_equals()->lineno(), node->get_equals()->colno(), -1, -1,
            var_name, value_expr);
        SET_RESULT(VariableDeclarationASTNode, var_name, assignment);
    } else {
        SET_RESULT(VariableDeclarationASTNode, var_name, nullptr);
    }
}

void ASTBuilder::visit(VariableAssignmentPTNodePtr node)
{
    StringPtr var_name = node->get_var_name()->get_value();
    auto value_expr = visit_and_cast<AbstractExpressionASTNodePtr>(node->get_expr());

    SET_RESULT(VariableAssignmentASTNode, var_name, value_expr);
}

void ASTBuilder::visit(FloorAssignmentPTNodePtr node)
{
    auto floor_number = visit_and_cast<AbstractExpressionASTNodePtr>(node->get_floor_access()->get_index_expr());
    auto value_expr = visit_and_cast<AbstractExpressionASTNodePtr>(node->get_expr());

    SET_RESULT(FloorAssignmentASTNode, floor_number, value_expr);
}

void ASTBuilder::visit(BinaryExpressionPTNodePtr node)
{
    auto left = visit_and_cast<AbstractExpressionASTNodePtr>(node->get_left());
    auto right = visit_and_cast<AbstractExpressionASTNodePtr>(node->get_right());

    switch (node->get_op()->get_op()) {
    case BinaryOperatorPTNode::GE:
        SET_RESULT(GreaterEqualExpressionASTNode, left, right);
    case BinaryOperatorPTNode::LE:
        SET_RESULT(LessEqualExpressionASTNode, left, right);
    case BinaryOperatorPTNode::EE:
        SET_RESULT(EqualExpressionASTNode, left, right);
    case BinaryOperatorPTNode::NE:
        SET_RESULT(NotEqualExpressionASTNode, left, right);
    case BinaryOperatorPTNode::GT:
        SET_RESULT(GreaterThanExpressionASTNode, left, right);
    case BinaryOperatorPTNode::LT:
        SET_RESULT(LessThanExpressionASTNode, left, right);
    case BinaryOperatorPTNode::AND:
        SET_RESULT(AndExpressionASTNode, left, right);
    case BinaryOperatorPTNode::OR:
        SET_RESULT(OrExpressionASTNode, left, right);
    case BinaryOperatorPTNode::ADD:
        SET_RESULT(AddExpressionASTNode, left, right);
    case BinaryOperatorPTNode::SUB:
        SET_RESULT(SubExpressionASTNode, left, right);
    case BinaryOperatorPTNode::MUL:
        SET_RESULT(MulExpressionASTNode, left, right);
    case BinaryOperatorPTNode::DIV:
        SET_RESULT(DivExpressionASTNode, left, right);
    case BinaryOperatorPTNode::MOD:
        SET_RESULT(ModExpressionASTNode, left, right);
    default:
        spdlog::critical("unrecognized binary operator {}. {}", static_cast<int>(node->get_op()->get_op()), __PRETTY_FUNCTION__);
        throw;
    }
}

void ASTBuilder::visit(NegativeExpressionPTNodePtr node)
{
    auto expr = visit_and_cast<AbstractExpressionASTNodePtr>(node->get_expr());

    SET_RESULT(NegativeExpressionASTNode, expr);
}

void ASTBuilder::visit(PositiveExpressionPTNodePtr node)
{
    node->get_expr()->accept(this);
    // passthrough
}

void ASTBuilder::visit(NotExpressionPTNodePtr node)
{
    auto expr = visit_and_cast<AbstractExpressionASTNodePtr>(node->get_expr());

    SET_RESULT(NotExpressionASTNode, expr);
}

void ASTBuilder::visit(IncrementExpressionPTNodePtr node)
{
    StringPtr var_name = node->get_var_name()->get_value();

    SET_RESULT(IncrementExpressionASTNode, var_name);
}

void ASTBuilder::visit(DecrementExpressionPTNodePtr node)
{
    StringPtr var_name = node->get_var_name()->get_value();

    SET_RESULT(DecrementExpressionASTNode, var_name);
}

void ASTBuilder::visit(FloorAccessPTNodePtr node)
{
    auto index = visit_and_cast<AbstractExpressionASTNodePtr>(node->get_index_expr());

    SET_RESULT(FloorAccessASTNode, index);
}

void ASTBuilder::visit(ParenthesizedExpressionPTNodePtr node)
{
    node->get_expr()->accept(this);
    // passthrough
}

void ASTBuilder::visit(InvocationExpressionPTNodePtr node)
{
    auto expr = visit_and_cast<AbstractExpressionASTNodePtr>(node->get_arg());
    SET_RESULT(InvocationExpressionASTNode, node->get_func_name()->get_value(), expr);
}

void ASTBuilder::visit(IfStatementPTNodePtr node)
{
    auto condition = visit_and_cast<AbstractExpressionASTNodePtr>(node->get_condition());
    auto then_body = visit_and_cast<AbstractEmbeddedStatementASTNodePtr>(node->get_then_stmt());
    auto else_body = visit_and_cast<AbstractEmbeddedStatementASTNodePtr>(node->get_else_stmt());
    SET_RESULT(IfStatementASTNode, condition, then_body, else_body);
}

void ASTBuilder::visit(WhileStatementPTNodePtr node)
{
    auto condition = visit_and_cast<AbstractExpressionASTNodePtr>(node->get_condition());
    auto body = visit_and_cast<AbstractEmbeddedStatementASTNodePtr>(node->get_body());
    SET_RESULT(WhileStatementASTNode, condition, body);
}

void ASTBuilder::visit(ForStatementPTNodePtr node)
{
    auto init = visit_and_cast<AbstractStatementASTNodePtr>(node->get_init_stmt());
    auto condition = visit_and_cast<AbstractExpressionASTNodePtr>(node->get_condition());
    auto update = visit_and_cast<AbstractStatementASTNodePtr>(node->get_update_stmt());
    auto body = visit_and_cast<AbstractEmbeddedStatementASTNodePtr>(node->get_body());
    SET_RESULT(ForStatementASTNode, init, condition, update, body);
}

void ASTBuilder::visit(ReturnStatementPTNodePtr node)
{
    auto expr = visit_and_cast<AbstractExpressionASTNodePtr>(node->get_expr());
    SET_RESULT(ReturnStatementASTNode, expr);
}

void ASTBuilder::visit(FloorBoxInitStatementPTNodePtr node)
{
    IntegerASTNodePtr index = make_literal_node(node->get_index());
    IntegerASTNodePtr val_expr = make_literal_node(node->get_value());
    FloorAssignmentASTNodePtr assignment = std::make_shared<FloorAssignmentASTNode>(node->lineno(), node->colno(), -1, -1, index, val_expr);
    SET_RESULT(FloorBoxInitStatementASTNode, assignment);
}

void ASTBuilder::visit(FloorMaxInitStatementPTNodePtr node)
{
    // This won't be called. Processed in parent.
    UNUSED(node);
    assert(false);
}

void ASTBuilder::visit(EmptyStatementPTNodePtr node)
{
    SET_RESULT(EmptyStatementASTNode);
}

void ASTBuilder::visit(StatementBlockPTNodePtr node)
{
    StatementsVector stmts;

    for (const auto &stmt : node->get_statements()) {
        stmts.push_back(visit_and_cast<AbstractStatementASTNodePtr>(stmt));
    }

    SET_RESULT(StatementBlockASTNode, stmts);
}

void ASTBuilder::visit(VariableDeclarationStatementPTNodePtr node)
{
    node->get_variable_decl()->accept(this);
    // result passed through to caller
}

void ASTBuilder::visit(VariableAssignmentStatementPTNodePtr node)
{
    node->get_variable_assignment()->accept(this);
    // result passed through to caller
}

void ASTBuilder::visit(FloorAssignmentStatementPTNodePtr node)
{
    node->get_floor_assignment()->accept(this);
    // result passed through to caller
}

void ASTBuilder::visit(InvocationStatementPTNodePtr node)
{
    node->get_expr()->accept(this);
    // result passed through to caller
}

void ASTBuilder::visit(SubprocDefinitionPTNodePtr node)
{
    StringPtr func_name = node->get_function_name()->get_value();
    auto param = node->get_formal_parameter();
    VariableDeclarationASTNodePtr param_node;

    if (param) {
        auto param_name = param->get_value();

        param_node = std::make_shared<VariableDeclarationASTNode>(
            param->lineno(), param->colno(), param->lineno(), param->colno() + static_cast<int>(param->get_token()->width()),
            param_name,
            nullptr);
    }

    auto stmt_block = visit_and_cast<StatementBlockASTNodePtr>(node->get_body());

    SET_RESULT(SubprocDefinitionASTNode, func_name, param_node, stmt_block);
}

void ASTBuilder::visit(FunctionDefinitionPTNodePtr node)
{
    StringPtr func_name = node->get_function_name()->get_value();
    auto param = node->get_formal_parameter();
    VariableDeclarationASTNodePtr param_node;

    if (param) {
        auto param_name = param->get_value();

        param_node = std::make_shared<VariableDeclarationASTNode>(
            param->lineno(), param->colno(), param->lineno(), param->colno() + static_cast<int>(param->get_token()->width()),
            param_name,
            nullptr);
    }

    auto stmt_block = visit_and_cast<StatementBlockASTNodePtr>(node->get_body());

    SET_RESULT(FunctionDefinitionASTNode, func_name, param_node, stmt_block);
}

void ASTBuilder::visit(ImportDirectivePTNodePtr node)
{
    // This won't be called. Processed in parent.
    UNUSED(node);
    assert(false);
}

void ASTBuilder::visit(CompilationUnitPTNodePtr node)
{
    std::vector<StringPtr> imports;
    for (const auto &import : node->get_imports()) {
        imports.push_back(import->get_module_name()->get_value());
    }

    std::vector<FloorBoxInitStatementASTNodePtr> floor_inits;
    for (const auto &init : node->get_floor_inits()) {
        floor_inits.push_back(visit_and_cast<FloorBoxInitStatementASTNodePtr>(init));
    }

    std::optional<int> floor_max = std::nullopt;
    if (node->get_floor_max()) {
        floor_max = node->get_floor_max()->get_value()->get_value();
    }

    std::vector<VariableDeclarationASTNodePtr> var_decls;
    for (const auto &var_decl : node->get_top_level_decls()) {
        var_decls.push_back(visit_and_cast<VariableDeclarationASTNodePtr>(var_decl));
    }

    std::vector<AbstractSubroutineASTNodePtr> subroutines;
    for (const auto &subroutine : node->get_subroutines()) {
        subroutines.push_back(visit_and_cast<AbstractSubroutineASTNodePtr>(subroutine));
    }

    SET_RESULT(CompilationUnitASTNode, imports, floor_inits, floor_max, var_decls, subroutines);
}

void ASTBuilder::visit(BreakContinueStatementPTNodePtr node)
{
    if (node->is_break()) {
        SET_RESULT(BreakStatementASTNode);
    } else {
        SET_RESULT(ContinueStatementASTNode);
    }
}

bool ASTBuilder::build(CompilationUnitASTNodePtr &result)
{
    result = visit_and_cast<CompilationUnitASTNodePtr>(_root);
    return true;
}

CLOSE_PARSER_NAMESPACE
// end
