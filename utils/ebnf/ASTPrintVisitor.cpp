#include <iostream>

#include "ASTNode.h"
#include "ASTNodeVisitor.h"

void ASTPrintVisitor::accept(std::shared_ptr<SyntaxNode> node) {
    for (const auto &prod : node->productions) {
        prod->accept(this);
    }
}

void ASTPrintVisitor::accept(std::shared_ptr<ProductionNode> node) {
    std::cout << node->id << " ::= ";
    node->expression->accept(this);
    std::cout << std::endl;
}

void ASTPrintVisitor::accept(std::shared_ptr<ExpressionNode> node) {
    bool first = true;
    for (const auto &term : node->terms) {
        if (first) {
            first = false;
        } else {
            std::cout << "|";
        }
        term->accept(this);
    }
}

void ASTPrintVisitor::accept(std::shared_ptr<TermNode> node) {
    for (const auto &factor : node->factors) {
        factor->accept(this);
    }
}

void ASTPrintVisitor::accept(std::shared_ptr<FactorNode> node) {
    if (node->value.empty()) {
        if (node->value_node) {
            node->value_node->accept(this);
        }
    } else {
        std::cout << " " << node->value << " ";
    }
}

void ASTPrintVisitor::accept(std::shared_ptr<OptionalNode> node) {
    std::cout << "[";
    node->expression->accept(this);
    std::cout << "]";
}

void ASTPrintVisitor::accept(std::shared_ptr<RepeatedNode> node) {
    std::cout << "{";
    node->expression->accept(this);
    std::cout << "}";
}

void ASTPrintVisitor::accept(std::shared_ptr<GroupedNode> node) {
    std::cout << "(";
    node->expression->accept(this);
    std::cout << ")";
}
