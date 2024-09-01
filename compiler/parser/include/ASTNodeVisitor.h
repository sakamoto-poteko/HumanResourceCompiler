#ifndef ASTNODEVISITOR_H
#define ASTNODEVISITOR_H

#include "parser_global.h"

#include "ASTNodeForward.h"

OPEN_PARSER_NAMESPACE

class ASTNodeVisitor {
public:
    ASTNodeVisitor() = default;
    virtual ~ASTNodeVisitor() = default;

    virtual void visit(IdentifierNodePtr node) = 0;
    virtual void visit(IntegerLiteralNodePtr node) = 0;
    virtual void visit(BooleanLiteralNodePtr node) = 0;
    virtual void visit(BinaryOperatorNodePtr node) = 0;
    virtual void visit(VariableDeclarationNodePtr node) = 0;
    virtual void visit(VariableAssignmentNodePtr node) = 0;
    virtual void visit(FloorAssignmentNodePtr node) = 0;
    virtual void visit(BinaryExpressionNodePtr node) = 0;
    virtual void visit(IncrementExpressionNodePtr node) = 0;
    virtual void visit(DecrementExpressionNodePtr node) = 0;
    virtual void visit(FloorAccessNodePtr node) = 0;
    virtual void visit(ParenthesizedExpressionNodePtr node) = 0;
    virtual void visit(InvocationExpressionNodePtr node) = 0;
    virtual void visit(IfStatementNodePtr node) = 0;
    virtual void visit(WhileStatementNodePtr node) = 0;
    virtual void visit(ForStatementNodePtr node) = 0;
    virtual void visit(ReturnStatementNodePtr node) = 0;
    virtual void visit(FloorBoxInitStatementNodePtr node) = 0;
    virtual void visit(FloorMaxInitStatementNodePtr node) = 0;
    virtual void visit(EmptyStatementNodePtr node) = 0;
    virtual void visit(StatementBlockNodePtr node) = 0;
    virtual void visit(VariableDeclarationStatementNodePtr node) = 0;
    virtual void visit(VariableAssignmentStatementNodePtr node) = 0;
    virtual void visit(FloorAssignmentStatementNodePtr node) = 0;
    virtual void visit(SubprocDefinitionNodePtr node) = 0;
    virtual void visit(FunctionDefinitionNodePtr node) = 0;
    virtual void visit(ImportDirectiveNodePtr node) = 0;
    virtual void visit(CompilationUnitNodePtr node) = 0;
};

CLOSE_PARSER_NAMESPACE

#endif