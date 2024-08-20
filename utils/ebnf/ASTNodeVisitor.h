#ifndef ASTNODEVISITOR_H
#define ASTNODEVISITOR_H

class SyntaxNode;
class ProductionNode;
class ExpressionNode;
class TermNode;
class FactorNode;
class OptionalNode;
class RepeatedNode;
class GroupedNode;

class ASTNodeVisitor {
  public:
    ASTNodeVisitor();
    virtual ~ASTNodeVisitor();
    virtual void accept(SyntaxNode *node) = 0;
    virtual void accept(ProductionNode *node) = 0;
    virtual void accept(ExpressionNode *node) = 0;
    virtual void accept(TermNode *node) = 0;
    virtual void accept(FactorNode *node) = 0;
    virtual void accept(OptionalNode *node) = 0;
    virtual void accept(RepeatedNode *node) = 0;
    virtual void accept(GroupedNode *node) = 0;
};

class ASTPrintVisitor : public ASTNodeVisitor {
  public:
    virtual ~ASTPrintVisitor() = default;
    virtual void accept(SyntaxNode *node) override;
    virtual void accept(ProductionNode *node) override;
    virtual void accept(ExpressionNode *node) override;
    virtual void accept(TermNode *node) override;
    virtual void accept(FactorNode *node) override;
    virtual void accept(OptionalNode *node) override;
    virtual void accept(RepeatedNode *node) override;
    virtual void accept(GroupedNode *node) override;
};

#endif