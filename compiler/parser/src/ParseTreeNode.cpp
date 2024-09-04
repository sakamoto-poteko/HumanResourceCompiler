#include "ParseTreeNode.h"
#include "ParseTreeNodeVisitor.h"

OPEN_PARSER_NAMESPACE

void IdentifierPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<IdentifierPTNode>());
}

void IntegerLiteralPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<IntegerLiteralPTNode>());
}

void BooleanLiteralPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<BooleanLiteralPTNode>());
}

void BinaryOperatorPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<BinaryOperatorPTNode>());
}

void VariableDeclarationPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<VariableDeclarationPTNode>());
}

void VariableAssignmentPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<VariableAssignmentPTNode>());
}

void FloorAssignmentPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FloorAssignmentPTNode>());
}

void BinaryExpressionPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<BinaryExpressionPTNode>());
}

void IncrementExpressionPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<IncrementExpressionPTNode>());
}

void DecrementExpressionPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<DecrementExpressionPTNode>());
}

void FloorAccessPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FloorAccessPTNode>());
}

void ParenthesizedExpressionPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<ParenthesizedExpressionPTNode>());
}

void InvocationExpressionPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<InvocationExpressionPTNode>());
}

void IfStatementPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<IfStatementPTNode>());
}

void WhileStatementPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<WhileStatementPTNode>());
}

void ForStatementPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<ForStatementPTNode>());
}

void ReturnStatementPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<ReturnStatementPTNode>());
}

void FloorBoxInitStatementPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FloorBoxInitStatementPTNode>());
}

void FloorMaxInitStatementPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FloorMaxInitStatementPTNode>());
}

void EmptyStatementPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<EmptyStatementPTNode>());
}

void StatementBlockPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<StatementBlockPTNode>());
}

void SubprocDefinitionPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<SubprocDefinitionPTNode>());
}

void FunctionDefinitionPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FunctionDefinitionPTNode>());
}

void ImportDirectivePTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<ImportDirectivePTNode>());
}

void CompilationUnitPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<CompilationUnitPTNode>());
}

void VariableDeclarationStatementPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<VariableDeclarationStatementPTNode>());
}

void VariableAssignmentStatementPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<VariableAssignmentStatementPTNode>());
}

void FloorAssignmentStatementPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<FloorAssignmentStatementPTNode>());
}

void NegativeExpressionPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<NegativeExpressionPTNode>());
}

void PositiveExpressionPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<PositiveExpressionPTNode>());
}

void NotExpressionPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<NotExpressionPTNode>());
}

void InvocationStatementPTNode::accept(ParseTreeNodeVisitor *visitor)
{
    visitor->visit(shared_from_this_casted<InvocationStatementPTNode>());
}

CLOSE_PARSER_NAMESPACE
// end
