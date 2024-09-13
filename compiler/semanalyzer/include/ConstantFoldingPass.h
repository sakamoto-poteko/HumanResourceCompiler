#ifndef CONSTANTFOLDINGPASS_H
#define CONSTANTFOLDINGPASS_H

#include <string>

#include "ASTNode.h"
#include "ASTNodeAttribute.h"
#include "SemanticAnalysisPass.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

using namespace parser;

template <typename Func>
concept BinaryIntOperation = requires(Func func, int a, int b, int &out) {
    {
        func(a, b, out)
    } -> std::same_as<int>;
};

template <typename Func>
concept UnaryIntOperation = requires(Func func, int a, int &out) {
    {
        func(a, out)
    } -> std::same_as<int>;
};

class ConstantFoldingAttribute : public ASTNodeAttribute, public GetSetAttribute<ConstantFoldingAttribute> {
public:
    ConstantFoldingAttribute(int value)
        : _value(value)
    {
    }

    int get_value() const { return _value; }

    int get_attribute_id() { return ATTR_SEMANALYZER_CONST_FOLDING_VALUE; }

    std::string to_string() override;

private:
    int _value;
};

/**
 * @brief
 * Constant Folding: detect and evaluate expressions involve only constant values, replace the ASTNode.
 * - Algo: Preorder in expression nodes. If the child is const, attach ATTR_SEMANALYZER_CONST_FOLDING_VALUE.
 *         If all children has such value, evaluate then replace the node.
 * It also performs integer overflow check, div/mod 0 check, div/mul/mod 1 or 0 opt, add/sub 0 opt.
 */
class ConstantFoldingPass : public SemanticAnalysisPass {
public:
    ConstantFoldingPass(StringPtr filename, parser::CompilationUnitASTNodePtr root);
    ~ConstantFoldingPass();

    // For all visit, the return value of 0 indicate success.
    int visit(const IntegerASTNodePtr &node) override;
    int visit(const BooleanASTNodePtr &node) override;
    int visit(const VariableDeclarationASTNodePtr &node) override;
    int visit(const VariableAssignmentASTNodePtr &node) override;
    int visit(const VariableAccessASTNodePtr &node) override;
    int visit(const FloorBoxInitStatementASTNodePtr &node) override;
    int visit(const FloorAssignmentASTNodePtr &node) override;
    int visit(const FloorAccessASTNodePtr &node) override;
    int visit(const NegativeExpressionASTNodePtr &node) override;
    int visit(const NotExpressionASTNodePtr &node) override;
    int visit(const IncrementExpressionASTNodePtr &node) override;
    int visit(const DecrementExpressionASTNodePtr &node) override;
    int visit(const AddExpressionASTNodePtr &node) override;
    int visit(const SubExpressionASTNodePtr &node) override;
    int visit(const MulExpressionASTNodePtr &node) override;
    int visit(const DivExpressionASTNodePtr &node) override;
    int visit(const ModExpressionASTNodePtr &node) override;
    int visit(const EqualExpressionASTNodePtr &node) override;
    int visit(const NotEqualExpressionASTNodePtr &node) override;
    int visit(const GreaterThanExpressionASTNodePtr &node) override;
    int visit(const GreaterEqualExpressionASTNodePtr &node) override;
    int visit(const LessThanExpressionASTNodePtr &node) override;
    int visit(const LessEqualExpressionASTNodePtr &node) override;
    int visit(const AndExpressionASTNodePtr &node) override;
    int visit(const OrExpressionASTNodePtr &node) override;
    int visit(const InvocationExpressionASTNodePtr &node) override;
    int visit(const EmptyStatementASTNodePtr &node) override;
    int visit(const IfStatementASTNodePtr &node) override;
    int visit(const WhileStatementASTNodePtr &node) override;
    int visit(const ForStatementASTNodePtr &node) override;
    int visit(const ReturnStatementASTNodePtr &node) override;
    int visit(const BreakStatementASTNodePtr &node) override;
    int visit(const ContinueStatementASTNodePtr &node) override;
    int visit(const StatementBlockASTNodePtr &node) override;
    int visit(const SubprocDefinitionASTNodePtr &node) override;
    int visit(const FunctionDefinitionASTNodePtr &node) override;
    int visit(const CompilationUnitASTNodePtr &node) override;

    int run() override;

protected:
private:
    void attach_constant(const ASTNodePtr &node, int value);

    int fold_binary_expression(const AbstractBinaryExpressionASTNodePtr &node, BinaryIntOperation auto op_func);
    int fold_unary_expression(const AbstractUnaryExpressionASTNodePtr &node, UnaryIntOperation auto op_func);

    int check_integer_range(int value, const ASTNodePtr &node);
};

CLOSE_SEMANALYZER_NAMESPACE

#endif