#ifndef ASTDEPGRAPHBUILDERVISITOR_H
#define ASTDEPGRAPHBUILDERVISITOR_H

#include "ASTExtendedInfo.h"
#include "ASTNode.h"
#include "ASTNodeVisitor.h"

class ASTDepGraphBuilderVisitor : public ASTNodeVisitor {
  public:
    ASTDepGraphBuilderVisitor();
    virtual ~ASTDepGraphBuilderVisitor();

    virtual void accept(std::shared_ptr<SyntaxNode> node) override;
    virtual void accept(std::shared_ptr<ProductionNode> node) override;
    virtual void accept(std::shared_ptr<ExpressionNode> node) override;
    virtual void accept(std::shared_ptr<TermNode> node) override;
    virtual void accept(std::shared_ptr<FactorNode> node) override;
    virtual void accept(std::shared_ptr<OptionalNode> node) override;
    virtual void accept(std::shared_ptr<RepeatedNode> node) override;
    virtual void accept(std::shared_ptr<GroupedNode> node) override;

  protected:
    ASTExtendedInfo _info;

  private:
};

#endif