#include <spdlog/spdlog.h>

#include "SemanticAnalysisPass.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

void SemanticAnalysisPass::enter_node(parser::ASTNodePtr node)
{
    _ancestors.push(node);
    _replace_node_asked_by_child_guard.push(0);
}

void SemanticAnalysisPass::leave_node()
{
    _ancestors.pop();
    _replace_node_asked_by_child_guard.pop();
}

int SemanticAnalysisPass::visit(parser::IntegerASTNodePtr node)
{
    UNUSED(node);
    return 0;
}

int SemanticAnalysisPass::visit(parser::BooleanASTNodePtr node)
{
    UNUSED(node);
    return 0;
}

int SemanticAnalysisPass::visit(parser::VariableDeclarationASTNodePtr node)
{
    enter_node(node);
    int rc = traverse(node->get_assignment());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::VariableAssignmentASTNodePtr node)
{
    enter_node(node);
    int rc = traverse(node->get_value());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::VariableAccessASTNodePtr node)
{
    UNUSED(node);
    return 0;
}

int SemanticAnalysisPass::visit(parser::FloorBoxInitStatementASTNodePtr node)
{
    enter_node(node);
    int rc = traverse(node->get_assignment());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::FloorAssignmentASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_floor_number(), node->get_value());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::FloorAccessASTNodePtr node)
{
    enter_node(node);
    int rc = traverse(node->get_index_expr());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::NegativeExpressionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse(node->get_operand());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::NotExpressionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse(node->get_operand());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::IncrementExpressionASTNodePtr node)
{
    UNUSED(node);
    return 0;
}

int SemanticAnalysisPass::visit(parser::DecrementExpressionASTNodePtr node)
{
    UNUSED(node);
    return 0;
}

int SemanticAnalysisPass::visit(parser::AddExpressionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::SubExpressionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::MulExpressionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::DivExpressionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::ModExpressionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::EqualExpressionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::NotEqualExpressionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::GreaterThanExpressionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::GreaterEqualExpressionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::LessThanExpressionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::LessEqualExpressionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::AndExpressionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::OrExpressionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::InvocationExpressionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse(node->get_argument());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::EmptyStatementASTNodePtr node)
{
    return 0;
}

int SemanticAnalysisPass::visit(parser::IfStatementASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_condition(), node->get_then_branch(), node->get_else_branch());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::WhileStatementASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_condition(), node->get_body());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::ForStatementASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_init(), node->get_condition(), node->get_update(), node->get_body());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::ReturnStatementASTNodePtr node)
{
    enter_node(node);
    int rc = traverse(node->get_expression());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::BreakStatementASTNodePtr node)
{
    UNUSED(node);
    return 0;
}

int SemanticAnalysisPass::visit(parser::ContinueStatementASTNodePtr node)
{
    UNUSED(node);
    return 0;
}

int SemanticAnalysisPass::visit(parser::StatementBlockASTNodePtr node)
{
    enter_node(node);
    int rc = traverse(node->get_statements());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::SubprocDefinitionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse(node->get_body());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::FunctionDefinitionASTNodePtr node)
{
    enter_node(node);
    int rc = traverse(node->get_body());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(parser::CompilationUnitASTNodePtr node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_floor_inits(), node->get_var_decls(), node->get_subroutines());
    leave_node();
    return rc;
}

void SemanticAnalysisPass::request_to_replace_self(parser::ASTNodePtr to_be_replaced_with)
{
    if (_replace_node_asked_by_child_guard.top() == 1) {
        spdlog::warn("[{}:{}] requested node replacement more than once", _ancestors.top()->lineno(), _ancestors.top()->colno());
        _replace_node_asked_by_child.pop();
    }
    _replace_node_asked_by_child.push(to_be_replaced_with);
}

CLOSE_SEMANALYZER_NAMESPACE
// end
