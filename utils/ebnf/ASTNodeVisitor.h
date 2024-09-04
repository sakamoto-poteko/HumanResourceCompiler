#ifndef ASTNODEVISITOR_H
#define ASTNODEVISITOR_H

#include "ASTNode.h"
#include "ASTNodeForward.h"

class ASTNodeVisitor {
public:
    ASTNodeVisitor();
    virtual ~ASTNodeVisitor();

    virtual int accept(SyntaxNodePtr node) = 0;
    virtual int accept(ProductionNodePtr node) = 0;
    virtual int accept(ExpressionNodePtr node) = 0;
    virtual int accept(TermNodePtr node) = 0;
    virtual int accept(FactorNodePtr node) = 0;
    virtual int accept(OptionalNodePtr node) = 0;
    virtual int accept(RepeatedNodePtr node) = 0;
    virtual int accept(GroupedNodePtr node) = 0;
    virtual int accept(IdentifierNodePtr node) = 0;
    virtual int accept(LiteralNodePtr node) = 0;
    virtual int accept(EpsilonNodePtr node) = 0;
};

class ASTPrintVisitor : public ASTNodeVisitor {
public:
    virtual ~ASTPrintVisitor() = default;

    virtual int accept(SyntaxNodePtr node) override;
    virtual int accept(ProductionNodePtr node) override;
    virtual int accept(ExpressionNodePtr node) override;
    virtual int accept(TermNodePtr node) override;
    virtual int accept(FactorNodePtr node) override;
    virtual int accept(OptionalNodePtr node) override;
    virtual int accept(RepeatedNodePtr node) override;
    virtual int accept(GroupedNodePtr node) override;
    virtual int accept(IdentifierNodePtr node) override;
    virtual int accept(LiteralNodePtr node) override;
    virtual int accept(EpsilonNodePtr node) override;
};

#endif