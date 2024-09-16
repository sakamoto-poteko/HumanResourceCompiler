#include "ASTNode.h"
#include "ASTNodeVisitor.h"
#include "parser_global.h"
#include <type_traits>

OPEN_PARSER_NAMESPACE

#define VISIT_NODE() \
    return visitor->visit(shared_from_this_casted<std::remove_reference_t<decltype(*this)>>())

// begin

int BooleanASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int IntegerASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int VariableDeclarationASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int VariableAssignmentASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int VariableAccessASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int FloorAssignmentASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int FloorAccessASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int NegativeExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int IncrementExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int DecrementExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int NotExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int AddExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int SubExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int MulExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int DivExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int ModExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int EqualExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int NotEqualExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int GreaterThanExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int GreaterEqualExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int LessThanExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int LessEqualExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int AndExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int OrExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int InvocationExpressionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int IfStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int WhileStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int ForStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int ReturnStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int StatementBlockASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int SubprocDefinitionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int FunctionDefinitionASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int CompilationUnitASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int EmptyStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int FloorBoxInitStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int BreakStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

int ContinueStatementASTNode::accept(ASTNodeVisitor *visitor) { VISIT_NODE(); }

bool ASTNode::get_attribute(int attribute_id, ASTNodeAttributePtr &out) const
{
    auto it = _attributes.find(attribute_id);
    if (it == _attributes.end()) {
        return false;
    } else {
        if (it->second) {
            out = it->second;
            return true;
        } else {
            return false;
        }
    }
}

void ASTNode::set_attribute(int attribute_id, ASTNodeAttributePtr attr)
{
    if (attr) {
        _attributes[attribute_id] = attr;
    }
}

void ASTNode::remove_attribute(int attribute_id)
{
    _attributes.erase(attribute_id);
}

void ASTNode::copy_attributes_from(const ASTNodePtr &node)
{
    _attributes = node->_attributes;
}

const char *ast_node_type_to_string(ASTNodeType type)
{
    switch (type) {
    case ASTNodeType::EmptyStatement:
        return "EmptyStatement";
    case ASTNodeType::Integer:
        return "Integer";
    case ASTNodeType::Boolean:
        return "Boolean";
    case ASTNodeType::VariableDeclaration:
        return "VariableDeclaration";
    case ASTNodeType::VariableAssignment:
        return "VariableAssignment";
    case ASTNodeType::VariableAccess:
        return "VariableAccess";
    case ASTNodeType::FloorBoxInitStatement:
        return "FloorBoxInitStatement";
    case ASTNodeType::FloorAssignment:
        return "FloorAssignment";
    case ASTNodeType::FloorAccess:
        return "FloorAccess";
    case ASTNodeType::NegativeExpression:
        return "NegativeExpression";
    case ASTNodeType::NotExpression:
        return "NotExpression";
    case ASTNodeType::IncrementExpression:
        return "IncrementExpression";
    case ASTNodeType::DecrementExpression:
        return "DecrementExpression";
    case ASTNodeType::AddExpression:
        return "AddExpression";
    case ASTNodeType::SubExpression:
        return "SubExpression";
    case ASTNodeType::MulExpression:
        return "MulExpression";
    case ASTNodeType::DivExpression:
        return "DivExpression";
    case ASTNodeType::ModExpression:
        return "ModExpression";
    case ASTNodeType::EqualExpression:
        return "EqualExpression";
    case ASTNodeType::NotEqualExpression:
        return "NotEqualExpression";
    case ASTNodeType::GreaterThanExpression:
        return "GreaterThanExpression";
    case ASTNodeType::GreaterEqualExpression:
        return "GreaterEqualExpression";
    case ASTNodeType::LessThanExpression:
        return "LessThanExpression";
    case ASTNodeType::LessEqualExpression:
        return "LessEqualExpression";
    case ASTNodeType::AndExpression:
        return "AndExpression";
    case ASTNodeType::OrExpression:
        return "OrExpression";
    case ASTNodeType::InvocationExpression:
        return "InvocationExpression";
    case ASTNodeType::IfStatement:
        return "IfStatement";
    case ASTNodeType::WhileStatement:
        return "WhileStatement";
    case ASTNodeType::ForStatement:
        return "ForStatement";
    case ASTNodeType::ReturnStatement:
        return "ReturnStatement";
    case ASTNodeType::BreakStatement:
        return "BreakStatement";
    case ASTNodeType::ContinueStatement:
        return "ContinueStatement";
    case ASTNodeType::StatementBlock:
        return "StatementBlock";
    case ASTNodeType::SubprocDefinition:
        return "SubprocDefinition";
    case ASTNodeType::FunctionDefinition:
        return "FunctionDefinition";
    case ASTNodeType::CompilationUnit:
        return "CompilationUnit";
    default:
        return "Unknown";
    }
}

CLOSE_PARSER_NAMESPACE

// end
