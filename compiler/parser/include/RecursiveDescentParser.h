#ifndef RECURSIVE_DESCENT_PARSER_H
#define RECURSIVE_DESCENT_PARSER_H

#include <cstddef>
#include <list>
#include <stack>
#include <string>
#include <vector>

#include "ErrorMessage.h"
#include "ParseTreeNodeForward.h"
#include "lexer_global.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

class RecursiveDescentParser {
public:
    explicit RecursiveDescentParser(const std::string &filename, const std::vector<lexer::TokenPtr> &token_list)
        : _tokens(token_list)
        , _filename(filename) {};

    virtual ~RecursiveDescentParser() = default;

    virtual bool parse(CompilationUnitPTNodePtr &result);

protected:
    std::vector<lexer::TokenPtr> _tokens;
    std::size_t _token_pointer = 0;
    std::stack<int> _parse_frame_token_pointer;

    std::list<CompilerMessage> _errors;

    [[nodiscard]] const lexer::TokenPtr &lookahead() const;
    void consume();

    [[nodiscard]] bool parse_compilation_unit(CompilationUnitPTNodePtr &node);
    [[nodiscard]] bool parse_import_directive(ImportDirectivePTNodePtr &node);
    [[nodiscard]] bool parse_floor_box_init_statement(FloorBoxInitStatementPTNodePtr &node);
    [[nodiscard]] bool parse_floor_max_statement(FloorMaxInitStatementPTNodePtr &node);
    [[nodiscard]] bool parse_function_definition(FunctionDefinitionPTNodePtr &node);
    [[nodiscard]] bool parse_subproc_definition(SubprocDefinitionPTNodePtr &node);
    [[nodiscard]] bool parse_statement_block(StatementBlockPTNodePtr &node);
    [[nodiscard]] bool parse_statement(AbstractStatementPTNodePtr &node);
    [[nodiscard]] bool parse_variable_declaration_statement(VariableDeclarationStatementPTNodePtr &node);
    [[nodiscard]] bool parse_variable_declaration(VariableDeclarationPTNodePtr &node);
    [[nodiscard]] bool parse_floor_assignment_statement(FloorAssignmentStatementPTNodePtr &node);
    [[nodiscard]] bool parse_floor_assignment(FloorAssignmentPTNodePtr &node);
    [[nodiscard]] bool parse_floor_access(FloorAccessPTNodePtr &node);
    [[nodiscard]] bool parse_variable_assignment_statement(VariableAssignmentStatementPTNodePtr &node);
    [[nodiscard]] bool parse_variable_assignment(VariableAssignmentPTNodePtr &node);
    [[nodiscard]] bool parse_invocation_statement(InvocationStatementPTNodePtr &node);
    [[nodiscard]] bool parse_embedded_statement(AbstractEmbeddedStatementPTNodePtr &node);
    [[nodiscard]] bool parse_if_statement(IfStatementPTNodePtr &node);
    [[nodiscard]] bool parse_while_statement(WhileStatementPTNodePtr &node);
    [[nodiscard]] bool parse_for_statement(ForStatementPTNodePtr &node);
    [[nodiscard]] bool parse_return_statement(ReturnStatementPTNodePtr &node);
    [[nodiscard]] bool parse_empty_statement(EmptyStatementPTNodePtr &node);

    [[nodiscard]] bool parse_expression(AbstractExpressionPTNodePtr &node);
    [[nodiscard]] bool parse_unary_expression(AbstractUnaryExpressionPTNodePtr &node);
    [[nodiscard]] bool parse_primary_expression(AbstractPrimaryExpressionPTNodePtr &node);
    [[nodiscard]] bool parse_invocation_expression(InvocationExpressionPTNodePtr &node);
    [[nodiscard]] bool parse_parenthesized_expression(ParenthesizedExpressionPTNodePtr &node);

    [[nodiscard]] bool parse_precedence_climbing(AbstractExpressionPTNodePtr &result, AbstractExpressionPTNodePtr lhs, int min_precedence);

    void push_error(const std::string &expect, const lexer::TokenPtr &got, int lineno = -1, int colno = -1, std::size_t width = 0);
    void push_error(int errid, const std::string &message, int lineno, int colno, std::size_t width);
    void pop_error();
    void pop_error_till(std::list<CompilerMessage>::iterator till_exclusive);
    void report_errors();

    void enter_parse_frame();
    void revert_parse_frame();
    void leave_parse_frame();

private:
    const static lexer::TokenPtr NULL_TOKEN;

    std::string _filename;
};

CLOSE_PARSER_NAMESPACE

#endif