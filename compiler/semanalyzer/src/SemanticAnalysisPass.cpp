#include <spdlog/spdlog.h>

#include "SemanticAnalysisPass.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

void SemanticAnalysisPass::enter_node(const parser::ASTNodePtr &node)
{
    _ancestors.push_back(node);
}

void SemanticAnalysisPass::leave_node()
{
    assert(!_ancestors.empty());
    _ancestors.pop_back();
}

int SemanticAnalysisPass::visit(const parser::IntegerASTNodePtr &node)
{

    enter_node(node);
    leave_node();
    return 0;
}

int SemanticAnalysisPass::visit(const parser::BooleanASTNodePtr &node)
{

    enter_node(node);
    leave_node();
    return 0;
}

int SemanticAnalysisPass::visit(const parser::VariableDeclarationASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse(node->get_assignment());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::VariableAssignmentASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse(node->get_value());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::VariableAccessASTNodePtr &node)
{

    enter_node(node);
    leave_node();
    return 0;
}

int SemanticAnalysisPass::visit(const parser::FloorBoxInitStatementASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse(node->get_assignment());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::FloorAssignmentASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_floor_number(), node->get_value());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::FloorAccessASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse(node->get_index_expr());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::NegativeExpressionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse(node->get_operand());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::NotExpressionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse(node->get_operand());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::IncrementExpressionASTNodePtr &node)
{
    enter_node(node);
    leave_node();
    return 0;
}

int SemanticAnalysisPass::visit(const parser::DecrementExpressionASTNodePtr &node)
{
    enter_node(node);
    leave_node();
    return 0;
}

int SemanticAnalysisPass::visit(const parser::AddExpressionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::SubExpressionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::MulExpressionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::DivExpressionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::ModExpressionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::EqualExpressionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::NotEqualExpressionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::GreaterThanExpressionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::GreaterEqualExpressionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::LessThanExpressionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::LessEqualExpressionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::AndExpressionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::OrExpressionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_left(), node->get_right());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::InvocationExpressionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse(node->get_argument());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::EmptyStatementASTNodePtr &node)
{
    enter_node(node);
    leave_node();
    return 0;
}

int SemanticAnalysisPass::visit(const parser::IfStatementASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_condition(), node->get_then_branch(), node->get_else_branch());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::WhileStatementASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_condition(), node->get_body());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::ForStatementASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_init(), node->get_condition(), node->get_update(), node->get_body());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::ReturnStatementASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse(node->get_expression());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::BreakStatementASTNodePtr &node)
{
    enter_node(node);
    leave_node();
    return 0;
}

int SemanticAnalysisPass::visit(const parser::ContinueStatementASTNodePtr &node)
{
    enter_node(node);
    leave_node();
    return 0;
}

int SemanticAnalysisPass::visit(const parser::StatementBlockASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse(node->get_statements());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::SubprocDefinitionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_parameter(), node->get_body());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::FunctionDefinitionASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_parameter(), node->get_body());
    leave_node();
    return rc;
}

int SemanticAnalysisPass::visit(const parser::CompilationUnitASTNodePtr &node)
{
    enter_node(node);
    int rc = traverse_multiple(node->get_floor_inits(), node->get_var_decls(), node->get_subroutines());
    leave_node();
    return rc;
}

void SemanticAnalysisPass::request_to_replace_self(parser::ASTNodePtr to_be_replaced_with)
{
    _node_replacement_requests[_ancestors.back()] = to_be_replaced_with;
}

void SemanticAnalysisPass::request_to_remove_self()
{
    _node_removal_requests.insert(_ancestors.back());
}

CLOSE_SEMANALYZER_NAMESPACE
// end
