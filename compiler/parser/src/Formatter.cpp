
#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>

#include <spdlog/spdlog.h>

#include "Formatter.h"
#include "ParseTreeNode.h"
#include "ParseTreeNodeForward.h"
#include "hrl_global.h"
#include "lexer_global.h"
#include "lexer_helper.h"

OPEN_PARSER_NAMESPACE

#define BEGIN_VISIT()              \
    bool comments_written = false; \
    if (comments_written) {        \
        UNUSED(node);              \
    }

#define DECL_TOKEN(var_name, func) \
    const auto &var_name = node->func;

#define WRITE_SEMICOLON() \
    WRITE_FOLLOWING_TOKEN(semicolon, false, false);

#define WRITE_FIRST_TOKEN(token, add_space_after)         \
    comments_written = process_preceding_metadata(token); \
    create_line(token, add_space_after);

#define WRITE_FOLLOWING_TOKEN(token, add_space_before, add_space_after) \
    comments_written = process_preceding_metadata(token);               \
    if (comments_written) {                                             \
        create_line(token, add_space_after);                            \
    } else {                                                            \
        append_line(token, add_space_before, add_space_after);          \
    }

void ParseTreeNodeFormatterVisitor::format(CompilationUnitPTNodePtr node)
{
    // FIXME: format is not yet supported
    spdlog::error("Format is not yet supported");
    node->accept(this);

    std::ostringstream buffer;

    buffer << "REFORMATTED:" << std::endl;
    for (const auto &line : _lines) {
        buffer << line.get_content() << std::endl;
    }

    std::cout << buffer.str() << std::endl;
    abort();
}

