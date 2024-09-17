#ifndef ASTNODEVISITOR_H
#define ASTNODEVISITOR_H

#include "ASTNode.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

class ASTNodeVisitor {
public:
    ASTNodeVisitor() = default;

    virtual ~ASTNodeVisitor() = default;

    [[nodiscard]] virtual int visit(const IntegerASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const BooleanASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const VariableDeclarationASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const VariableAssignmentASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const VariableAccessASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const FloorBoxInitStatementASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const FloorAssignmentASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const FloorAccessASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const NegativeExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const NotExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const IncrementExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const DecrementExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const AddExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const SubExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const MulExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const DivExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const ModExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const EqualExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const NotEqualExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const GreaterThanExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const GreaterEqualExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const LessThanExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const LessEqualExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const AndExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const OrExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const InvocationExpressionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const EmptyStatementASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const IfStatementASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const WhileStatementASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const ForStatementASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const ReturnStatementASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const BreakStatementASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const ContinueStatementASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const StatementBlockASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const SubprocDefinitionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const FunctionDefinitionASTNodePtr &node) = 0;
    [[nodiscard]] virtual int visit(const CompilationUnitASTNodePtr &node) = 0;

private:
};

CLOSE_PARSER_NAMESPACE

#endif