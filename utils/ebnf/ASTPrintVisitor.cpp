#include <iostream>

#include "ParseTreeNode.h"
#include "ParseTreeNodeVisitor.h"

int ParseTreePrintVisitor::accept(SyntaxNodePtr node)
{
    for (const auto &prod : node->productions) {
        prod->accept(this);
    }
    return 0;
}

int ParseTreePrintVisitor::accept(ProductionNodePtr node)
{
    std::cout << node->id << " ::= ";
    node->expression->accept(this);
    std::cout << std::endl;
    return 0;
}

int ParseTreePrintVisitor::accept(ExpressionNodePtr node)
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

int ParseTreePrintVisitor::accept(TermNodePtr node)
{
    for (const auto &factor : node->factors) {
        factor->accept(this);
        std::cout << " ";
    }
    return 0;
}

int ParseTreePrintVisitor::accept(FactorNodePtr node)
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

int ParseTreePrintVisitor::accept(OptionalNodePtr node)
{
    std::cout << "[ ";
    node->expression->accept(this);
    std::cout << "]";
    return 0;
}

int ParseTreePrintVisitor::accept(RepeatedNodePtr node)
{
    std::cout << "{ ";
    node->expression->accept(this);
    std::cout << "}";
    return 0;
}

int ParseTreePrintVisitor::accept(GroupedNodePtr node)
{
    std::cout << "( ";
    node->expression->accept(this);
    std::cout << ")";
    return 0;
}

int ParseTreePrintVisitor::accept(IdentifierNodePtr node)
{
    std::cout << node->value;
    return 0;
}

int ParseTreePrintVisitor::accept(LiteralNodePtr node)
{
    std::cout << node->value;
    return 0;
}

int ParseTreePrintVisitor::accept(EpsilonNodePtr node)
{
    std::cout << "(epsilon)";
    return 0;
}
