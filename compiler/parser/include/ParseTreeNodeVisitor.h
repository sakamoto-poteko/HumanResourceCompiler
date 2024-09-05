#ifndef PARSE_TREE_NODE_VISITOR_H
#define PARSE_TREE_NODE_VISITOR_H

#include "parser_global.h"

#include "ParseTreeNodeForward.h"

OPEN_PARSER_NAMESPACE

class ParseTreeNodeVisitor {
public:
    ParseTreeNodeVisitor() = default;
    virtual ~ParseTreeNodeVisitor() = default;

    virtual void visit(IdentifierPTNodePtr node) = 0;
    virtual void visit(IntegerLiteralPTNodePtr node) = 0;
    virtual void visit(BooleanLiteralPTNodePtr node) = 0;
    virtual void visit(BinaryOperatorPTNodePtr node) = 0;
    virtual void visit(VariableDeclarationPTNodePtr node) = 0;
    virtual void visit(VariableAssignmentPTNodePtr node) = 0;
    virtual void visit(FloorAssignmentPTNodePtr node) = 0;
    virtual void visit(BinaryExpressionPTNodePtr node) = 0;
    virtual void visit(NegativeExpressionPTNodePtr node) = 0;
    virtual void visit(PositiveExpressionPTNodePtr node) = 0;
    virtual void visit(NotExpressionPTNodePtr node) = 0;
    virtual void visit(IncrementExpressionPTNodePtr node) = 0;
    virtual void visit(DecrementExpressionPTNodePtr node) = 0;
    virtual void visit(FloorAccessPTNodePtr node) = 0;
    virtual void visit(ParenthesizedExpressionPTNodePtr node) = 0;
    virtual void visit(InvocationExpressionPTNodePtr node) = 0;
    virtual void visit(IfStatementPTNodePtr node) = 0;
    virtual void visit(WhileStatementPTNodePtr node) = 0;
    virtual void visit(ForStatementPTNodePtr node) = 0;
    virtual void visit(ReturnStatementPTNodePtr node) = 0;
    virtual void visit(BreakContinueStatementPTNodePtr node) = 0;
    virtual void visit(FloorBoxInitStatementPTNodePtr node) = 0;
    virtual void visit(FloorMaxInitStatementPTNodePtr node) = 0;
    virtual void visit(EmptyStatementPTNodePtr node) = 0;
    virtual void visit(StatementBlockPTNodePtr node) = 0;
    virtual void visit(VariableDeclarationStatementPTNodePtr node) = 0;
    virtual void visit(VariableAssignmentStatementPTNodePtr node) = 0;
    virtual void visit(FloorAssignmentStatementPTNodePtr node) = 0;
    virtual void visit(InvocationStatementPTNodePtr node) = 0;
    virtual void visit(SubprocDefinitionPTNodePtr node) = 0;
    virtual void visit(FunctionDefinitionPTNodePtr node) = 0;
    virtual void visit(ImportDirectivePTNodePtr node) = 0;
    virtual void visit(CompilationUnitPTNodePtr node) = 0;
};

CLOSE_PARSER_NAMESPACE

#endif