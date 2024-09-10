#include <cstdint>

#include <memory>

#include <boost/format.hpp>

#include "ASTNode.h"
#include "ASTNodeForward.h"
#include "ConstantFoldingPass.h"
#include "ErrorManager.h"
#include "ErrorMessage.h"
#include "SemanticAnalysisErrors.h"
#include "SemanticAnalysisPass.h"
#include "SemanticConstants.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

#define BEGIN_VISIT()       \
    enter_node(node);       \
    int result = 0, rc = 0; \
    UNUSED(rc)

#define END_VISIT() \
    leave_node();   \
    return result

#define SET_RESULT_RC() \
    if (rc != 0) {      \
        result = rc;    \
        END_VISIT();    \
    }

ConstantFoldingPass::ConstantFoldingPass(StringPtr filename, parser::CompilationUnitASTNodePtr root)
    : SemanticAnalysisPass(std::move(filename), std::move(root))
{
}

ConstantFoldingPass::~ConstantFoldingPass()
{
}

int ConstantFoldingPass::visit(IntegerASTNodePtr node)
{
    BEGIN_VISIT();
    rc = check_integer_range(node->get_value(), node);
    SET_RESULT_RC();

    attach_constant(node, node->get_value());
    END_VISIT();
}

int ConstantFoldingPass::visit(BooleanASTNodePtr node)
{
    BEGIN_VISIT();
    attach_constant(node, node->get_value() ? 1 : 0);
    END_VISIT();
}

int ConstantFoldingPass::visit(VariableDeclarationASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(VariableAssignmentASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(VariableAccessASTNodePtr node)
{
    // NOTE: This is not something that can be folded, but it's worth a shot for propagation
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(FloorBoxInitStatementASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(FloorAssignmentASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(FloorAccessASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(NegativeExpressionASTNodePtr node)
{
    BEGIN_VISIT();
    rc = fold_unary_expression(node, [](int a, int &out) { out = -a; return 0; });
    SET_RESULT_RC();
    END_VISIT();
}

int ConstantFoldingPass::visit(NotExpressionASTNodePtr node)
{
    BEGIN_VISIT();
    rc = fold_unary_expression(node, [](int a, int &out) {
        if (a) {
            // a is true
            out = 0;
        } else {
            out = 1;
        }
        return 0;
    });
    SET_RESULT_RC();
    END_VISIT();
}

int ConstantFoldingPass::visit(IncrementExpressionASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(DecrementExpressionASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(AddExpressionASTNodePtr node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a + b; return 0; });
    SET_RESULT_RC();

    // visit 2: add 0 and 0 add opt
    auto left = node->get_left();
    auto right = node->get_right();
    ASTNodeAttributePtr lattr, rattr;
    left->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, lattr);
    right->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, rattr);
    do {
        if (rattr && !lattr) {
            int value = std::static_pointer_cast<ConstantFoldingAttribute>(rattr)->get_value();
            if (value == 0) {
                request_to_replace_self(left);
                break;
            }
        }

        if (lattr && !rattr) {
            int value = std::static_pointer_cast<ConstantFoldingAttribute>(lattr)->get_value();
            if (value == 0) {
                request_to_replace_self(right);
                break;
            }
        }
    } while (false);

    END_VISIT();
}

int ConstantFoldingPass::visit(SubExpressionASTNodePtr node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a - b; return 0; });
    SET_RESULT_RC();

    // visit 2: sub 0 and 0 sub opt
    auto left = node->get_left();
    auto right = node->get_right();
    ASTNodeAttributePtr lattr, rattr;
    left->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, lattr);
    right->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, rattr);
    do {
        if (rattr && !lattr) {
            int value = std::static_pointer_cast<ConstantFoldingAttribute>(rattr)->get_value();
            if (value == 0) {
                request_to_replace_self(left);
                break;
            }
        }

        if (lattr && !rattr) {
            int value = std::static_pointer_cast<ConstantFoldingAttribute>(lattr)->get_value();
            if (value == 0) {
                auto negative = std::make_shared<NegativeExpressionASTNode>(node->lineno(), node->colno(), node->last_lineno(), node->last_colno(), right);
                negative->copy_attributes_from(right);
                request_to_replace_self(negative);
                break;
            }
        }
    } while (false);

    END_VISIT();
}

int ConstantFoldingPass::visit(MulExpressionASTNodePtr node)
{
    BEGIN_VISIT();

    // visit 1: fold
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a * b; return 0; });
    SET_RESULT_RC();

    // visit 2: mul 0 and mul 1 opt
    auto left = node->get_left();
    auto right = node->get_right();
    ASTNodeAttributePtr lattr, rattr;
    left->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, lattr);
    right->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, rattr);
    do {
        if (lattr && !rattr) {
            int value = std::static_pointer_cast<ConstantFoldingAttribute>(lattr)->get_value();
            if (value == 0) {
                auto zero = std::make_shared<IntegerASTNode>(node->lineno(), node->colno(), node->last_lineno(), node->last_colno(), 0);
                zero->copy_attributes_from(node);
                attach_constant(zero, 0);
                request_to_replace_self(zero);
                break;
            } else if (value == 1) {
                request_to_replace_self(right);
                break;
            }
        }

        if (rattr && !lattr) {
            int value = std::static_pointer_cast<ConstantFoldingAttribute>(rattr)->get_value();
            if (value == 0) {
                auto zero = std::make_shared<IntegerASTNode>(node->lineno(), node->colno(), node->last_lineno(), node->last_colno(), 0);
                zero->copy_attributes_from(node);
                attach_constant(zero, 0);
                request_to_replace_self(zero);
                break;
            } else if (value == 1) {
                request_to_replace_self(left);
                break;
            }
        }
    } while (false);

    END_VISIT();
}

