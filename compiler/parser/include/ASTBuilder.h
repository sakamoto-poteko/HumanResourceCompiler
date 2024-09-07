#ifndef AST_BUILDER_H
#define AST_BUILDER_H

#include <cassert>

#include <stack>

#include "ASTNode.h"
#include "ASTNodeForward.h"
#include "ParseTreeNode.h"
#include "ParseTreeNodeForward.h"
#include "ParseTreeNodeVisitor.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

class ASTBuilder : public ParseTreeNodeVisitor {
public:
    explicit ASTBuilder(CompilationUnitPTNodePtr root);
    ~ASTBuilder() override = default;

    bool build(CompilationUnitASTNodePtr &result);

    void visit(IdentifierPTNodePtr node) override;
    void visit(IntegerLiteralPTNodePtr node) override;
    void visit(BooleanLiteralPTNodePtr node) override;
    void visit(BinaryOperatorPTNodePtr node) override;
    void visit(VariableDeclarationPTNodePtr node) override;
    void visit(VariableAssignmentPTNodePtr node) override;
    void visit(FloorAssignmentPTNodePtr node) override;
    void visit(BinaryExpressionPTNodePtr node) override;
    void visit(NegativeExpressionPTNodePtr node) override;
    void visit(PositiveExpressionPTNodePtr node) override;
    void visit(NotExpressionPTNodePtr node) override;
    void visit(IncrementExpressionPTNodePtr node) override;
    void visit(DecrementExpressionPTNodePtr node) override;
    void visit(FloorAccessPTNodePtr node) override;
    void visit(ParenthesizedExpressionPTNodePtr node) override;
    void visit(InvocationExpressionPTNodePtr node) override;
    void visit(IfStatementPTNodePtr node) override;
    void visit(WhileStatementPTNodePtr node) override;
    void visit(ForStatementPTNodePtr node) override;
    void visit(ReturnStatementPTNodePtr node) override;
    void visit(BreakContinueStatementPTNodePtr node) override;
    void visit(FloorBoxInitStatementPTNodePtr node) override;
    void visit(FloorMaxInitStatementPTNodePtr node) override;
    void visit(EmptyStatementPTNodePtr node) override;
    void visit(StatementBlockPTNodePtr node) override;
    void visit(VariableDeclarationStatementPTNodePtr node) override;
    void visit(VariableAssignmentStatementPTNodePtr node) override;
    void visit(FloorAssignmentStatementPTNodePtr node) override;
    void visit(InvocationStatementPTNodePtr node) override;
    void visit(SubprocDefinitionPTNodePtr node) override;
    void visit(FunctionDefinitionPTNodePtr node) override;
    void visit(ImportDirectivePTNodePtr node) override;
    void visit(CompilationUnitPTNodePtr node) override;

protected:
    CompilationUnitPTNodePtr _root;
    std::stack<ASTNodePtr> _result_stack;

    template <typename ASTNodePtrT, typename ParseTreeNodePtrT>
        requires convertible_to_ASTNodePtr<ASTNodePtrT> && convertible_to_ParseTreeNodePtr<ParseTreeNodePtrT>
    inline ASTNodePtrT visit_and_cast(ParseTreeNodePtrT target)
    {
        using ElementType = typename ASTNodePtrT::element_type;

        if (target) {
            target->accept(this);
            auto top = _result_stack.top();
            ASTNodePtrT result = std::dynamic_pointer_cast<ElementType>(top);
            assert(result);
            _result_stack.pop();
            return result;
        } else {
            return ASTNodePtrT();
        }
    }

    static IntegerASTNodePtr make_literal_node(const IntegerLiteralPTNodePtr &node)
    {
        IntegerASTNodePtr val_expr = std::make_shared<IntegerASTNode>(node->lineno(), node->colno(), node->lineno(), int(node->lineno() + node->get_token()->width()), node->get_value());
        return val_expr;
    }

private:
};

CLOSE_PARSER_NAMESPACE

#endif
