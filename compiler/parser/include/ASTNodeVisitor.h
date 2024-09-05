#ifndef ASTNODEVISITOR_H
#define ASTNODEVISITOR_H

#include "ASTNodeForward.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

class ASTNodeVisitor {
public:
    ASTNodeVisitor() = default;

    virtual ~ASTNodeVisitor() = default;

    virtual int visit(IntegerASTNodePtr node) = 0;
    virtual int visit(BooleanASTNodePtr node) = 0;
    virtual int visit(VariableDeclarationASTNodePtr node) = 0;
    virtual int visit(VariableAssignmentASTNodePtr node) = 0;
    virtual int visit(VariableAccessASTNodePtr node) = 0;
    virtual int visit(FloorBoxInitStatementASTNodePtr node) = 0;
    virtual int visit(FloorAssignmentASTNodePtr node) = 0;
    virtual int visit(FloorAccessASTNodePtr node) = 0;
    virtual int visit(NegativeExpressionASTNodePtr node) = 0;
    virtual int visit(NotExpressionASTNodePtr node) = 0;
    virtual int visit(IncrementExpressionASTNodePtr node) = 0;
    virtual int visit(DecrementExpressionASTNodePtr node) = 0;
    virtual int visit(AddExpressionASTNodePtr node) = 0;
    virtual int visit(SubExpressionASTNodePtr node) = 0;
    virtual int visit(MulExpressionASTNodePtr node) = 0;
    virtual int visit(DivExpressionASTNodePtr node) = 0;
    virtual int visit(ModExpressionASTNodePtr node) = 0;
    virtual int visit(EqualExpressionASTNodePtr node) = 0;
    virtual int visit(NotEqualExpressionASTNodePtr node) = 0;
    virtual int visit(GreaterThanExpressionASTNodePtr node) = 0;
    virtual int visit(GreaterEqualExpressionASTNodePtr node) = 0;
    virtual int visit(LessThanExpressionASTNodePtr node) = 0;
    virtual int visit(LessEqualExpressionASTNodePtr node) = 0;
    virtual int visit(AndExpressionASTNodePtr node) = 0;
    virtual int visit(OrExpressionASTNodePtr node) = 0;
    virtual int visit(InvocationExpressionASTNodePtr node) = 0;
    virtual int visit(EmptyStatementASTNodePtr node) = 0;
    virtual int visit(IfStatementASTNodePtr node) = 0;
    virtual int visit(WhileStatementASTNodePtr node) = 0;
    virtual int visit(ForStatementASTNodePtr node) = 0;
    virtual int visit(ReturnStatementASTNodePtr node) = 0;
    virtual int visit(StatementBlockASTNodePtr node) = 0;
    virtual int visit(SubprocDefinitionASTNodePtr node) = 0;
    virtual int visit(FunctionDefinitionASTNodePtr node) = 0;
    virtual int visit(CompilationUnitASTNodePtr node) = 0;

private:
};

CLOSE_PARSER_NAMESPACE

#endif