int ConstantFoldingPass::visit(DivExpressionASTNodePtr node)
{
    BEGIN_VISIT();

    // visit 1: fold
    rc = fold_binary_expression(node, [&](int a, int b, int &out) -> int {
        if (b == 0) {
            auto fmt = boost::format(
                           "Error: Division by zero detected. The expression '%1% / %2%' results in undefined behavior.")
                % a % b;
            ErrorManager::instance().report(
                E_SEMA_DIV_MOD_0,
                ErrorSeverity::Error,
                ErrorLocation(_filename, node->lineno(), node->colno(), 0),
                fmt.str());
            return E_SEMA_DIV_MOD_0;
        } else {
            out = a / b;
            return 0;
        }
    });
    SET_RESULT_RC();

    // visit 2: div 0 check, div 1 opt, 0 div opt
    auto right = node->get_right();
    auto left = node->get_left();
    ASTNodeAttributePtr lattr, rattr;
    right->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, rattr);
    left->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, lattr);

    do {
        if (rattr && !lattr) {
            int value = std::static_pointer_cast<ConstantFoldingAttribute>(rattr)->get_value();
            // div 0
            if (value == 0) {
                ErrorManager::instance().report(
                    E_SEMA_DIV_MOD_0,
                    ErrorSeverity::Error,
                    ErrorLocation(_filename, node->lineno(), node->colno(), 0),
                    "Error: Division by zero detected. The expression results in undefined behavior.");
                result = E_SEMA_DIV_MOD_0;
                break;
            } else if (value == 1) { // div 1
                request_to_replace_self(left);
                break;
            }
        }

        if (lattr && !rattr) {
            int value = std::static_pointer_cast<ConstantFoldingAttribute>(lattr)->get_value();
            if (value == 0) { // 0 div
                auto zero = std::make_shared<IntegerASTNode>(node->lineno(), node->colno(), node->last_lineno(), node->last_colno(), 0);
                zero->copy_attributes_from(node);
                attach_constant(zero, 0);
                request_to_replace_self(zero);
                break;
            }
        }

    } while (false);

    END_VISIT();
}

int ConstantFoldingPass::visit(ModExpressionASTNodePtr node)
{
    BEGIN_VISIT();

    // visit 1: fold
    rc = fold_binary_expression(node, [&](int a, int b, int &out) -> int {
        if (b == 0) {
            auto fmt = boost::format(
                           "Error: Mod by zero detected. The expression '%1% / %2%' results in undefined behavior.")
                % a % b;
            ErrorManager::instance().report(
                E_SEMA_DIV_MOD_0,
                ErrorSeverity::Error,
                ErrorLocation(_filename, node->lineno(), node->colno(), 0),
                fmt.str());
            return E_SEMA_DIV_MOD_0;
        } else {
            out = a % b;
            return 0;
        }
    });
    SET_RESULT_RC();

    // visit 2: mod 0 check, mod 1 opt, 0 mod opt
    auto left = node->get_left();
    auto right = node->get_right();
    ASTNodeAttributePtr lattr, rattr;
    right->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, rattr);
    left->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, lattr);

    do {
        if (rattr && !lattr) {
            int value = std::static_pointer_cast<ConstantFoldingAttribute>(rattr)->get_value();
            // mod 0
            if (value == 0) {
                ErrorManager::instance().report(
                    E_SEMA_DIV_MOD_0,
                    ErrorSeverity::Error,
                    ErrorLocation(_filename, node->lineno(), node->colno(), 0),
                    "Error: Mod by zero detected. The expression results in undefined behavior.");
                result = E_SEMA_DIV_MOD_0;
                break;
            } else if (value == 1) { // mod 1
                request_to_replace_self(left);
                break;
            }
        }

        if (lattr && !rattr) {
            int value = std::static_pointer_cast<ConstantFoldingAttribute>(lattr)->get_value();
            if (value == 0) { // 0 mod
                auto zero = std::make_shared<IntegerASTNode>(node->lineno(), node->colno(), node->last_lineno(), node->last_colno(), 0);
                zero->copy_attributes_from(node);
                attach_constant(zero, 0);
                request_to_replace_self(zero);
                break;
            }
        }
    } while (false);

    END_VISIT();
}

int ConstantFoldingPass::visit(EqualExpressionASTNodePtr node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a == b ? 1 : 0; return 0; });
    SET_RESULT_RC();
    END_VISIT();
}

