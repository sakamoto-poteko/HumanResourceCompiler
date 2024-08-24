#include "ASTNode.h"
#include "ASTNodeVisitor.h"

void SyntaxNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<SyntaxNode>());
};
void ProductionNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<ProductionNode>());
};
void ExpressionNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<ExpressionNode>());
};
void TermNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<TermNode>());
};
void FactorNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<FactorNode>());
};
void IdentifierNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<IdentifierNode>());
};
void LiteralNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<LiteralNode>());
};
void OptionalNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<OptionalNode>());
};
void RepeatedNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<RepeatedNode>());
};
void GroupedNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<GroupedNode>());
};