#include <list>
#include <memory>
#include <string>


#include <spdlog/spdlog.h>

#include "HRLToken.h"
#include "ParseTreeNode.h"
#include "ParseTreeNodeForward.h"
#include "RecursiveDescentParser.Common.h"
#include "RecursiveDescentParser.h"

OPEN_PARSER_NAMESPACE

bool RecursiveDescentParser::parse_expression(AbstractExpressionPTNodePtr &node)
{
    ENTER_PARSE_FRAME();
    bool ok;

    AbstractUnaryExpressionPTNodePtr lhs;
    ok = parse_unary_expression(lhs);
    CHECK_ERROR(ok);

    ok = parse_precedence_climbing(node, lhs, 0);
    CHECK_ERROR(ok);

    LEAVE_PARSE_FRAME();
}

// Precedence climbing works both for binary and unary expression
bool RecursiveDescentParser::parse_precedence_climbing(AbstractExpressionPTNodePtr &result, AbstractExpressionPTNodePtr lhs, int min_precedence)
{
    ENTER_PARSE_FRAME();

    while (lexer::is_token_binary_operator(token->token_id())
        && BinaryOperatorPTNode::get_operator_precedence(token->token_id()) >= min_precedence) {

        bool ok;
        BinaryOperatorPTNodePtr op = std::make_shared<BinaryOperatorPTNode>(token);
        int current_precedence = BinaryOperatorPTNode::get_operator_precedence(token->token_id());
        CONSUME_TOKEN();

        AbstractUnaryExpressionPTNodePtr rhs_unary;
        AbstractExpressionPTNodePtr rhs;
        ok = parse_unary_expression(rhs_unary);
        CHECK_ERROR(ok);
        rhs = std::static_pointer_cast<AbstractExpressionPTNode>(rhs_unary);

        UPDATE_TOKEN_LOOKAHEAD();

        while (lexer::is_token_binary_operator(token->token_id())) {
            bool is_left_assoc = BinaryOperatorPTNode::get_operator_associativity(token->token_id()) == BinaryOperatorPTNode::LEFT_TO_RIGHT;
            bool is_right_assoc = !is_left_assoc;
            int lookahead_precedence = BinaryOperatorPTNode::get_operator_precedence(token->token_id());

            if ((is_left_assoc && lookahead_precedence > current_precedence) || (is_right_assoc && lookahead_precedence >= current_precedence)) {
                AbstractExpressionPTNodePtr rhs_result;
                ok = parse_precedence_climbing(
                    rhs_result,
                    rhs,
                    current_precedence == lookahead_precedence ? current_precedence : current_precedence + 1);
                CHECK_ERROR(ok);

                rhs = rhs_result;

                UPDATE_TOKEN_LOOKAHEAD();
            } else {
                break;
            }
        }
        lhs = std::make_shared<BinaryExpressionPTNode>(token->lineno(), token->colno(), lhs, op, rhs);
    }

    result = lhs;

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_unary_expression(AbstractUnaryExpressionPTNodePtr &node)
{
    ENTER_PARSE_FRAME();

    bool ok;
    AbstractPrimaryExpressionPTNodePtr primary;

    switch (token->token_id()) {
    case lexer::ADD: // positive_expression
    {
        CHECK_TOKEN_AND_CONSUME(lexer::ADD, "'+'", add_token);
        ok = parse_primary_expression(primary);
        CHECK_ERROR(ok);
        SET_NODE_FROM(std::make_shared<PositiveExpressionPTNode>(lineno, colno, primary, add_token));
        break;
    }

    case lexer::SUB: // negative_expression
    {
        CHECK_TOKEN_AND_CONSUME(lexer::SUB, "'-'", sub_token);
        ok = parse_primary_expression(primary);
        CHECK_ERROR(ok);
        SET_NODE_FROM(std::make_shared<NegativeExpressionPTNode>(lineno, colno, primary, sub_token));
        break;
    }

    case lexer::ADDADD: // increment_expression
    {
        CHECK_TOKEN_AND_CONSUME(lexer::ADDADD, "'++'", inc_token);
        CHECK_TOKEN_AND_CONSUME(lexer::IDENTIFIER, "an identifier (variable name)", id_token);
        SET_NODE_FROM(std::make_shared<IncrementExpressionPTNode>(lineno, colno, TO_IDENTIFIER_NODE(), inc_token));
        break;
    }
    
    case lexer::SUBSUB: // decrement_expression
    {
        DecrementExpressionPTNodePtr decrement;
        CHECK_TOKEN_AND_CONSUME(lexer::SUBSUB, "'--'", dec_token);
        CHECK_TOKEN_AND_CONSUME(lexer::IDENTIFIER, "an identifier (variable name)", id_token);
        SET_NODE_FROM(std::make_shared<DecrementExpressionPTNode>(lineno, colno, TO_IDENTIFIER_NODE(), dec_token));
        break;
    }

    case lexer::NOT: // not_expression
    {
        CHECK_TOKEN_AND_CONSUME(lexer::NOT, "'!'", not_token);
        ok = parse_primary_expression(primary);
        CHECK_ERROR(ok);
        SET_NODE_FROM(std::make_shared<NotExpressionPTNode>(lineno, colno, primary, not_token));
        break;
    }

    case lexer::BOOLEAN: // primary_expression
    case lexer::FLOOR: // primary_expression
    case lexer::IDENTIFIER: // primary_expression
    case lexer::INTEGER: // primary_expression
    case lexer::OPEN_PAREN: // primary_expression
        ok = parse_primary_expression(primary);
        CHECK_ERROR(ok);
        SET_NODE_FROM(primary);
        break;

    default:
        CHECK_ERROR_MSG(false, "Expect a unary expression but got '" + *token->token_text(), token->lineno(), token->colno(), token->width());
    }

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_primary_expression(AbstractPrimaryExpressionPTNodePtr &node)
{
    ENTER_PARSE_FRAME();

    bool ok;
    BooleanLiteralPTNodePtr bool_literal;
    IntegerLiteralPTNodePtr int_literal;
    IdentifierPTNodePtr id;
    FloorAccessPTNodePtr floor_access;
    ParenthesizedExpressionPTNodePtr parenthesized_expr;
    InvocationExpressionPTNodePtr invocation;

    switch (token->token_id()) {
    case lexer::BOOLEAN: // literal
        bool_literal = TO_BOOLEAN_NODE();
        CONSUME_TOKEN();
        SET_NODE_FROM(bool_literal);
        break;
    case lexer::FLOOR: // floor_access
        ok = parse_floor_access(floor_access);
        CHECK_ERROR(ok);
        node = std::static_pointer_cast<AbstractPrimaryExpressionPTNode>(floor_access);
        break;
    case lexer::IDENTIFIER: // invocation_expression, variable
        // it's either an invocation or a variable
        ok = parse_invocation_expression(invocation);
        if (ok) {
            SET_NODE_FROM(invocation);
        } else {
            CLEAR_ERROR_BEYOND();
            id = TO_IDENTIFIER_NODE();
            CONSUME_TOKEN();
            SET_NODE_FROM(id);
        }
        break;
    case lexer::INTEGER: // literal
        int_literal = TO_INTEGER_NODE();
        CONSUME_TOKEN();
        SET_NODE_FROM(int_literal);
        break;
    case lexer::OPEN_PAREN: // parenthesized_expression
        ok = parse_parenthesized_expression(parenthesized_expr);
        CHECK_ERROR(ok);
        SET_NODE_FROM(parenthesized_expr);
        break;
    default:
        CHECK_ERROR_MSG(false, "Expect a primary expression (literal/floor access/invocation/parenthesized) but got '" + *token->token_text() + "'", token->lineno(), token->colno(), token->width());
    }

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_invocation_expression(InvocationExpressionPTNodePtr &node)
{
    ENTER_PARSE_FRAME();

    IdentifierPTNodePtr func_name;
    AbstractExpressionPTNodePtr arg;

    CHECK_TOKEN_AND_CONSUME(lexer::IDENTIFIER, "an identifier (function or subprocedure)", id_token);
    func_name = TO_IDENTIFIER_NODE();

    CHECK_TOKEN_AND_CONSUME(lexer::OPEN_PAREN, "'(", open_paren);
    UPDATE_TOKEN_LOOKAHEAD();
    // optional expr
    if (!TOKEN_IS(lexer::CLOSE_PAREN)) {
        bool ok = parse_expression(arg);
        CHECK_ERROR(ok);

        UPDATE_TOKEN_LOOKAHEAD();
    }
    CHECK_TOKEN_AND_CONSUME(lexer::CLOSE_PAREN, "')'", close_paren);

    SET_NODE(func_name, arg, open_paren, close_paren);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_parenthesized_expression(ParenthesizedExpressionPTNodePtr &node)
{
    ENTER_PARSE_FRAME();

    CHECK_TOKEN_AND_CONSUME(lexer::OPEN_PAREN, "'('", open_paren);

    AbstractExpressionPTNodePtr expr;
    bool ok = parse_expression(expr);
    CHECK_ERROR(ok);

    CHECK_TOKEN_AND_CONSUME(lexer::CLOSE_PAREN, "')'", close_paren);

    SET_NODE(expr, open_paren, close_paren);

    LEAVE_PARSE_FRAME();
}

CLOSE_PARSER_NAMESPACE

// end
