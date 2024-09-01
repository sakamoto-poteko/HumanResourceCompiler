#include "ASTNode.h"
#include "ASTNodeVisitor.h"

OPEN_PARSER_NAMESPACE

void IdentifierNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<IdentifierNode>());
}

void IntegerLiteralNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<IntegerLiteralNode>());
}

void BooleanLiteralNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<BooleanLiteralNode>());
}

void BinaryOperatorNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<BinaryOperatorNode>());
}

void VariableDeclarationNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<VariableDeclarationNode>());
}

void VariableAssignmentNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<VariableAssignmentNode>());
}

void FloorAssignmentNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FloorAssignmentNode>());
}

void BinaryExpressionNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<BinaryExpressionNode>());
}

void IncrementExpressionNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<IncrementExpressionNode>());
}

void DecrementExpressionNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<DecrementExpressionNode>());
}

void FloorAccessNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FloorAccessNode>());
}

void ParenthesizedExpressionNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<ParenthesizedExpressionNode>());
}

void InvocationExpressionNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<InvocationExpressionNode>());
}

void IfStatementNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<IfStatementNode>());
}

void WhileStatementNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<WhileStatementNode>());
}

void ForStatementNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<ForStatementNode>());
}

void ReturnStatementNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<ReturnStatementNode>());
}

void FloorBoxInitStatementNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FloorBoxInitStatementNode>());
}

void FloorMaxInitStatementNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FloorMaxInitStatementNode>());
}

void EmptyStatementNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<EmptyStatementNode>());
}

void StatementBlockNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<StatementBlockNode>());
}

void SubprocDefinitionNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<SubprocDefinitionNode>());
}

void FunctionDefinitionNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FunctionDefinitionNode>());
}

void ImportDirectiveNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<ImportDirectiveNode>());
}

void CompilationUnitNode::accept(ASTNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<CompilationUnitNode>());
}

CLOSE_PARSER_NAMESPACE