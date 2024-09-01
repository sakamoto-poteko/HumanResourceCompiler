#include <iterator>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include <boost/format.hpp>

#include <spdlog/spdlog.h>

#include "ASTNode.h"
#include "ASTNodeForward.h"
#include "HRLToken.h"
#include "RecursiveDescentParser.Common.h"
#include "RecursiveDescentParser.h"
#include "hrl_global.h"
#include "lexer_global.h"

OPEN_PARSER_NAMESPACE

bool RecursiveDescentParser::parse_expression(AbstractExpressionNodePtr &node)
{
    ENTER_PARSE_FRAME();
    // FIXME: impl
    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_unary_expression(AbstractUnaryExpressionNodePtr &node)
{
    ENTER_PARSE_FRAME();

    bool ok;
    AbstractPrimaryExpressionNodePtr primary;
    IncrementExpressionNodePtr increment;
    DecrementExpressionNodePtr decrement;

    switch (token->token_id()) {
    case lexer::ADD: // positive_expression
        CHECK_TOKEN_AND_CONSUME(lexer::ADD, "'+'");
        ok = parse_primary_expression(primary);
        CHECK_ERROR(ok);
        SET_NODE_FROM(std::make_shared<PositiveExpressionNode>(lineno, colno, primary));
        break;

    case lexer::SUB: // negative_expression
        CHECK_TOKEN_AND_CONSUME(lexer::SUB, "'-'");
        ok = parse_primary_expression(primary);
        CHECK_ERROR(ok);
        SET_NODE_FROM(std::make_shared<NegativeExpressionNode>(lineno, colno, primary));
        break;

    case lexer::ADDADD: // increment_expression
        CHECK_TOKEN_AND_CONSUME(lexer::ADDADD, "'++'");
        CHECK_TOKEN_AND_CONSUME(lexer::IDENTIFIER, "an identifier (variable name)");
        SET_NODE_FROM(std::make_shared<IncrementExpressionNode>(lineno, colno, TO_IDENTIFIER_NODE()));
        break;

    case lexer::SUBSUB: // decrement_expression
        CHECK_TOKEN_AND_CONSUME(lexer::SUBSUB, "'--'");
        CHECK_TOKEN_AND_CONSUME(lexer::IDENTIFIER, "an identifier (variable name)");
        SET_NODE_FROM(std::make_shared<IncrementExpressionNode>(lineno, colno, TO_IDENTIFIER_NODE()));
        break;

    case lexer::NOT: // not_expression
        CHECK_TOKEN_AND_CONSUME(lexer::NOT, "'!'");
        ok = parse_primary_expression(primary);
        CHECK_ERROR(ok);
        SET_NODE_FROM(std::make_shared<NotExpressionNode>(lineno, colno, primary));
        break;

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
        CHECK_ERROR_MSG(false, "Expect a unary expression but got '" + *token->token_text(), token->lineno(), token->colno());
    }

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_primary_expression(AbstractPrimaryExpressionNodePtr &node)
{
    ENTER_PARSE_FRAME();

    bool ok;
    BooleanLiteralNodePtr bool_literal;
    IntegerLiteralNodePtr int_literal;
    IdentifierNodePtr id;
    FloorAccessNodePtr floor_access;
    ParenthesizedExpressionNodePtr parenthesized_expr;

    switch (token->token_id()) {
    case lexer::BOOLEAN: // literal
        bool_literal = TO_BOOLEAN_NODE();
        SET_NODE_FROM(bool_literal);
        break;
    case lexer::FLOOR: // floor_access
        ok = parse_floor_access(floor_access);
        CHECK_ERROR(ok);
        node = std::static_pointer_cast<AbstractPrimaryExpressionNode>(floor_access);
        break;
    case lexer::IDENTIFIER: // invocation_expression
        id = TO_IDENTIFIER_NODE();
        SET_NODE_FROM(id);
        break;
    case lexer::INTEGER: // literal
        int_literal = TO_INTEGER_NODE();
        SET_NODE_FROM(int_literal);
        break;
    case lexer::OPEN_PAREN: // parenthesized_expression
        ok = parse_parenthesized_expression(parenthesized_expr);
        CHECK_ERROR(ok);
        SET_NODE_FROM(parenthesized_expr);
        break;
    default:
        CHECK_ERROR_MSG(false, "Expect a primary expression (literal/floor access/invocation/parathensized) but got '" + *token->token_text() + "'", token->lineno(), token->colno());
    }

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_invocation_expression(InvocationExpressionNodePtr &node)
{
    ENTER_PARSE_FRAME();

    CHECK_TOKEN_AND_CONSUME(lexer::IDENTIFIER, "an identifier (function or subprocedure)");
    CHECK_TOKEN_AND_CONSUME(lexer::OPEN_PAREN, "'(");
    AbstractExpressionNodePtr expr;
    bool ok = parse_expression(expr);
    CHECK_ERROR(ok);
    CHECK_TOKEN_AND_CONSUME(lexer::CLOSE_PAREN, "')'");

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_parenthesized_expression(ParenthesizedExpressionNodePtr &node)
{
    ENTER_PARSE_FRAME();

    CHECK_TOKEN_AND_CONSUME(lexer::OPEN_PAREN, "'('");
    AbstractExpressionNodePtr expr;
    bool ok = parse_expression(expr);
    CHECK_TOKEN_AND_CONSUME(lexer::CLOSE_PAREN, "')'");

    LEAVE_PARSE_FRAME();
}

CLOSE_PARSER_NAMESPACE

// end