void ParseTreeNodeFormatterVisitor::visit(IdentifierPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(IntegerLiteralPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(BooleanLiteralPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(BinaryOperatorPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(VariableDeclarationPTNodePtr node)
{
    BEGIN_VISIT();

    DECL_TOKEN(let, get_let_token());
    DECL_TOKEN(var_name, get_var_name()->get_token());

    DECL_TOKEN(eq, get_equals());

    WRITE_FIRST_TOKEN(let, false);
    WRITE_FOLLOWING_TOKEN(var_name, true, false);

    if (eq) {
        WRITE_FOLLOWING_TOKEN(eq, true, true);
        node->get_expr()->accept(this);
    }
}

void ParseTreeNodeFormatterVisitor::visit(VariableAssignmentPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(FloorAssignmentPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(BinaryExpressionPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(NegativeExpressionPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(PositiveExpressionPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(NotExpressionPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(IncrementExpressionPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(DecrementExpressionPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(FloorAccessPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(ParenthesizedExpressionPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(InvocationExpressionPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(IfStatementPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(WhileStatementPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(ForStatementPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(ReturnStatementPTNodePtr node)
{
    BEGIN_VISIT();

    DECL_TOKEN(return_token, get_return_token());
    DECL_TOKEN(semicolon, get_semicolon());

    WRITE_FIRST_TOKEN(return_token, false);
    node->get_expr()->accept(this);
    WRITE_SEMICOLON();
}

void ParseTreeNodeFormatterVisitor::visit(FloorBoxInitStatementPTNodePtr node)
{
    BEGIN_VISIT();

    DECL_TOKEN(init_token, get_init_token());
    DECL_TOKEN(floor_token, get_floor_token());
    DECL_TOKEN(open_bracket, get_open_bracket());
    DECL_TOKEN(index, get_floor_index());
    DECL_TOKEN(close_bracket, get_close_bracket());
    DECL_TOKEN(eq, get_equal_token());
    DECL_TOKEN(val, get_value_token());
    DECL_TOKEN(semicolon, get_semicolon());

    WRITE_FIRST_TOKEN(init_token, false);
    WRITE_FOLLOWING_TOKEN(floor_token, true, false);
    WRITE_FOLLOWING_TOKEN(open_bracket, false, false);
    WRITE_FOLLOWING_TOKEN(index, false, false);
    WRITE_FOLLOWING_TOKEN(close_bracket, false, false);
    WRITE_FOLLOWING_TOKEN(eq, true, true);
    WRITE_FOLLOWING_TOKEN(val, false, false);
    WRITE_SEMICOLON();
}

void ParseTreeNodeFormatterVisitor::visit(FloorMaxInitStatementPTNodePtr node)
{
    BEGIN_VISIT();

    DECL_TOKEN(init_token, get_init_token());
    DECL_TOKEN(floormax_token, get_floor_max_token());
    DECL_TOKEN(eq, get_equals());
    DECL_TOKEN(value, get_value_token());
    DECL_TOKEN(semicolon, get_semicolon());

    WRITE_FIRST_TOKEN(init_token, false);
    WRITE_FOLLOWING_TOKEN(floormax_token, true, false);
    WRITE_FOLLOWING_TOKEN(eq, true, true);
    WRITE_FOLLOWING_TOKEN(value, false, false);
    WRITE_SEMICOLON();
}

void ParseTreeNodeFormatterVisitor::visit(EmptyStatementPTNodePtr node)
{
    BEGIN_VISIT();
    WRITE_FIRST_TOKEN(node->get_semicolon(), false);
}

void ParseTreeNodeFormatterVisitor::visit(StatementBlockPTNodePtr node)
{
    BEGIN_VISIT();
    DECL_TOKEN(open_brace, get_open_brace());
    DECL_TOKEN(close_brace, get_close_brace());

    WRITE_FIRST_TOKEN(open_brace, false);
    ++_indent_level;
    traverse_statements(node->get_statements());
    --_indent_level;
    WRITE_FIRST_TOKEN(close_brace, false);
}

void ParseTreeNodeFormatterVisitor::visit(VariableDeclarationStatementPTNodePtr node)
{
    BEGIN_VISIT();

    node->get_variable_decl()->accept(this);
    DECL_TOKEN(semicolon, get_semicolon());
    WRITE_SEMICOLON();
}

void ParseTreeNodeFormatterVisitor::visit(VariableAssignmentStatementPTNodePtr node)
{
    BEGIN_VISIT();
    DECL_TOKEN(semicolon, get_semicolon());
    node->get_variable_assignment()->accept(this);
    WRITE_SEMICOLON();
}

void ParseTreeNodeFormatterVisitor::visit(FloorAssignmentStatementPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(InvocationStatementPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(SubprocDefinitionPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(FunctionDefinitionPTNodePtr node)
{
    UNUSED(node);
}

void ParseTreeNodeFormatterVisitor::visit(ImportDirectivePTNodePtr node)
{
    BEGIN_VISIT();

    DECL_TOKEN(import_token, get_import_token());
    DECL_TOKEN(module_token, get_module_name()->get_token());
    DECL_TOKEN(semicolon, get_semicolon());

    WRITE_FIRST_TOKEN(import_token, true);
    WRITE_FOLLOWING_TOKEN(module_token, false, false)
    WRITE_FOLLOWING_TOKEN(semicolon, false, false)
}

void ParseTreeNodeFormatterVisitor::visit(CompilationUnitPTNodePtr node)
{
    traverse_import_directives(node->get_imports());
    traverse_floor_inits(node->get_floor_inits());
    node->get_floor_max()->accept(this);
    traverse_top_level_decls(node->get_top_level_decls());
    traverse_subroutines(node->get_subroutines());
}

bool ParseTreeNodeFormatterVisitor::process_preceding_metadata(lexer::TokenPtr token)
{
    const std::vector<lexer::TokenMetadata> &token_metadata = token->metadata();

    if (token_metadata.empty()) {
        return false;
    }

    auto list = token_metadata;
    auto last = std::unique(list.begin(), list.end(),
        [](const lexer::TokenMetadata &first, const lexer::TokenMetadata &second) {
            return first.type == lexer::TokenMetadata::Newline && second.type == lexer::TokenMetadata::Newline;
        });
    list.erase(last, list.end());

    std::vector<CommentGroup> groups;
    CommentGroup *current_group = nullptr;

    for (const auto &element : list) {
        if (element.type == lexer::TokenMetadata::Comment) {
            if (current_group == nullptr || !current_group->is_newline) {
                groups.emplace_back();
                current_group = &groups.back();
            }
            current_group->comments.push_back(element.value);
        } else {
            groups.emplace_back();
            groups.back().is_newline = true;
            current_group = nullptr;
        }
    }

    // groups now contains no consecutive new lines. consecutive comment lines are grouped.
    // if there's no newline, it means the first comment group is trailing comment of a token.
    // we need to put it into last line. if there's no last line, it's created as newlines then.
    if (!groups.front().is_newline) {
        CommentGroup &first_group = groups.front();
        if (_lines.empty()) {
            create_line(first_group);
        } else {
            _lines.back().set_trailing_comments(first_group);
        }
    }

    // remove the last newline
    if (groups.back().is_newline) {
        groups.pop_back();
    }

    if (groups.empty()) {
        return false;
    }

    // from the second element to the last, if it's a new line, we create a new line
    // otherwise, we create comment lines.
    for (auto it = std::next(groups.begin()); it != groups.end(); ++it) {
        if (it->is_newline) {
            create_line();
        } else {
            create_line(*it);
        }
    }

    return true;
}

void ParseTreeNodeFormatterVisitor::create_line(lexer::TokenPtr token, bool add_space_after)
{
    if (token) {
        _lines.emplace_back(_indent_level, get_text_from_token(token));
        if (add_space_after) {
            _lines.back().append_space();
        }
    } else {
        create_line();
    }
}

void ParseTreeNodeFormatterVisitor::append_line(lexer::TokenPtr token, bool add_space_before, bool add_space_after)
{
    if (token) {
        if (_lines.empty()) {
            // NOTE: add_space_before is ignored here.
            create_line(token, add_space_after);
        } else {
            auto &last = _lines.back();
            if (add_space_before) {
                last.append_space();
            }
            last.append_content(get_text_from_token(token));
            if (add_space_after) {
                last.append_space();
            }
        }
    }
}

void ParseTreeNodeFormatterVisitor::create_line()
{
    _lines.emplace_back(0, std::string());
}

void ParseTreeNodeFormatterVisitor::create_line(const CommentGroup &comment_group)
{
    for (auto &comment : comment_group.comments) {
        _lines.emplace_back(_indent_level, strip_comment(comment));
    }
}

void ParseTreeNodeFormatterVisitor::traverse_import_directives(const std::vector<ImportDirectivePTNodePtr> &imports)
{
    // sort the imports with module names, ascending
    auto sorted = imports;
    std::sort(sorted.begin(), sorted.end(), [](const ImportDirectivePTNodePtr &first, const ImportDirectivePTNodePtr &second) {
        return *first->get_module_name()->get_value() < *second->get_module_name()->get_value();
    });

    for (const auto &import : sorted) {
        import->accept(this);
    }
}

void ParseTreeNodeFormatterVisitor::traverse_floor_inits(const std::vector<FloorBoxInitStatementPTNodePtr> &floor_inits)
{
    auto sorted = floor_inits;
    std::sort(sorted.begin(), sorted.end(), [](const FloorBoxInitStatementPTNodePtr &first, const FloorBoxInitStatementPTNodePtr &second) {
        return first->get_index()->get_value() < second->get_index()->get_value();
    });

    for (const auto &init : floor_inits) {
        init->accept(this);
    }
}

void ParseTreeNodeFormatterVisitor::traverse_subroutines(const std::vector<AbstractSubroutinePTNodePtr> &subroutines)
{
    if (!subroutines.empty()) {
        create_line();

        for (const auto &subroutine : subroutines) {
            subroutine->accept(this);
        }
    }
}

void ParseTreeNodeFormatterVisitor::traverse_top_level_decls(const std::vector<VariableDeclarationStatementPTNodePtr> &tlds)
{
    if (!tlds.empty()) {
        create_line();
        for (const auto &tld : tlds) {
            tld->accept(this);
        }
    }
}

void ParseTreeNodeFormatterVisitor::traverse_statements(const std::vector<AbstractStatementPTNodePtr> &statements)
{
    for (const auto &stmt : statements) {
        stmt->accept(this);
    }
}

std::string ParseTreeNodeFormatterVisitor::strip_comment(StringPtr comment)
{
    std::string str = *comment;
    boost::algorithm::trim(str);
    return str;
}

std::string ParseTreeNodeFormatterVisitor::get_text_from_token(lexer::TokenPtr token)
{
    if (!(token->token_text()) || token->token_text()->empty()) {
        switch (token->token_id()) {
        case lexer::END:
            return "<EOF>";
        case lexer::IMPORT:
            return "import";
        case lexer::RETURN:
            return "return";
        case lexer::LET:
            return "let";
        case lexer::INIT:
            return "init";
        case lexer::FLOOR:
            return "floor";
        case lexer::FLOOR_MAX:
            return "floor_max";
        case lexer::FUNCTION:
            return "function";
        case lexer::SUBWORD:
            return "sub";
        case lexer::BOOLEAN:
            return "boolean";
        case lexer::INTEGER:
            return "integer";
        case lexer::IDENTIFIER:
            return "identifier";
        case lexer::IF:
            return "if";
        case lexer::ELSE:
            return "else";
        case lexer::WHILE:
            return "while";
        case lexer::FOR:
            return "for";
        case lexer::GE:
            return ">=";
        case lexer::LE:
            return "<=";
        case lexer::EE:
            return "==";
        case lexer::NE:
            return "!=";
        case lexer::GT:
            return ">";
        case lexer::LT:
            return "<";
        case lexer::AND:
            return "&";
        case lexer::OR:
            return "|";
        case lexer::NOT:
            return "!";
        case lexer::ADDADD:
            return "++";
        case lexer::ADD:
            return "+";
        case lexer::SUBSUB:
            return "--";
        case lexer::SUB:
            return "-";
        case lexer::MUL:
            return "*";
        case lexer::DIV:
            return "/";
        case lexer::MOD:
            return "%";
        case lexer::EQ:
            return "=";
        case lexer::T:
            return ";";
        case lexer::OPEN_PAREN:
            return "(";
        case lexer::CLOSE_PAREN:
            return ")";
        case lexer::OPEN_BRACE:
            return "{";
        case lexer::CLOSE_BRACE:
            return "}";
        case lexer::OPEN_BRACKET:
            return "[";
        case lexer::CLOSE_BRACKET:
            return "]";
        case lexer::COMMA:
            return ",";
        default:
            return "<unknown>";
        }
    } else {
        return *token->token_text();
    }
}

CLOSE_PARSER_NAMESPACE
// end
