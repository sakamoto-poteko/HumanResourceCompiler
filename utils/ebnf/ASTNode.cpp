#include "ASTNode.h"
#include "ASTNodeVisitor.h"

void SyntaxNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<SyntaxNode>());
}

void ProductionNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<ProductionNode>());
}

void ExpressionNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<ExpressionNode>());
}

void TermNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<TermNode>());
}

void FactorNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<FactorNode>());
}

void IdentifierNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<IdentifierNode>());
}

void LiteralNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<LiteralNode>());
}

void OptionalNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<OptionalNode>());
}

void RepeatedNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<RepeatedNode>());
}

void GroupedNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<GroupedNode>());
}

void EpsilonNode::accept(ASTNodeVisitor *visitor)
{
    visitor->accept(shared_from_this_casted<EpsilonNode>());
}

const char *EpsilonNode::name()
{
    return "Epsilon";
}

const char *GroupedNode::name()
{
    return "Grouped";
}

const char *RepeatedNode::name()
{
    return "Repeated";
}

const char *OptionalNode::name()
{
    return "Optional";
}

const char *LiteralNode::name()
{
    return "Literal";
}

const char *IdentifierNode::name()
{
    return "Identifier";
}

const char *FactorNode::name()
{
    return "Factor";
}

const char *TermNode::name()
{
    return "Term";
}

const char *ExpressionNode::name()
{
    return "Expression";
}

const char *ProductionNode::name()
{
    return "Production";
}

const char *SyntaxNode::name()
{
    return "Syntax";
}
