#ifndef RECURSIVEDESCENTPARSER_H
#define RECURSIVEDESCENTPARSER_H

#include <list>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "ASTNodeForward.h"
#include "HRLToken.h"
#include "hrl_global.h"
#include "lexer_global.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

class RecursiveDescentParser {
public:
    RecursiveDescentParser(const std::vector<lexer::TokenPtr> &token_list)
        : _tokens(token_list) {
        };
    virtual ~RecursiveDescentParser() = default;

    virtual bool parse(CompilationUnitNodePtr &result);

protected:
    std::vector<lexer::TokenPtr> _tokens;
    int _token_pointer = 0;
    std::stack<int> _parse_frame_token_pointer;

    std::list<std::string> _errors;

    const lexer::TokenPtr &lookahead() const;
    lexer::TokenId lookahead_id() const;
    void consume();

    bool parse_compilation_unit(CompilationUnitNodePtr &node);
    bool parse_import_directive(ImportDirectiveNodePtr &node);
    bool parse_floor_box_init_statement(FloorBoxInitStatementNodePtr &node);
    bool parse_floor_max_statement(FloorMaxInitStatementNodePtr &node);
    bool parse_function_definition(FunctionDefinitionNodePtr &node);
    bool parse_subproc_definition(SubprocDefinitionNodePtr &node);
    bool parse_statement_block(StatementBlockNodePtr &node);
    bool parse_statement(AbstractStatementNodePtr &node);
    bool parse_variable_declaration_statement(VariableDeclarationStatementNodePtr &node);
    bool parse_variable_declaration(VariableDeclarationNodePtr &node);
    bool parse_floor_assignment_statement(FloorAssignmentStatementNodePtr &node);
    bool parse_floor_assignment(FloorAssignmentNodePtr &node);
    bool parse_floor_access(FloorAccessNodePtr &node);
    bool parse_variable_assignment_statement(VariableAssignmentStatementNodePtr &node);
    bool parse_variable_assignment(VariableAssignmentNodePtr &node);
    bool parse_invocation_statement(InvocationStatementNodePtr &node);
    bool parse_embedded_statement(AbstractEmbeddedStatementNodePtr &node);
    bool parse_if_statement(IfStatementNodePtr &node);
    bool parse_while_statement(WhileStatementNodePtr &node);
    bool parse_for_statement(ForStatementNodePtr &node);
    bool parse_return_statement(ReturnStatementNodePtr &node);
    bool parse_empty_statement(EmptyStatementNodePtr &node);

    bool parse_expression(AbstractExpressionNodePtr &node);
    bool parse_unary_expression(AbstractUnaryExpressionNodePtr &node);
    bool parse_primary_expression(AbstractPrimaryExpressionNodePtr &node);
    bool parse_invocation_expression(InvocationExpressionNodePtr &node);
    bool parse_parenthesized_expression(ParenthesizedExpressionNodePtr &node);

    bool parse_precedence_climbing(AbstractExpressionNodePtr &result, AbstractExpressionNodePtr lhs, int min_precedence);

    void push_error(const std::string &expect, const lexer::TokenPtr &got, int lineno = -1, int colno = -1, int width = -1);
    void push_error(const std::string &message, int lineno, int colno, int width);
    void pop_error();
    void pop_error_till(std::list<std::string>::iterator till_exclusive);
    void print_error();

    void enter_parse_frame();
    void revert_parse_frame();
    void leave_parse_frame();

private:
    const static lexer::TokenPtr NULL_TOKEN;
};

CLOSE_PARSER_NAMESPACE

#endif