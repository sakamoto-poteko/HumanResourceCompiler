#ifndef RECURSIVEDESCENTPARSER_H
#define RECURSIVEDESCENTPARSER_H

#include <list>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "ParseTreeNodeForward.h"
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

    virtual bool parse(CompilationUnitPTNodePtr &result);

protected:
    std::vector<lexer::TokenPtr> _tokens;
    int _token_pointer = 0;
    std::stack<int> _parse_frame_token_pointer;

    std::list<std::string> _errors;

    const lexer::TokenPtr &lookahead() const;
    lexer::TokenId lookahead_id() const;
    void consume();

    bool parse_compilation_unit(CompilationUnitPTNodePtr &node);
    bool parse_import_directive(ImportDirectivePTNodePtr &node);
    bool parse_floor_box_init_statement(FloorBoxInitStatementPTNodePtr &node);
    bool parse_floor_max_statement(FloorMaxInitStatementPTNodePtr &node);
    bool parse_function_definition(FunctionDefinitionPTNodePtr &node);
    bool parse_subproc_definition(SubprocDefinitionPTNodePtr &node);
    bool parse_statement_block(StatementBlockPTNodePtr &node);
    bool parse_statement(AbstractStatementPTNodePtr &node);
    bool parse_variable_declaration_statement(VariableDeclarationStatementPTNodePtr &node);
    bool parse_variable_declaration(VariableDeclarationPTNodePtr &node);
    bool parse_floor_assignment_statement(FloorAssignmentStatementPTNodePtr &node);
    bool parse_floor_assignment(FloorAssignmentPTNodePtr &node);
    bool parse_floor_access(FloorAccessPTNodePtr &node);
    bool parse_variable_assignment_statement(VariableAssignmentStatementPTNodePtr &node);
    bool parse_variable_assignment(VariableAssignmentPTNodePtr &node);
    bool parse_invocation_statement(InvocationStatementPTNodePtr &node);
    bool parse_embedded_statement(AbstractEmbeddedStatementPTNodePtr &node);
    bool parse_if_statement(IfStatementPTNodePtr &node);
    bool parse_while_statement(WhileStatementPTNodePtr &node);
    bool parse_for_statement(ForStatementPTNodePtr &node);
    bool parse_return_statement(ReturnStatementPTNodePtr &node);
    bool parse_empty_statement(EmptyStatementPTNodePtr &node);

    bool parse_expression(AbstractExpressionPTNodePtr &node);
    bool parse_unary_expression(AbstractUnaryExpressionPTNodePtr &node);
    bool parse_primary_expression(AbstractPrimaryExpressionPTNodePtr &node);
    bool parse_invocation_expression(InvocationExpressionPTNodePtr &node);
    bool parse_parenthesized_expression(ParenthesizedExpressionPTNodePtr &node);

    bool parse_precedence_climbing(AbstractExpressionPTNodePtr &result, AbstractExpressionPTNodePtr lhs, int min_precedence);

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