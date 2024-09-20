#include <memory>

#include <boost/format.hpp>

#include "ASTNode.h"
#include "ASTNodeForward.h"
#include "ConstantFoldingPass.h"
#include "ErrorManager.h"
#include "ErrorMessage.h"
#include "SemanticAnalysisErrors.h"
#include "SemanticAnalysisPass.h"
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

#define SET_RESULT_RC_AND_RETURN_IN_VISIT() \
    if (rc != 0) {                          \
        result = rc;                        \
        END_VISIT();                        \
    }

#define RETURN_IF_FAIL() \
    if (rc != 0) {       \
        return rc;       \
    }

ConstantFoldingPass::ConstantFoldingPass(StringPtr filename, parser::CompilationUnitASTNodePtr root)
    : SemanticAnalysisPass(std::move(filename), std::move(root))
{
}

ConstantFoldingPass::~ConstantFoldingPass()
{
}

int ConstantFoldingPass::visit(const IntegerASTNodePtr &node)
{
    BEGIN_VISIT();
    rc = check_integer_range(node->get_value(), node);
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    attach_constant(node, node->get_value(), node->get_is_char());
    END_VISIT();
}

int ConstantFoldingPass::visit(const BooleanASTNodePtr &node)
{
    BEGIN_VISIT();
    attach_constant(node, node->get_value() ? 1 : 0, false);
    END_VISIT();
}

int ConstantFoldingPass::visit(const VariableDeclarationASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const VariableAssignmentASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const VariableAccessASTNodePtr &node)
{
    // NOTE: This is not something that can be folded, but it's worth a shot for propagation
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const FloorBoxInitStatementASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const FloorAssignmentASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const FloorAccessASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const NegativeExpressionASTNodePtr &node)
{
    BEGIN_VISIT();
    rc = fold_unary_expression(node, [](int a, int &out) { out = -a; return 0; });
    SET_RESULT_RC_AND_RETURN_IN_VISIT();
    END_VISIT();
}

int ConstantFoldingPass::visit(const NotExpressionASTNodePtr &node)
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
    SET_RESULT_RC_AND_RETURN_IN_VISIT();
    END_VISIT();
}

int ConstantFoldingPass::visit(const IncrementExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const DecrementExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const AddExpressionASTNodePtr &node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a + b; return 0; });
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

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

int ConstantFoldingPass::visit(const SubExpressionASTNodePtr &node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a - b; return 0; });
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

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

int ConstantFoldingPass::visit(const MulExpressionASTNodePtr &node)
{
    BEGIN_VISIT();

    // visit 1: fold
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a * b; return 0; });
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    // visit 2: mul 0 and mul 1 opt
    auto left = node->get_left();
    auto right = node->get_right();
    ASTNodeAttributePtr lattr, rattr;
    left->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, lattr);
    right->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, rattr);
    do {
        if (lattr && !rattr) {
            int lvalue = std::static_pointer_cast<ConstantFoldingAttribute>(lattr)->get_value();
            if (lvalue == 0) {
                // 0 * 'A' = 0
                auto zero = std::make_shared<IntegerASTNode>(node->lineno(), node->colno(), node->last_lineno(), node->last_colno(), 0, false);
                zero->copy_attributes_from(node);
                attach_constant(zero, 0, false);
                request_to_replace_self(zero);
                break;
            } else if (lvalue == 1) {
                request_to_replace_self(right);
                break;
            }
        }

        if (rattr && !lattr) {
            int rvalue = std::static_pointer_cast<ConstantFoldingAttribute>(rattr)->get_value();
            if (rvalue == 0) {
                auto zero = std::make_shared<IntegerASTNode>(node->lineno(), node->colno(), node->last_lineno(), node->last_colno(), 0, false);
                zero->copy_attributes_from(node);
                attach_constant(zero, 0, false);
                request_to_replace_self(zero);
                break;
            } else if (rvalue == 1) {
                request_to_replace_self(left);
                break;
            }
        }
    } while (false);

    END_VISIT();
}

int ConstantFoldingPass::visit(const DivExpressionASTNodePtr &node)
{
    BEGIN_VISIT();

    // visit 1: fold
    rc = fold_binary_expression(node, [&](int a, int b, int &out) -> int {
        if (b == 0) {
            auto fmt = boost::format(
                           "Division by zero detected. The expression '%1% / %2%' results in undefined behavior.")
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
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

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
                    "Division by zero detected. The expression results in undefined behavior.");
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
                auto zero = std::make_shared<IntegerASTNode>(node->lineno(), node->colno(), node->last_lineno(), node->last_colno(), 0, false);
                zero->copy_attributes_from(node);
                attach_constant(zero, 0, false);
                request_to_replace_self(zero);
                break;
            }
        }

    } while (false);

    END_VISIT();
}

int ConstantFoldingPass::visit(const ModExpressionASTNodePtr &node)
{
    BEGIN_VISIT();

    // visit 1: fold
    rc = fold_binary_expression(node, [&](int a, int b, int &out) -> int {
        if (b == 0) {
            auto fmt = boost::format(
                           "Mod by zero detected. The expression '%1% / %2%' results in undefined behavior.")
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
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

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
                    "Mod by zero detected. The expression results in undefined behavior.");
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
                auto zero = std::make_shared<IntegerASTNode>(node->lineno(), node->colno(), node->last_lineno(), node->last_colno(), 0, false);
                zero->copy_attributes_from(node);
                attach_constant(zero, 0, false);
                request_to_replace_self(zero);
                break;
            }
        }
    } while (false);

    END_VISIT();
}

