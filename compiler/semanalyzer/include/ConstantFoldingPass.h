#ifndef CONSTANTFOLDINGPASS_H
#define CONSTANTFOLDINGPASS_H

#include "ASTNode.h"
#include "ASTNodeForward.h"
#include "SemanticAnalysisPass.h"
#include "semanalyzer_global.h"
#include <cstdint>
#include <stack>
#include <string>

OPEN_SEMANALYZER_NAMESPACE

using namespace parser;

template <typename Func>
concept BinaryIntOperation = requires(Func func, int a, int b, int &out) {
    { func(a, b, out) } -> std::same_as<int>;
};

template <typename Func>
concept UnaryIntOperation = requires(Func func, int a, int &out) {
    { func(a, out) } -> std::same_as<int>;
};

class ConstantFoldingAttribute : public ASTNodeAttribute {
public:
    ConstantFoldingAttribute(int value)
        : _value(value)
    {
    }

    int get_value() const { return _value; }

    int get_type() override;

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
    int visit(IntegerASTNodePtr node) override;
    int visit(BooleanASTNodePtr node) override;
    int visit(VariableDeclarationASTNodePtr node) override;
    int visit(VariableAssignmentASTNodePtr node) override;
    int visit(VariableAccessASTNodePtr node) override;
    int visit(FloorBoxInitStatementASTNodePtr node) override;
    int visit(FloorAssignmentASTNodePtr node) override;
    int visit(FloorAccessASTNodePtr node) override;
    int visit(NegativeExpressionASTNodePtr node) override;
    int visit(NotExpressionASTNodePtr node) override;
    int visit(IncrementExpressionASTNodePtr node) override;
    int visit(DecrementExpressionASTNodePtr node) override;
    int visit(AddExpressionASTNodePtr node) override;
    int visit(SubExpressionASTNodePtr node) override;
    int visit(MulExpressionASTNodePtr node) override;
    int visit(DivExpressionASTNodePtr node) override;
    int visit(ModExpressionASTNodePtr node) override;
    int visit(EqualExpressionASTNodePtr node) override;
    int visit(NotEqualExpressionASTNodePtr node) override;
    int visit(GreaterThanExpressionASTNodePtr node) override;
    int visit(GreaterEqualExpressionASTNodePtr node) override;
    int visit(LessThanExpressionASTNodePtr node) override;
    int visit(LessEqualExpressionASTNodePtr node) override;
    int visit(AndExpressionASTNodePtr node) override;
    int visit(OrExpressionASTNodePtr node) override;
    int visit(InvocationExpressionASTNodePtr node) override;
    int visit(EmptyStatementASTNodePtr node) override;
    int visit(IfStatementASTNodePtr node) override;
    int visit(WhileStatementASTNodePtr node) override;
    int visit(ForStatementASTNodePtr node) override;
    int visit(ReturnStatementASTNodePtr node) override;
    int visit(BreakStatementASTNodePtr node) override;
    int visit(ContinueStatementASTNodePtr node) override;
    int visit(StatementBlockASTNodePtr node) override;
    int visit(SubprocDefinitionASTNodePtr node) override;
    int visit(FunctionDefinitionASTNodePtr node) override;
    int visit(CompilationUnitASTNodePtr node) override;

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