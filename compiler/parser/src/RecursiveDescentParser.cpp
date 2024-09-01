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

#define CHECK_ERROR_MSG(ok, msg, lineno, colno) \
    if (!(ok)) {                                \
        push_error((msg), lineno, colno);       \
        revert_parse_frame();                   \
        return false;                           \
    }

#define CHECK_ERROR(ok)       \
    if (!(ok)) {              \
        revert_parse_frame(); \
        return false;         \
    }

#define TOKEN_IS(id) \
    (token->token_id() == (id))

#define SET_NODE(T, ...) \
    std::make_shared<T>(lineno, colno, ##__VA_ARGS__)


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

    std::vector<VariableDeclarationStatementNodePtr> variable_declarations;
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
            VariableDeclarationStatementNodePtr var;
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

bool RecursiveDescentParser::parse_variable_declaration_statement(VariableDeclarationStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();

    VariableDeclarationNodePtr decl;
    bool ok = parse_variable_declaration(decl);
    CHECK_ERROR(ok);

    CHECK_TOKEN_AND_CONSUME(lexer::T, "';'");

    node = std::make_shared<VariableDeclarationStatementNode>(lineno, colno, decl);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_statement(AbstractStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();

    bool ok;
    // statement
    //        = variable_declaration_statement
    //        | variable_assignment_statement
    //        | floor_assignment_statement
    //        | embedded_statement;

    FloorAssignmentStatementNodePtr floor_assignment;
    VariableAssignmentStatementNodePtr var_assignment;
    VariableDeclarationStatementNodePtr var_decl;
    AbstractEmbeddedStatementNodePtr embedded_statement;

    switch (token->token_id()) {
    case lexer::FLOOR: // floor_assignment_statement
        ok = parse_floor_assignment_statement(floor_assignment);
        CHECK_ERROR(ok);
        node = std::static_pointer_cast<AbstractStatementNode>(floor_assignment);
        break;
    case lexer::IDENTIFIER: // variable_assignment_statement
        ok = parse_variable_assignment_statement(var_assignment);
        CHECK_ERROR(ok);
        node = std::static_pointer_cast<AbstractStatementNode>(var_assignment);
        break;
    case lexer::LET: // variable_declaration_statement
        ok = parse_variable_declaration_statement(var_decl);
        CHECK_ERROR(ok);
        node = std::static_pointer_cast<AbstractStatementNode>(var_decl);
        break;
    case lexer::RETURN: // embedded_statement
    case lexer::T: // embedded_statement
    case lexer::IF: // embedded_statement
    case lexer::FOR: // embedded_statement
    case lexer::WHILE: // embedded_statement
    case lexer::OPEN_BRACE: // embedded_statement
        ok = parse_embedded_statement(embedded_statement);
        CHECK_ERROR(ok);
        node = std::static_pointer_cast<AbstractStatementNode>(embedded_statement);
        break;
    default:
        CHECK_ERROR_MSG(false, "Expect a statement but got '" + *token->token_text() + "'", lineno, colno);
    }

    // FIXME: impl
    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_variable_declaration(VariableDeclarationNodePtr &node)
{
    ENTER_PARSE_FRAME();

    IdentifierNodePtr var_name;
    AbstractExpressionNodePtr expr;

    CHECK_TOKEN_AND_CONSUME(lexer::LET, "let");

    CHECK_TOKEN_AND_CONSUME(lexer::IDENTIFIER, "an identifier (variable name)");
    var_name = TO_IDENTIFIER_NODE();

    UPDATE_TOKEN_LOOKAHEAD();
    // optional assignment?
    if (TOKEN_IS(lexer::EQ)) {
        CHECK_TOKEN_AND_CONSUME(lexer::EQ, "'='");
        bool ok = parse_expression(expr);
        CHECK_ERROR(ok);
    }

    node = std::make_shared<VariableDeclarationNode>(lineno, colno, var_name, expr);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_expression(AbstractExpressionNodePtr &node)
{
    ENTER_PARSE_FRAME();
    // FIXME: impl
    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_floor_assignment_statement(FloorAssignmentStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();

    FloorAssignmentNodePtr floor_assignment;
    bool ok = parse_floor_assignment(floor_assignment);
    CHECK_ERROR(ok);
    CHECK_TOKEN_AND_CONSUME(lexer::T, "';'");

    node = std::make_shared<FloorAssignmentStatementNode>(lineno, colno, floor_assignment);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_floor_assignment(FloorAssignmentNodePtr &node)
{
    ENTER_PARSE_FRAME();

    FloorAccessNodePtr floor_access;
    AbstractExpressionNodePtr expr;

    bool ok;
    ok = parse_floor_access(floor_access);
    CHECK_ERROR(ok);

    CHECK_TOKEN_AND_CONSUME(lexer::EQ, "'='");

    ok = parse_expression(expr);
    CHECK_ERROR(ok);

    node = std::make_shared<FloorAssignmentNode>(lineno, colno, floor_access, expr);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_variable_assignment_statement(VariableAssignmentStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();
    // FIXME: impl
    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_variable_assignment(VariableAssignmentNodePtr &node)
{
    ENTER_PARSE_FRAME();
    // FIXME: impl
    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_embedded_statement(AbstractEmbeddedStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();
    // FIXME: impl
    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_floor_access(FloorAccessNodePtr &node)
{
    ENTER_PARSE_FRAME();

    CHECK_TOKEN_AND_CONSUME(lexer::FLOOR, "'floor'");
    CHECK_TOKEN_AND_CONSUME(lexer::OPEN_BRACKET, "'['");

    AbstractExpressionNodePtr expr;
    bool ok = parse_expression(expr);
    CHECK_ERROR(ok);

    CHECK_TOKEN_AND_CONSUME(lexer::CLOSE_BRACKET, "']'");

    node = std::make_shared<FloorAccessNode>(lineno, colno, expr);

    LEAVE_PARSE_FRAME();
}

CLOSE_PARSER_NAMESPACE

// end
