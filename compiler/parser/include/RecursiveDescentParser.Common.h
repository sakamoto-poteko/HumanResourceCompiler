#ifndef RECURSIVEDESCENTPARSER_COMMON_H
#define RECURSIVEDESCENTPARSER_COMMON_H

#include "RecursiveDescentParser.h"

OPEN_PARSER_NAMESPACE

#define LEAVE_PARSE_FRAME() \
    leave_parse_frame();    \
    return true;

#define ENTER_PARSE_FRAME()                        \
    enter_parse_frame();                           \
    lexer::TokenPtr token = lookahead();           \
    int lineno = token->lineno();                  \
    int colno = token->colno();                    \
    int width = token->width();                    \
    std::list<std::string>::iterator last_error_it \
        = _errors.empty() ? _errors.end() : std::prev(_errors.end());

#define CLEAR_ERROR_BEYOND() \
    pop_error_till(last_error_it);

#define TO_IDENTIFIER_TOKEN() \
    std::static_pointer_cast<hrl::lexer::IdentifierToken>(token)

#define TO_IDENTIFIER_NODE() \
    std::make_shared<IdentifierNode>(std::static_pointer_cast<hrl::lexer::IdentifierToken>(token))

#define TO_INTEGER_NODE() \
    std::make_shared<IntegerLiteralNode>(std::static_pointer_cast<hrl::lexer::IntegerToken>(token))

#define TO_BOOLEAN_NODE() \
    std::make_shared<BooleanLiteralNode>(std::static_pointer_cast<hrl::lexer::BooleanToken>(token))

#define CHECK_TOKEN_AND_CONSUME(expected_token, expected_message) \
    token = lookahead();                                          \
    if (token->token_id() != (expected_token)) {                  \
        push_error((expected_message), token);                    \
        revert_parse_frame();                                     \
        return false;                                             \
    }                                                             \
    consume();

#define CONSUME_TOKEN() \
    consume();

#define UPDATE_TOKEN_LOOKAHEAD() \
    token = lookahead();

// #define CHECK_TOKEN(expected_token, expected_message) \
//     token = lookahead();                              \
//     if (token->token_id() != expected_token) {        \
//         push_error(expected_message, token);          \
//         revert_parse_frame();                         \
//         return false;                                 \
//     }

#define CHECK_ERROR_NO_LINE(ok, msg) \
    if (!(ok)) {                     \
        push_error((msg));           \
        revert_parse_frame();        \
        return false;                \
    }

#define CHECK_ERROR_MSG(ok, msg, lineno, colno, width) \
    if (!(ok)) {                                       \
        push_error((msg), lineno, colno, width);       \
        revert_parse_frame();                          \
        return false;                                  \
    }

#define CHECK_ERROR(ok)       \
    if (!(ok)) {              \
        revert_parse_frame(); \
        return false;         \
    }

#define TOKEN_IS(id) \
    (token->token_id() == (id))

#define SET_NODE(...) \
    node = std::make_shared<std::remove_reference_t<decltype(node)>::element_type>(lineno, colno, ##__VA_ARGS__)

#define SET_NODE_FROM(ptr) \
    node = std::static_pointer_cast<std::remove_reference_t<decltype(node)>::element_type>(ptr)

CLOSE_PARSER_NAMESPACE

#endif