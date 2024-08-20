#ifndef ASTNODEVISITOR_H
#define ASTNODEVISITOR_H

#include "ASTNode.h"

class ASTNodeVisitor {
public:
  ASTNodeVisitor();
  virtual ~ASTNodeVisitor();
  virtual void visit(ASTNodePtr node) = 0;

private:
};

class ASTPrintVisitor : public ASTNodeVisitor {
public:
  virtual ~ASTPrintVisitor() = default;
  virtual void visit(ASTNodePtr node) override;
  virtual void visit(std::shared_ptr<SyntaxNode> node);
};

#endif