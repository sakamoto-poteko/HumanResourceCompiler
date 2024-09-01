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

    SET_NODE(imports, floor_inits, floor_max,
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

    CHECK_TOKEN_AND_CONSUME(lexer::IDENTIFIER, "an identifier")
    identifier = TO_IDENTIFIER_NODE();

    CHECK_TOKEN_AND_CONSUME(lexer::T, "';'");

    SET_NODE(identifier);

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

    SET_NODE(index, value);

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

    SET_NODE(max);

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

    SET_NODE(subproc_name, formal_parameters, body);

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

    SET_NODE(function_name, formal_parameters, body);

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

    SET_NODE(statements);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_variable_declaration_statement(VariableDeclarationStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();

    VariableDeclarationNodePtr decl;
    bool ok = parse_variable_declaration(decl);
    CHECK_ERROR(ok);

    CHECK_TOKEN_AND_CONSUME(lexer::T, "';'");

    SET_NODE(decl);

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
        SET_NODE_FROM(floor_assignment);
        break;
    case lexer::IDENTIFIER: // variable_assignment_statement
        ok = parse_variable_assignment_statement(var_assignment);
        CHECK_ERROR(ok);
        SET_NODE_FROM(var_assignment);
        break;
    case lexer::LET: // variable_declaration_statement
        ok = parse_variable_declaration_statement(var_decl);
        CHECK_ERROR(ok);
        SET_NODE_FROM(var_decl);
        break;
    case lexer::RETURN: // embedded_statement
    case lexer::T: // embedded_statement
    case lexer::IF: // embedded_statement
    case lexer::FOR: // embedded_statement
    case lexer::WHILE: // embedded_statement
    case lexer::OPEN_BRACE: // embedded_statement
        ok = parse_embedded_statement(embedded_statement);
        CHECK_ERROR(ok);
        SET_NODE_FROM(embedded_statement);
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

    SET_NODE(var_name, expr);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_floor_assignment_statement(FloorAssignmentStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();

    FloorAssignmentNodePtr floor_assignment;
    bool ok = parse_floor_assignment(floor_assignment);
    CHECK_ERROR(ok);
    CHECK_TOKEN_AND_CONSUME(lexer::T, "';'");

    SET_NODE(floor_assignment);

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

    SET_NODE(floor_access, expr);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_variable_assignment_statement(VariableAssignmentStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();

    VariableAssignmentNodePtr variable_assignment;

    bool ok;
    ok = parse_variable_assignment(variable_assignment);
    CHECK_ERROR(ok);

    CHECK_TOKEN_AND_CONSUME(lexer::T, "';'");

    SET_NODE(variable_assignment);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_variable_assignment(VariableAssignmentNodePtr &node)
{
    ENTER_PARSE_FRAME();

    CHECK_TOKEN_AND_CONSUME(lexer::IDENTIFIER, "an identifier (variable name)");
    IdentifierNodePtr variable = TO_IDENTIFIER_NODE();

    CHECK_TOKEN_AND_CONSUME(lexer::EQ, "'='");

    AbstractExpressionNodePtr expr;
    bool ok = parse_expression(expr);

    SET_NODE(variable, expr);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_embedded_statement(AbstractEmbeddedStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();

    ForStatementNodePtr for_statement;
    IfStatementNodePtr if_statement;
    ReturnStatementNodePtr return_statement;
    StatementBlockNodePtr statement_block;
    EmptyStatementNodePtr empty_statement;
    WhileStatementNodePtr while_statement;
    bool ok;

    switch (token->token_id()) {
    case lexer::FOR: // iteration_statement
        ok = parse_for_statement(for_statement);
        CHECK_ERROR(ok);
        SET_NODE_FROM(for_statement);
        break;
    case lexer::WHILE: // iteration_statement
        ok = parse_while_statement(while_statement);
        CHECK_ERROR(ok);
        SET_NODE_FROM(while_statement);
        break;
    case lexer::IF: // selection_statement
        ok = parse_if_statement(if_statement);
        CHECK_ERROR(ok);
        SET_NODE_FROM(if_statement);
        break;
    case lexer::OPEN_BRACE: // statement_block
        ok = parse_statement_block(statement_block);
        CHECK_ERROR(ok);
        SET_NODE_FROM(statement_block);
        break;
    case lexer::RETURN: // return_statement
        ok = parse_return_statement(return_statement);
        CHECK_ERROR(ok);
        SET_NODE_FROM(return_statement);
        break;
    case lexer::T: // empty_statement
        ok = parse_empty_statement(empty_statement);
        CHECK_ERROR(ok);
        SET_NODE_FROM(empty_statement);
        break;
    default:
        CHECK_ERROR_MSG(false,
            "Expect an iteration/selection/return/empty statement or a statement block but got '"
                + *token->token_text()
                + "'",
            lineno, colno);
    }

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

    SET_NODE(expr);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_if_statement(IfStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();

    CHECK_TOKEN_AND_CONSUME(lexer::IF, "'if'");
    CHECK_TOKEN_AND_CONSUME(lexer::OPEN_PAREN, "'('");

    bool ok;
    AbstractExpressionNodePtr cond;
    AbstractEmbeddedStatementNodePtr then_stmt;
    AbstractEmbeddedStatementNodePtr else_stmt;

    ok = parse_expression(cond);
    CHECK_ERROR(ok);

    CHECK_TOKEN_AND_CONSUME(lexer::CLOSE_PAREN, "')'");

    ok = parse_embedded_statement(then_stmt);
    CHECK_ERROR(ok);

    UPDATE_TOKEN_LOOKAHEAD();
    if (TOKEN_IS(lexer::ELSE)) {
        CHECK_TOKEN_AND_CONSUME(lexer::ELSE, "'else'");

        ok = parse_embedded_statement(else_stmt);
        CHECK_ERROR(ok);
    }

    SET_NODE(cond, then_stmt, else_stmt);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_while_statement(WhileStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();

    CHECK_TOKEN_AND_CONSUME(lexer::WHILE, "'while'");
    CHECK_TOKEN_AND_CONSUME(lexer::OPEN_PAREN, "'('");

    bool ok;
    AbstractExpressionNodePtr expr;
    AbstractEmbeddedStatementNodePtr body;

    ok = parse_expression(expr);
    CHECK_ERROR(ok);

    CHECK_TOKEN_AND_CONSUME(lexer::CLOSE_PAREN, "')'");

    ok = parse_embedded_statement(body);
    CHECK_ERROR(ok);

    SET_NODE(expr, body);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_for_statement(ForStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();

    CHECK_TOKEN_AND_CONSUME(lexer::FOR, "'for'");
    CHECK_TOKEN_AND_CONSUME(lexer::OPEN_PAREN, "'('");

    bool ok;

    VariableAssignmentNodePtr init_var_assignment;
    VariableDeclarationNodePtr init_var_declaration;
    AbstractExpressionNodePtr cond;
    AbstractExpressionNodePtr update;
    AbstractEmbeddedStatementNodePtr body;

    // optional init stmt
    UPDATE_TOKEN_LOOKAHEAD();
    if (token->token_id() != lexer::COMMA) {
        ok = parse_variable_assignment(init_var_assignment);
        // if it's not var assignment, it should be var decl
        if (!ok) {
            ok = parse_variable_declaration(init_var_declaration);
            if (ok) {
                CLEAR_ERROR_BEYOND();
            } else {
                CHECK_ERROR_MSG(false, "for init statment should either be variable assignment or variable declaration", token->lineno(), token->colno());
            }
        }
    }

    CHECK_TOKEN_AND_CONSUME(lexer::COMMA, "','");

    // optional cond stmt
    UPDATE_TOKEN_LOOKAHEAD();
    if (token->token_id() != lexer::COMMA) {
        ok = parse_expression(cond);
        CHECK_ERROR(ok);
    }

    CHECK_TOKEN_AND_CONSUME(lexer::COMMA, "','");

    // optional update stmt
    UPDATE_TOKEN_LOOKAHEAD();
    if (token->token_id() != lexer::COMMA) {
        ok = parse_expression(update);
        CHECK_ERROR(ok);
    }

    CHECK_TOKEN_AND_CONSUME(lexer::CLOSE_PAREN, "')'");

    ok = parse_embedded_statement(body);
    CHECK_ERROR(ok);

    if (init_var_assignment) {
        SET_NODE(init_var_assignment, cond, update, body);
    } else if (init_var_declaration) {
        SET_NODE(init_var_declaration, cond, update, body);
    } else {
        throw; // not supposed to be here
    }

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_return_statement(ReturnStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();

    CHECK_TOKEN_AND_CONSUME(lexer::RETURN, "'return'");

    bool ok;
    AbstractExpressionNodePtr expr;
    ok = parse_expression(expr);
    CHECK_ERROR(ok);

    CHECK_TOKEN_AND_CONSUME(lexer::T, "';'");

    SET_NODE(expr);

    LEAVE_PARSE_FRAME();
}

bool RecursiveDescentParser::parse_empty_statement(EmptyStatementNodePtr &node)
{
    ENTER_PARSE_FRAME();

    CHECK_TOKEN_AND_CONSUME(lexer::T, "';'");

    SET_NODE();

    LEAVE_PARSE_FRAME();
}

CLOSE_PARSER_NAMESPACE

// end
