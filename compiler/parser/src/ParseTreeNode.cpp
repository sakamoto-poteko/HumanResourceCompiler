#include "ParseTreeNode.h"
#include "ParseTreeNodeVisitor.h"

OPEN_PARSER_NAMESPACE

void IdentifierNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<IdentifierNode>());
}

void IntegerLiteralNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<IntegerLiteralNode>());
}

void BooleanLiteralNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<BooleanLiteralNode>());
}

void BinaryOperatorNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<BinaryOperatorNode>());
}

void VariableDeclarationNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<VariableDeclarationNode>());
}

void VariableAssignmentNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<VariableAssignmentNode>());
}

void FloorAssignmentNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FloorAssignmentNode>());
}

void BinaryExpressionNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<BinaryExpressionNode>());
}

void IncrementExpressionNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<IncrementExpressionNode>());
}

void DecrementExpressionNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<DecrementExpressionNode>());
}

void FloorAccessNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FloorAccessNode>());
}

void ParenthesizedExpressionNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<ParenthesizedExpressionNode>());
}

void InvocationExpressionNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<InvocationExpressionNode>());
}

void IfStatementNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<IfStatementNode>());
}

void WhileStatementNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<WhileStatementNode>());
}

void ForStatementNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<ForStatementNode>());
}

void ReturnStatementNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<ReturnStatementNode>());
}

void FloorBoxInitStatementNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FloorBoxInitStatementNode>());
}

void FloorMaxInitStatementNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FloorMaxInitStatementNode>());
}

void EmptyStatementNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<EmptyStatementNode>());
}

void StatementBlockNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<StatementBlockNode>());
}

void SubprocDefinitionNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<SubprocDefinitionNode>());
}

void FunctionDefinitionNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FunctionDefinitionNode>());
}

void ImportDirectiveNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<ImportDirectiveNode>());
}

void CompilationUnitNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<CompilationUnitNode>());
}

void VariableDeclarationStatementNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<VariableDeclarationStatementNode>());
}

void VariableAssignmentStatementNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<VariableAssignmentStatementNode>());
}

void FloorAssignmentStatementNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FloorAssignmentStatementNode>());
}

void NegativeExpressionNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<NegativeExpressionNode>());
}

void PositiveExpressionNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<PositiveExpressionNode>());
}

void NotExpressionNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<NotExpressionNode>());
}

void InvocationStatementNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<InvocationStatementNode>());
}

CLOSE_PARSER_NAMESPACE
// end
