#include <iostream>

#include "ASTNode.h"
#include "ASTNodeVisitor.h"

int ASTPrintVisitor::accept(SyntaxNodePtr node)
{
    for (const auto &prod : node->productions) {
        prod->accept(this);
    }
    return 0;
}

int ASTPrintVisitor::accept(ProductionNodePtr node)
{
    std::cout << node->id << " ::= ";
    node->expression->accept(this);
    std::cout << std::endl;
    return 0;
}

int ASTPrintVisitor::accept(ExpressionNodePtr node)
{
    bool first = true;
    for (const auto &term : node->terms) {
        if (first) {
            first = false;
        } else {
            std::cout << "| ";
        }
        term->accept(this);
    }
    return 0;
}

int ASTPrintVisitor::accept(TermNodePtr node)
{
    for (const auto &factor : node->factors) {
        factor->accept(this);
        std::cout << " ";
    }
    return 0;
}

int ASTPrintVisitor::accept(FactorNodePtr node)
{
    if (node->node) {
        node->node->accept(this);
    } else {
        if (node->identifier) {
            node->identifier->accept(this);
        } else if (node->literal) {
            node->literal->accept(this);
        } else {
            throw;
        }
    }
    return 0;
}

int ASTPrintVisitor::accept(OptionalNodePtr node)
{
    std::cout << "[ ";
    node->expression->accept(this);
    std::cout << "]";
    return 0;
}

int ASTPrintVisitor::accept(RepeatedNodePtr node)
{
    std::cout << "{ ";
    node->expression->accept(this);
    std::cout << "}";
    return 0;
}

int ASTPrintVisitor::accept(GroupedNodePtr node)
{
    std::cout << "( ";
    node->expression->accept(this);
    std::cout << ")";
    return 0;
}

int ASTPrintVisitor::accept(IdentifierNodePtr node)
{
    std::cout << node->value;
    return 0;
}

int ASTPrintVisitor::accept(LiteralNodePtr node)
{
    std::cout << node->value;
    return 0;
}