int ConstantFoldingPass::visit(NotEqualExpressionASTNodePtr node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a != b ? 1 : 0; return 0; });
    SET_RESULT_RC();
    END_VISIT();
}

int ConstantFoldingPass::visit(GreaterThanExpressionASTNodePtr node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a > b ? 1 : 0; return 0; });
    SET_RESULT_RC();
    END_VISIT();
}

int ConstantFoldingPass::visit(GreaterEqualExpressionASTNodePtr node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a >= b ? 1 : 0; return 0; });
    SET_RESULT_RC();
    END_VISIT();
}

int ConstantFoldingPass::visit(LessThanExpressionASTNodePtr node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a < b ? 1 : 0; return 0; });
    SET_RESULT_RC();
    END_VISIT();
}

int ConstantFoldingPass::visit(LessEqualExpressionASTNodePtr node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a <= b ? 1 : 0;return 0; });
    SET_RESULT_RC();
    END_VISIT();
}

int ConstantFoldingPass::visit(AndExpressionASTNodePtr node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a && b ? 1 : 0; return 0; });
    SET_RESULT_RC();
    END_VISIT();
}

int ConstantFoldingPass::visit(OrExpressionASTNodePtr node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a || b ? 1 : 0; return 0; });
    SET_RESULT_RC();
    END_VISIT();
}

int ConstantFoldingPass::visit(InvocationExpressionASTNodePtr node)
{
    // There's no folding for invocation expression, but we can dive into it's expression.
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(EmptyStatementASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(IfStatementASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(WhileStatementASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(ForStatementASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(ReturnStatementASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(BreakStatementASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(ContinueStatementASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(StatementBlockASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(SubprocDefinitionASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(FunctionDefinitionASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(CompilationUnitASTNodePtr node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::run()
{
    return visit(_root);
}

void ConstantFoldingPass::attach_constant(const ASTNodePtr &node, int value)
{
    auto attr = std::make_shared<ConstantFoldingAttribute>(value);
    node->set_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, attr);
}

int ConstantFoldingPass::fold_binary_expression(const AbstractBinaryExpressionASTNodePtr &node, BinaryIntOperation auto op_func)
{
    int result = 0, rc = 0;

    rc = traverse(node->get_left());
    SET_RESULT_RC();

    rc = traverse(node->get_right());
    SET_RESULT_RC();

    ASTNodeAttributePtr left_const_fld;
    ASTNodeAttributePtr right_const_fld;
    bool left_const = node->get_left()->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, left_const_fld);
    bool right_const = node->get_right()->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, right_const_fld);

    if (left_const && right_const) {
        int left = std::static_pointer_cast<ConstantFoldingAttribute>(left_const_fld)->get_value();
        int right = std::static_pointer_cast<ConstantFoldingAttribute>(right_const_fld)->get_value();
        int val;
        rc = op_func(left, right, val);
        SET_RESULT_RC();

        rc = check_integer_range(val, node);
        SET_RESULT_RC();

        attach_constant(node, val);

        auto const_node = std::make_shared<IntegerASTNode>(node->lineno(), node->colno(), node->last_lineno(), node->last_colno(), val);
        const_node->copy_attributes_from(node);
        attach_constant(const_node, val);

        request_to_replace_self(const_node);
    }

    return result;
}

int ConstantFoldingPass::fold_unary_expression(const AbstractUnaryExpressionASTNodePtr &node, UnaryIntOperation auto op_func)
{
    int result = 0, rc = 0;

    rc = traverse(node->get_operand());
    SET_RESULT_RC();

    ASTNodeAttributePtr const_fld;
    bool is_const = node->get_operand()->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, const_fld);

    if (is_const) {
        int operand = std::static_pointer_cast<ConstantFoldingAttribute>(const_fld)->get_value();
        int val;
        rc = op_func(operand, val);
        SET_RESULT_RC();

        rc = check_integer_range(val, node);
        SET_RESULT_RC();

        attach_constant(node, val);
    }

    return result;
}

int ConstantFoldingPass::check_integer_range(int value, const ASTNodePtr &node)
{
    if (value > MAX_INTEGER_VALUE || value < MIN_INTEGER_VALUE) {
        auto fmt = boost::format(
                       "Integer overflow detected: The value '%1%' exceeds the allowed range [-999, 999]. "
                       "Ensure that the value assigned or computed stays within the valid limits.")
            % value;
        ErrorManager::instance().report(
            E_SEMA_INT_OVERFLOW,
            ErrorSeverity::Error,
            ErrorLocation(_filename, node->lineno(), node->colno(), 0),
            fmt.str());
        return E_SEMA_INT_OVERFLOW;
    }
    return 0;
}

std::string ConstantFoldingAttribute::to_string()
{
    return "const: " + std::to_string(_value);
}

int ConstantFoldingAttribute::get_type()
{
    return ATTR_SEMANALYZER_CONST_FOLDING_VALUE;
}

CLOSE_SEMANALYZER_NAMESPACE

// end
