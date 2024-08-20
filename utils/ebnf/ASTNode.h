#ifndef ASTNODE_H
#define ASTNODE_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "ASTNodeVisitor.h"

class ASTNode {
  public:
    virtual ~ASTNode() = default;

    virtual void accept(ASTNodeVisitor *visitor) = 0;
};

using ASTNodePtr = std::shared_ptr<ASTNode>;

class SyntaxNode : public ASTNode {
  public:
    std::vector<ASTNodePtr> productions;

    virtual void accept(ASTNodeVisitor *visitor) override {
        visitor->accept(this);
    };
};

// Production node
class ProductionNode : public ASTNode {
  public:
    std::string id;
    ASTNodePtr expression;

    ProductionNode(std::string id, ASTNodePtr expr)
        : id(std::move(id)), expression(std::move(expr)) {}

    virtual void accept(ASTNodeVisitor *visitor) override {
        visitor->accept(this);
    };
};

// Expression node
class ExpressionNode : public ASTNode {
  public:
    std::vector<ASTNodePtr> terms;

    void addTerm(ASTNodePtr term) { terms.push_back(std::move(term)); }

    virtual void accept(ASTNodeVisitor *visitor) override {
        visitor->accept(this);
    };
};

// Term node
class TermNode : public ASTNode {
  public:
    std::vector<ASTNodePtr> factors;

    void addFactor(ASTNodePtr factor) { factors.push_back(std::move(factor)); }

    virtual void accept(ASTNodeVisitor *visitor) override {
        visitor->accept(this);
    };
};

// Factor node
class FactorNode : public ASTNode {
  public:
    std::string value;
    ASTNodePtr value_node;

    explicit FactorNode(std::string value) : value(std::move(value)) {}
    explicit FactorNode(ASTNodePtr value) : value_node(value) {}

    virtual void accept(ASTNodeVisitor *visitor) override {
        visitor->accept(this);
    };
};

// Optional node
class OptionalNode : public ASTNode {
  public:
    ASTNodePtr expression;

    explicit OptionalNode(ASTNodePtr expr) : expression(std::move(expr)) {}

    virtual void accept(ASTNodeVisitor *visitor) override {
        visitor->accept(this);
    };
};

// Repeated node
class RepeatedNode : public ASTNode {
  public:
    ASTNodePtr expression;

    explicit RepeatedNode(ASTNodePtr expr) : expression(std::move(expr)) {}

    virtual void accept(ASTNodeVisitor *visitor) override {
        visitor->accept(this);
    };
};

// Grouped node
class GroupedNode : public ASTNode {
  public:
    ASTNodePtr expression;

    explicit GroupedNode(ASTNodePtr expr) : expression(std::move(expr)) {}

    virtual void accept(ASTNodeVisitor *visitor) override {
        visitor->accept(this);
    };
};

#endif