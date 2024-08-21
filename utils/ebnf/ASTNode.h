#ifndef ASTNODE_H
#define ASTNODE_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "ASTNodeVisitor.h"

class ASTNode : public std::enable_shared_from_this<ASTNode> {
  public:
    ASTNode(int lineno, int colno) : _lineno(lineno), _colno(colno) {}
    virtual ~ASTNode() = default;

    virtual void accept(ASTNodeVisitor *visitor) = 0;

    int lineno() { return _lineno; }
    int colno() { return _colno; }

    template <typename T> std::shared_ptr<T> shared_from_this_casted() {
        return std::static_pointer_cast<T>(shared_from_this());
    }

  protected:
    int _lineno;
    int _colno;
};

using ASTNodePtr = std::shared_ptr<ASTNode>;

class SyntaxNode : public ASTNode {
  public:
    std::vector<ASTNodePtr> productions;

    explicit SyntaxNode(int lineno, int colno) : ASTNode(lineno, colno) {}

    virtual void accept(ASTNodeVisitor *visitor) override {
        visitor->accept(shared_from_this_casted<SyntaxNode>());
    };
};

// Production node
class ProductionNode : public ASTNode {
  public:
    std::string id;
    ASTNodePtr expression;

    ProductionNode(std::string id, ASTNodePtr expr, int lineno, int colno)
        : ASTNode(lineno, colno), id(std::move(id)),
          expression(std::move(expr)) {}

    virtual void accept(ASTNodeVisitor *visitor) override {
        visitor->accept(shared_from_this_casted<ProductionNode>());
    };
};

// Expression node
class ExpressionNode : public ASTNode {
  public:
    std::vector<ASTNodePtr> terms;
    explicit ExpressionNode(int lineno, int colno) : ASTNode(lineno, colno) {}

    void addTerm(ASTNodePtr term) { terms.push_back(std::move(term)); }

    virtual void accept(ASTNodeVisitor *visitor) override {
        visitor->accept(shared_from_this_casted<ExpressionNode>());
    };
};

// Term node
class TermNode : public ASTNode {
  public:
    std::vector<ASTNodePtr> factors;
    explicit TermNode(int lineno, int colno) : ASTNode(lineno, colno) {}

    void addFactor(ASTNodePtr factor) { factors.push_back(std::move(factor)); }

    virtual void accept(ASTNodeVisitor *visitor) override {
        visitor->accept(shared_from_this_casted<TermNode>());
    };
};

// Factor node
class FactorNode : public ASTNode {
  public:
    std::string value;
    ASTNodePtr value_node;

    explicit FactorNode(std::string value, int lineno, int colno)
        : ASTNode(lineno, colno), value(std::move(value)) {}
    explicit FactorNode(ASTNodePtr value, int lineno, int colno)
        : ASTNode(lineno, colno), value_node(value) {}

    virtual void accept(ASTNodeVisitor *visitor) override {
        visitor->accept(shared_from_this_casted<FactorNode>());
    };
};

// Optional node
class OptionalNode : public ASTNode {
  public:
    ASTNodePtr expression;

    explicit OptionalNode(ASTNodePtr expr, int lineno, int colno)
        : ASTNode(lineno, colno), expression(std::move(expr)) {}

    virtual void accept(ASTNodeVisitor *visitor) override {
        visitor->accept(shared_from_this_casted<OptionalNode>());
    };
};

// Repeated node
class RepeatedNode : public ASTNode {
  public:
    ASTNodePtr expression;

    explicit RepeatedNode(ASTNodePtr expr, int lineno, int colno)
        : ASTNode(lineno, colno), expression(std::move(expr)) {}

    virtual void accept(ASTNodeVisitor *visitor) override {
        visitor->accept(shared_from_this_casted<RepeatedNode>());
    };
};

// Grouped node
class GroupedNode : public ASTNode {
  public:
    ASTNodePtr expression;

    explicit GroupedNode(ASTNodePtr expr, int lineno, int colno)
        : ASTNode(lineno, colno), expression(std::move(expr)) {}

    virtual void accept(ASTNodeVisitor *visitor) override {
        visitor->accept(shared_from_this_casted<GroupedNode>());
    };
};

#endif