int ConstantFoldingPass::visit(const EqualExpressionASTNodePtr &node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a == b ? 1 : 0; return 0; });
    SET_RESULT_RC_AND_RETURN_IN_VISIT();
    END_VISIT();
}

int ConstantFoldingPass::visit(const NotEqualExpressionASTNodePtr &node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a != b ? 1 : 0; return 0; });
    SET_RESULT_RC_AND_RETURN_IN_VISIT();
    END_VISIT();
}

int ConstantFoldingPass::visit(const GreaterThanExpressionASTNodePtr &node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a > b ? 1 : 0; return 0; });
    SET_RESULT_RC_AND_RETURN_IN_VISIT();
    END_VISIT();
}

int ConstantFoldingPass::visit(const GreaterEqualExpressionASTNodePtr &node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a >= b ? 1 : 0; return 0; });
    SET_RESULT_RC_AND_RETURN_IN_VISIT();
    END_VISIT();
}

int ConstantFoldingPass::visit(const LessThanExpressionASTNodePtr &node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a < b ? 1 : 0; return 0; });
    SET_RESULT_RC_AND_RETURN_IN_VISIT();
    END_VISIT();
}

int ConstantFoldingPass::visit(const LessEqualExpressionASTNodePtr &node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a <= b ? 1 : 0;return 0; });
    SET_RESULT_RC_AND_RETURN_IN_VISIT();
    END_VISIT();
}

int ConstantFoldingPass::visit(const AndExpressionASTNodePtr &node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a && b ? 1 : 0; return 0; });
    SET_RESULT_RC_AND_RETURN_IN_VISIT();
    END_VISIT();
}

int ConstantFoldingPass::visit(const OrExpressionASTNodePtr &node)
{
    BEGIN_VISIT();
    rc = fold_binary_expression(node, [](int a, int b, int &out) { out = a || b ? 1 : 0; return 0; });
    SET_RESULT_RC_AND_RETURN_IN_VISIT();
    END_VISIT();
}

int ConstantFoldingPass::visit(const InvocationExpressionASTNodePtr &node)
{
    // There's no folding for invocation expression, but we can dive into it's expression.
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const EmptyStatementASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const IfStatementASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const WhileStatementASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const ForStatementASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const ReturnStatementASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const BreakStatementASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const ContinueStatementASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const StatementBlockASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const SubprocDefinitionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const FunctionDefinitionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::visit(const CompilationUnitASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int ConstantFoldingPass::run()
{
    return visit(_root);
}

void ConstantFoldingPass::attach_constant(const ASTNodePtr &node, int value, bool is_char)
{
    auto attr = std::make_shared<ConstantFoldingAttribute>(value, is_char);
    node->set_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, attr);
}

int ConstantFoldingPass::fold_binary_expression(const AbstractBinaryExpressionASTNodePtr &node, BinaryIntOperation auto op_func)
{
    int result = 0, rc = 0;

    rc = traverse(node->get_left());
    RETURN_IF_FAIL();

    rc = traverse(node->get_right());
    RETURN_IF_FAIL();

    ASTNodeAttributePtr left_const_fld;
    ASTNodeAttributePtr right_const_fld;
    bool left_const = node->get_left()->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, left_const_fld);
    bool right_const = node->get_right()->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, right_const_fld);

    if (left_const && right_const) {
        auto left_attr = std::static_pointer_cast<ConstantFoldingAttribute>(left_const_fld);
        auto right_attr = std::static_pointer_cast<ConstantFoldingAttribute>(right_const_fld);

        // only perform folding when two types are equal
        if (left_attr->get_is_char() == right_attr->get_is_char()) {
            int left = left_attr->get_value();
            int right = right_attr->get_value();

            int val;
            rc = op_func(left, right, val);
            RETURN_IF_FAIL();

            rc = check_integer_range(val, node);
            RETURN_IF_FAIL();

            attach_constant(node, val, left_attr->get_is_char());

            auto const_node = std::make_shared<IntegerASTNode>(node->lineno(), node->colno(), node->last_lineno(), node->last_colno(), val, left_attr->get_is_char());
            const_node->copy_attributes_from(node);
            attach_constant(const_node, val, left_attr->get_is_char());

            request_to_replace_self(const_node);
        }
    }
    return result;
}

int ConstantFoldingPass::fold_unary_expression(const AbstractUnaryExpressionASTNodePtr &node, UnaryIntOperation auto op_func)
{
    int result = 0, rc = 0;

    rc = traverse(node->get_operand());
    RETURN_IF_FAIL();

    ASTNodeAttributePtr const_fld;
    bool is_const = node->get_operand()->get_attribute(ATTR_SEMANALYZER_CONST_FOLDING_VALUE, const_fld);

    if (is_const) {
        auto attr = std::static_pointer_cast<ConstantFoldingAttribute>(const_fld);
        int operand = attr->get_value();
        int val;
        rc = op_func(operand, val);
        RETURN_IF_FAIL();

        rc = check_integer_range(val, node);
        RETURN_IF_FAIL();

        attach_constant(node, val, attr->get_is_char());
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

CLOSE_SEMANALYZER_NAMESPACE

// end
