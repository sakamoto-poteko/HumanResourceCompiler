#ifndef ASTNODE_H
#define ASTNODE_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

class ASTNodeVisitor;

// Base class for all AST nodes
class ASTNode {
public:
  virtual ~ASTNode() = default;

  virtual void visit(ASTNodeVisitor *visitor){};
};

using ASTNodePtr = std::shared_ptr<ASTNode>;

// Syntax node (root of the AST)
class SyntaxNode : public ASTNode {
public:
  std::vector<ASTNodePtr> productions;
};

// Production node
class ProductionNode : public ASTNode {
public:
  std::string id;
  ASTNodePtr expression;

  ProductionNode(std::string id, ASTNodePtr expr)
      : id(std::move(id)), expression(std::move(expr)) {}
};

// Expression node
class ExpressionNode : public ASTNode {
public:
  std::vector<ASTNodePtr> terms;

  void addTerm(ASTNodePtr term) { terms.push_back(std::move(term)); }
};

// Term node
class TermNode : public ASTNode {
public:
  std::vector<ASTNodePtr> factors;

  void addFactor(ASTNodePtr factor) { factors.push_back(std::move(factor)); }
};

// Factor node
class FactorNode : public ASTNode {
public:
  std::string value;
  ASTNodePtr value_node;

  explicit FactorNode(std::string value) : value(std::move(value)) {}
  explicit FactorNode(ASTNodePtr value) : value_node(value) {}
};

// Optional node
class OptionalNode : public ASTNode {
public:
  ASTNodePtr expression;

  explicit OptionalNode(ASTNodePtr expr) : expression(std::move(expr)) {}
};

// Repeated node
class RepeatedNode : public ASTNode {
public:
  ASTNodePtr expression;

  explicit RepeatedNode(ASTNodePtr expr) : expression(std::move(expr)) {}
};

// Grouped node
class GroupedNode : public ASTNode {
public:
  ASTNodePtr expression;

  explicit GroupedNode(ASTNodePtr expr) : expression(std::move(expr)) {}
};

#endif