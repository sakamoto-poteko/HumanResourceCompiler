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
#include "RecursiveDescentParser.h"
#include "hrl_global.h"
#include "lexer_global.h"

OPEN_PARSER_NAMESPACE

// #define ENTER_PARSE_FRAME() \
//     enter_parse_frame();

#define LEAVE_PARSE_FRAME() \
    leave_parse_frame();    \
    return true;

#define ENTER_PARSE_FRAME()                        \
    enter_parse_frame();                           \
    TokenPtr token = lookahead();                  \
    int lineno = token->lineno();                  \
    int colno = token->colno();                    \
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

#define CHECK_TOKEN_AND_CONSUME(expected_token, expected_message) \
    token = lookahead();                                          \
    if (token->token_id() != expected_token) {                    \
        push_error(expected_message, token);                      \
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
    if (!ok) {                       \
        push_error(msg);             \
        revert_parse_frame();        \
        return false;                \
    }

#define CHECK_ERROR_MSG(ok, msg, lineno, colno) \
    if (!ok) {                                  \
        push_error(msg, lineno, colno);         \
        revert_parse_frame();                   \
        return false;                           \
    }

#define CHECK_ERROR(ok)       \
    if (!ok) {                \
        revert_parse_frame(); \
        return false;         \
    }

#define TOKEN_IS(id) \
    (token->token_id() == id)

using namespace lexer;

const lexer::TokenPtr RecursiveDescentParser::NULL_TOKEN = lexer::TokenPtr();

const TokenPtr &RecursiveDescentParser::lookahead() const
{
    if (_token_pointer < _tokens.size()) {
        return _tokens.at(_token_pointer);
    } else {
        return NULL_TOKEN;
    }
}

TokenId RecursiveDescentParser::lookahead_id() const
{
    if (_token_pointer < _tokens.size()) {
        return _tokens.at(_token_pointer)->token_id();
    } else {
        return END;
    }
}

inline void RecursiveDescentParser::consume()
{
    ++_token_pointer;
}

bool RecursiveDescentParser::parse_compilation_unit(CompilationUnitNodePtr &node)
{
    ENTER_PARSE_FRAME();

    std::vector<ImportDirectiveNodePtr> imports;
    std::vector<FloorBoxInitStatementNodePtr> floor_inits;
    FloorMaxInitStatementNodePtr floor_max;

    std::vector<VariableDeclarationNodePtr> variable_declarations;
    std::vector<FunctionDefinitionNodePtr> function_definitions;
    std::vector<SubprocDefinitionNodePtr> subproc_definitions;

    // There must be 0 to any import directives at the beginning of the file
    while (TOKEN_IS(lexer::IMPORT)) {
        ImportDirectiveNodePtr import;
        bool ok = parse_import_directive(import);
        CHECK_ERROR(ok);
        imports.push_back(import);

        UPDATE_TOKEN_LOOKAHEAD();
    }

    // Then floor_box_init_statement and floor_max_init_statement in any order
    // They both starts with 'init'
    while (TOKEN_IS(lexer::INIT)) {
        FloorBoxInitStatementNodePtr init;
        bool ok = parse_floor_box_init_statement(init);
        if (ok) {
            floor_inits.push_back(init);
        } else {
            // It's either floor init or floor max
            FloorMaxInitStatementNodePtr max;
            ok = parse_floor_max_statement(max);

            // if !ok
            CHECK_ERROR_MSG(
                ok,
                "Expect either 'init floor' or 'init floor_max' statement.",
                token->lineno(),
                token->colno());

            // clear the error of parse floor box init failure
            CLEAR_ERROR_BEYOND();

            if (floor_max) {
                // floor max already set. there can be only one.
                push_error("Maximum one 'init floor_max' allowed");
                revert_parse_frame();
                return false;
            } else {
                floor_max.swap(max);
            }
        }

        UPDATE_TOKEN_LOOKAHEAD();
    }

    // Then variable_declaration_statement and function_definition and subproc_definition in any order
    do {
        bool ok = false;
        if (TOKEN_IS(lexer::LET)) {
            VariableDeclarationNodePtr var;
            ok = parse_variable_declaration_statement(var);
            CHECK_ERROR(ok);

            variable_declarations.push_back(var);
        } else if (TOKEN_IS(lexer::FUNCTION)) {
            FunctionDefinitionNodePtr func;
            ok = parse_function_definition(func);
            CHECK_ERROR(ok);

            function_definitions.push_back(func);
        } else if (TOKEN_IS(lexer::SUBWORD)) {
            SubprocDefinitionNodePtr sub;
            ok = parse_subproc_definition(sub);
            CHECK_ERROR(ok);

            subproc_definitions.push_back(sub);
        } else {
            throw; // won't be here
        }

        UPDATE_TOKEN_LOOKAHEAD();
    } while (TOKEN_IS(lexer::LET) || TOKEN_IS(lexer::FUNCTION) || TOKEN_IS(lexer::SUBWORD));

    node = std::make_shared<CompilationUnitNode>(
        lineno, colno,
        imports,
        floor_inits, floor_max,
        variable_declarations, function_definitions, subproc_definitions);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse(CompilationUnitNodePtr &result)
{
    bool success = parse_compilation_unit(result);

    if (!success) {
        print_error();
    }

    return success;
}

bool RecursiveDescentParser::parse_import_directive(ImportDirectiveNodePtr &node)
{
    ENTER_PARSE_FRAME();

    IdentifierNodePtr identifier;

    CHECK_TOKEN_AND_CONSUME(lexer::IMPORT, "'import'");

    CHECK_TOKEN_AND_CONSUME(IDENTIFIER, "an identifier")
    identifier = TO_IDENTIFIER_NODE();

    CHECK_TOKEN_AND_CONSUME(lexer::T, "';'");

    node = std::make_shared<ImportDirectiveNode>(lineno, colno, identifier);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_floor_box_init_statement(FloorBoxInitStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();

    IntegerLiteralNodePtr index, value;

    CHECK_TOKEN_AND_CONSUME(lexer::INIT, "'init'");
    CHECK_TOKEN_AND_CONSUME(lexer::FLOOR, "'floor'");
    CHECK_TOKEN_AND_CONSUME(lexer::OPEN_BRACKET, "'['");

    CHECK_TOKEN_AND_CONSUME(lexer::INTEGER, "an integer (floor index)");
    index = TO_INTEGER_NODE();

    CHECK_TOKEN_AND_CONSUME(lexer::CLOSE_BRACKET, "']'");
    CHECK_TOKEN_AND_CONSUME(lexer::EQ, "'='");

    CHECK_TOKEN_AND_CONSUME(lexer::INTEGER, "an integer (value)");
    value = TO_INTEGER_NODE();

    CHECK_TOKEN_AND_CONSUME(lexer::T, "';'");

    node = std::make_shared<FloorBoxInitStatementNode>(lineno, colno, index, value);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_floor_max_statement(FloorMaxInitStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();

    IntegerLiteralNodePtr max;

    CHECK_TOKEN_AND_CONSUME(lexer::INIT, "'init'");
    CHECK_TOKEN_AND_CONSUME(lexer::FLOOR_MAX, "'floor_max'");
    CHECK_TOKEN_AND_CONSUME(lexer::EQ, "'='");
    CHECK_TOKEN_AND_CONSUME(lexer::INTEGER, "an integer (floor max value)");
    max = TO_INTEGER_NODE();
    CHECK_TOKEN_AND_CONSUME(lexer::T, "';'");

    node = std::make_shared<FloorMaxInitStatementNode>(lineno, colno, max);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_subproc_definition(SubprocDefinitionNodePtr &node)
{
    ENTER_PARSE_FRAME();

    IdentifierNodePtr subproc_name;
    std::vector<IdentifierNodePtr> formal_parameters;
    StatementBlockNodePtr body;

    CHECK_TOKEN_AND_CONSUME(lexer::SUBWORD, "'sub'");
    CHECK_TOKEN_AND_CONSUME(lexer::IDENTIFIER, "an identifier (subproc name)");
    subproc_name = TO_IDENTIFIER_NODE();
    CHECK_TOKEN_AND_CONSUME(lexer::OPEN_PAREN, "'('");

    UPDATE_TOKEN_LOOKAHEAD();
    while (TOKEN_IS(lexer::IDENTIFIER)) {
        CHECK_TOKEN_AND_CONSUME(lexer::IDENTIFIER, "an identifier (formal parameter)");
        formal_parameters.push_back(TO_IDENTIFIER_NODE());
        UPDATE_TOKEN_LOOKAHEAD();
    }
    CHECK_TOKEN_AND_CONSUME(lexer::CLOSE_PAREN, "')'");
    bool ok = parse_statement_block(body);
    CHECK_ERROR(ok);

    node = std::make_shared<SubprocDefinitionNode>(lineno, colno, subproc_name, formal_parameters, body);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_function_definition(FunctionDefinitionNodePtr &node)
{
    ENTER_PARSE_FRAME();

    IdentifierNodePtr function_name;
    std::vector<IdentifierNodePtr> formal_parameters;
    StatementBlockNodePtr body;

    CHECK_TOKEN_AND_CONSUME(lexer::FUNCTION, "'function'");
    CHECK_TOKEN_AND_CONSUME(lexer::IDENTIFIER, "an identifier (function name)");
    function_name = TO_IDENTIFIER_NODE();
    CHECK_TOKEN_AND_CONSUME(lexer::OPEN_PAREN, "'('");

    UPDATE_TOKEN_LOOKAHEAD();
    while (TOKEN_IS(lexer::IDENTIFIER)) {
        CHECK_TOKEN_AND_CONSUME(lexer::IDENTIFIER, "an identifier (formal parameter)");
        formal_parameters.push_back(TO_IDENTIFIER_NODE());
        UPDATE_TOKEN_LOOKAHEAD();
    }
    CHECK_TOKEN_AND_CONSUME(lexer::CLOSE_PAREN, "')'");
    bool ok = parse_statement_block(body);
    CHECK_ERROR(ok);

    node = std::make_shared<FunctionDefinitionNode>(lineno, colno, function_name, formal_parameters, body);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_statement_block(StatementBlockNodePtr &node)
{
    ENTER_PARSE_FRAME();

    std::vector<AbstractStatementNodePtr> statements;

    CHECK_TOKEN_AND_CONSUME(lexer::OPEN_BRACE, "'{'");

    // zero or more statements
    UPDATE_TOKEN_LOOKAHEAD();

    while (!TOKEN_IS(lexer::CLOSE_BRACE)) {
        AbstractStatementNodePtr stmt;
        bool ok = parse_statement(stmt);
        CHECK_ERROR(ok);
        statements.push_back(stmt);

        UPDATE_TOKEN_LOOKAHEAD();
    }

    CHECK_TOKEN_AND_CONSUME(lexer::CLOSE_BRACE, "'}'");

    node = std::make_shared<StatementBlockNode>(lineno, colno, statements);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_variable_declaration_statement(VariableDeclarationNodePtr &node)
{
    ENTER_PARSE_FRAME();

    // FIXME: add
    // node = std::make_shared<VariableDeclarationNode>();

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_statement(AbstractStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();

    LEAVE_PARSE_FRAME();
}

void RecursiveDescentParser::placeholder()
{
    // TODO: clangd anchoring. remove after dev.
}

CLOSE_PARSER_NAMESPACE

// end
