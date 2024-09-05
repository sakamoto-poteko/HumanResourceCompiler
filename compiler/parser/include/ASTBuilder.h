#ifndef ASTBUILDER_H
#define ASTBUILDER_H

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
    ASTBuilder(const CompilationUnitPTNodePtr &root);
    virtual ~ASTBuilder();

    bool build(CompilationUnitASTNodePtr &result);

    virtual void visit(IdentifierPTNodePtr node) override;
    virtual void visit(IntegerLiteralPTNodePtr node) override;
    virtual void visit(BooleanLiteralPTNodePtr node) override;
    virtual void visit(BinaryOperatorPTNodePtr node) override;
    virtual void visit(VariableDeclarationPTNodePtr node) override;
    virtual void visit(VariableAssignmentPTNodePtr node) override;
    virtual void visit(FloorAssignmentPTNodePtr node) override;
    virtual void visit(BinaryExpressionPTNodePtr node) override;
    virtual void visit(NegativeExpressionPTNodePtr node) override;
    virtual void visit(PositiveExpressionPTNodePtr node) override;
    virtual void visit(NotExpressionPTNodePtr node) override;
    virtual void visit(IncrementExpressionPTNodePtr node) override;
    virtual void visit(DecrementExpressionPTNodePtr node) override;
    virtual void visit(FloorAccessPTNodePtr node) override;
    virtual void visit(ParenthesizedExpressionPTNodePtr node) override;
    virtual void visit(InvocationExpressionPTNodePtr node) override;
    virtual void visit(IfStatementPTNodePtr node) override;
    virtual void visit(WhileStatementPTNodePtr node) override;
    virtual void visit(ForStatementPTNodePtr node) override;
    virtual void visit(ReturnStatementPTNodePtr node) override;
    virtual void visit(FloorBoxInitStatementPTNodePtr node) override;
    virtual void visit(FloorMaxInitStatementPTNodePtr node) override;
    virtual void visit(EmptyStatementPTNodePtr node) override;
    virtual void visit(StatementBlockPTNodePtr node) override;
    virtual void visit(VariableDeclarationStatementPTNodePtr node) override;
    virtual void visit(VariableAssignmentStatementPTNodePtr node) override;
    virtual void visit(FloorAssignmentStatementPTNodePtr node) override;
    virtual void visit(InvocationStatementPTNodePtr node) override;
    virtual void visit(SubprocDefinitionPTNodePtr node) override;
    virtual void visit(FunctionDefinitionPTNodePtr node) override;
    virtual void visit(ImportDirectivePTNodePtr node) override;
    virtual void visit(CompilationUnitPTNodePtr node) override;

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

    IntegerASTNodePtr make_literal_node(IntegerLiteralPTNodePtr node)
    {
        IntegerASTNodePtr val_expr = std::make_shared<IntegerASTNode>(node->lineno(), node->colno(), node->lineno(), node->lineno() + node->get_token()->width(), node->get_value());
        return val_expr;
    }

private:
};

CLOSE_PARSER_NAMESPACE

#endif
