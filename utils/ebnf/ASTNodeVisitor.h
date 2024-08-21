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

    virtual void accept(std::shared_ptr<SyntaxNode> node) = 0;
    virtual void accept(std::shared_ptr<ProductionNode> node) = 0;
    virtual void accept(std::shared_ptr<ExpressionNode> node) = 0;
    virtual void accept(std::shared_ptr<TermNode> node) = 0;
    virtual void accept(std::shared_ptr<FactorNode> node) = 0;
    virtual void accept(std::shared_ptr<OptionalNode> node) = 0;
    virtual void accept(std::shared_ptr<RepeatedNode> node) = 0;
    virtual void accept(std::shared_ptr<GroupedNode> node) = 0;
};

class ASTPrintVisitor : public ASTNodeVisitor {
  public:
    virtual ~ASTPrintVisitor() = default;

    virtual void accept(std::shared_ptr<SyntaxNode> node) override;
    virtual void accept(std::shared_ptr<ProductionNode> node) override;
    virtual void accept(std::shared_ptr<ExpressionNode> node) override;
    virtual void accept(std::shared_ptr<TermNode> node) override;
    virtual void accept(std::shared_ptr<FactorNode> node) override;
    virtual void accept(std::shared_ptr<OptionalNode> node) override;
    virtual void accept(std::shared_ptr<RepeatedNode> node) override;
    virtual void accept(std::shared_ptr<GroupedNode> node) override;
};

#endif