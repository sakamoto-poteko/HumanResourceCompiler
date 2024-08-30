#ifndef ASTNODEVISITOR_H
#define ASTNODEVISITOR_H

#include "parser_global.h"

#include "ASTNodeForward.h"

OPEN_PARSER_NAMESPACE

class ASTNodeVisitor {
public:
    ASTNodeVisitor() = default;

    virtual ~ASTNodeVisitor() = default;

    virtual void visit(IdentifierNode *node) = 0;
    virtual void visit(IntegerLiteralNode *node) = 0;
    virtual void visit(BooleanLiteralNode *node) = 0;
    virtual void visit(BinaryOperatorNode *node) = 0;
    virtual void visit(VariableDeclarationNode *node) = 0;
    virtual void visit(VariableAssignmentNode *node) = 0;
    virtual void visit(FloorAssignmentNode *node) = 0;
    virtual void visit(BinaryExpressionNode *node) = 0;
    virtual void visit(IncrementExpressionNode *node) = 0;
    virtual void visit(DecrementExpressionNode *node) = 0;
    virtual void visit(FloorAccessNode *node) = 0;
    virtual void visit(ParenthesizedExpressionNode *node) = 0;
    virtual void visit(InvocationExpressionNode *node) = 0;
    virtual void visit(IfStatementNode *node) = 0;
    virtual void visit(WhileStatementNode *node) = 0;
    virtual void visit(ForStatementNode *node) = 0;
    virtual void visit(ReturnStatementNode *node) = 0;
    virtual void visit(FloorBoxInitStatementNode *node) = 0;
    virtual void visit(FloorMaxInitStatementNode *node) = 0;
    virtual void visit(EmptyStatementNode *node) = 0;
    virtual void visit(StatementBlockNode *node) = 0;
    virtual void visit(RoutineDefinitionCommonNode *node) = 0;
    virtual void visit(SubprocDefinitionNode *node) = 0;
    virtual void visit(FunctionDefinitionNode *node) = 0;
    virtual void visit(CompilationUnitNode *node) = 0;
};

CLOSE_PARSER_NAMESPACE

#endif