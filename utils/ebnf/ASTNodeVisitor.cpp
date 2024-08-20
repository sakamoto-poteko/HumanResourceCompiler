#include <iostream>

#include "ASTNodeVisitor.h"

ASTNodeVisitor::ASTNodeVisitor() {}

ASTNodeVisitor::~ASTNodeVisitor() {}

void ASTPrintVisitor::visit(ASTNodePtr node) {
  std::cout << typeid(node).name() << std::endl;
  node.get()->visit(this);
}

void ASTPrintVisitor::visit(std::shared_ptr<SyntaxNode> node) {
  std::cout << "called!" << std::endl;
}
