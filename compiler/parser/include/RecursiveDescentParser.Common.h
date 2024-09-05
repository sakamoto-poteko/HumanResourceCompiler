#ifndef RECURSIVEDESCENTPARSER_COMMON_H
#define RECURSIVEDESCENTPARSER_COMMON_H

#include "RecursiveDescentParser.h"

OPEN_PARSER_NAMESPACE

#define LEAVE_PARSE_FRAME() \
    leave_parse_frame();    \
    return true

#define ENTER_PARSE_FRAME()                        \
    enter_parse_frame();                           \
    lexer::TokenPtr token = lookahead();           \
    int lineno = token->lineno();                  \
    int colno = token->colno();                    \
    int width = token->width();                    \
    auto last_error_it = _errors.empty() ? _errors.end() : std::prev(_errors.end())

#define CLEAR_ERROR_BEYOND() \
    pop_error_till(last_error_it)

#define TO_IDENTIFIER_TOKEN() \
    std::static_pointer_cast<hrl::lexer::IdentifierToken>(token)

#define TO_IDENTIFIER_NODE() \
    std::make_shared<IdentifierPTNode>(std::static_pointer_cast<hrl::lexer::IdentifierToken>(token))

#define TO_INTEGER_NODE() \
    std::make_shared<IntegerLiteralPTNode>(std::static_pointer_cast<hrl::lexer::IntegerToken>(token))

#define TO_BOOLEAN_NODE() \
    std::make_shared<BooleanLiteralPTNode>(std::static_pointer_cast<hrl::lexer::BooleanToken>(token))

#define CHECK_TOKEN_AND_CONSUME(expected_token, expected_message, token_name) \
    token = lookahead();                                                      \
    auto token_name = token;                                                  \
    if (token->token_id() != (expected_token)) {                              \
        push_error((expected_message), token);                                \
        revert_parse_frame();                                                 \
        return false;                                                         \
    }                                                                         \
    consume()

#define CONSUME_TOKEN() \
    consume()

#define UPDATE_TOKEN_LOOKAHEAD() \
    token = lookahead()

#define CHECK_ERROR_MSG(ok, msg, lineno, colno, width) \
    if (!(ok)) {                                       \
        push_error((msg), lineno, colno, width);       \
        revert_parse_frame();                          \
        return false;                                  \
    }0 // 0 is required to suppress empty statement check

#define CHECK_ERROR(ok)       \
    if (!(ok)) {              \
        revert_parse_frame(); \
        return false;         \
    }0  // 0 is required to suppress empty statement check

#define TOKEN_IS(id) \
    (token->token_id() == (id))

#define SET_NODE(...) \
    node = std::make_shared<std::remove_reference_t<decltype(node)>::element_type>(lineno, colno, ##__VA_ARGS__)

#define SET_NODE_FROM(ptr) \
    node = std::static_pointer_cast<std::remove_reference_t<decltype(node)>::element_type>(ptr)

CLOSE_PARSER_NAMESPACE

#endif