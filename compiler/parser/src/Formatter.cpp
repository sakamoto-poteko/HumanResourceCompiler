
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

#include "ASTNode.h"
#include "ASTNodeForward.h"
#include "Formatter.h"
#include "hrl_global.h"
#include "lexer_global.h"
#include "lexer_helper.h"

OPEN_PARSER_NAMESPACE

#define BEGIN_VISIT() \
    bool comments_written = false;

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

void ASTNodeFormatterVisitor::format(CompilationUnitNodePtr node)
{
    node->accept(this);

    std::ostringstream buffer;

    for (const auto &line : _lines) {
        buffer << line.get_content() << std::endl;
    }

    std::cout << buffer.str() << std::endl;
};

void ASTNodeFormatterVisitor::visit(IdentifierNodePtr node) {};
void ASTNodeFormatterVisitor::visit(IntegerLiteralNodePtr node) {};
void ASTNodeFormatterVisitor::visit(BooleanLiteralNodePtr node) {};
void ASTNodeFormatterVisitor::visit(BinaryOperatorNodePtr node) {};
void ASTNodeFormatterVisitor::visit(VariableDeclarationNodePtr node) {};
void ASTNodeFormatterVisitor::visit(VariableAssignmentNodePtr node) {};
void ASTNodeFormatterVisitor::visit(FloorAssignmentNodePtr node) {};
void ASTNodeFormatterVisitor::visit(BinaryExpressionNodePtr node) {};
void ASTNodeFormatterVisitor::visit(NegativeExpressionNodePtr node) {};
void ASTNodeFormatterVisitor::visit(PositiveExpressionNodePtr node) {};
void ASTNodeFormatterVisitor::visit(NotExpressionNodePtr node) {};
void ASTNodeFormatterVisitor::visit(IncrementExpressionNodePtr node) {};
void ASTNodeFormatterVisitor::visit(DecrementExpressionNodePtr node) {};
void ASTNodeFormatterVisitor::visit(FloorAccessNodePtr node) {};
void ASTNodeFormatterVisitor::visit(ParenthesizedExpressionNodePtr node) {};
void ASTNodeFormatterVisitor::visit(InvocationExpressionNodePtr node) {};
void ASTNodeFormatterVisitor::visit(IfStatementNodePtr node) {};
void ASTNodeFormatterVisitor::visit(WhileStatementNodePtr node) {};
void ASTNodeFormatterVisitor::visit(ForStatementNodePtr node) {};
void ASTNodeFormatterVisitor::visit(ReturnStatementNodePtr node) {};

void ASTNodeFormatterVisitor::visit(FloorBoxInitStatementNodePtr node)
{
    BEGIN_VISIT()

    const auto &init_token = node->get_init_token();
    const auto &floor_token = node->get_floor_token();
    const auto &open_bracket = node->get_open_bracket();
    const auto &index = node->get_floor_index();
    const auto &close_bracket = node->get_close_bracket();
    const auto &eq = node->get_equal_token();
    const auto &val = node->get_value_token();
    const auto &semicolon = node->get_semicolon();

    WRITE_FIRST_TOKEN(init_token, false);
    WRITE_FOLLOWING_TOKEN(floor_token, true, false);
    WRITE_FOLLOWING_TOKEN(open_bracket, false, false);
    WRITE_FOLLOWING_TOKEN(index, false, false);
    WRITE_FOLLOWING_TOKEN(close_bracket, false, false);
    WRITE_FOLLOWING_TOKEN(eq, true, true);
    WRITE_FOLLOWING_TOKEN(val, false, false);
    WRITE_FOLLOWING_TOKEN(semicolon, false, false);
};

void ASTNodeFormatterVisitor::visit(FloorMaxInitStatementNodePtr node) {};
void ASTNodeFormatterVisitor::visit(EmptyStatementNodePtr node) {};
void ASTNodeFormatterVisitor::visit(StatementBlockNodePtr node) {};
void ASTNodeFormatterVisitor::visit(VariableDeclarationStatementNodePtr node) {};
void ASTNodeFormatterVisitor::visit(VariableAssignmentStatementNodePtr node) {};
void ASTNodeFormatterVisitor::visit(FloorAssignmentStatementNodePtr node) {};
void ASTNodeFormatterVisitor::visit(InvocationStatementNodePtr node) {};
void ASTNodeFormatterVisitor::visit(SubprocDefinitionNodePtr node) {};
void ASTNodeFormatterVisitor::visit(FunctionDefinitionNodePtr node) {};

void ASTNodeFormatterVisitor::visit(ImportDirectiveNodePtr node)
{
    BEGIN_VISIT();

    const auto &import_token = node->get_import_token();
    const auto &module_token = node->get_module_name()->get_token();
    const auto &semicolon = node->get_semicolon();

    WRITE_FIRST_TOKEN(import_token, true);
    WRITE_FOLLOWING_TOKEN(module_token, false, false)
    WRITE_FOLLOWING_TOKEN(semicolon, false, false)
};

void ASTNodeFormatterVisitor::visit(CompilationUnitNodePtr node)
{
    traverse_import_directives(node->get_imports());
    traverse_floor_inits(node->get_floor_inits());
    node->get_floor_max()->accept(this);
    traverse_top_level_decls(node->get_top_level_decls());
    traverse_subroutines(node->get_subroutines());
};

bool ASTNodeFormatterVisitor::process_preceding_metadata(lexer::TokenPtr token)
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

void ASTNodeFormatterVisitor::create_line(lexer::TokenPtr token, bool add_space_after)
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

void ASTNodeFormatterVisitor::append_line(lexer::TokenPtr token, bool add_space_before, bool add_space_after)
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

void ASTNodeFormatterVisitor::create_line()
{
    _lines.emplace_back(0, std::string());
}

void ASTNodeFormatterVisitor::create_line(const CommentGroup &comment_group)
{
    for (auto &comment : comment_group.comments) {
        _lines.emplace_back(_indent_level, strip_comment(comment));
    }
}

void ASTNodeFormatterVisitor::traverse_import_directives(const std::vector<ImportDirectiveNodePtr> &imports)
{
    // sort the imports with module names, ascending
    auto sorted = imports;
    std::sort(sorted.begin(), sorted.end(), [](const ImportDirectiveNodePtr &first, const ImportDirectiveNodePtr &second) {
        return *first->get_module_name()->get_value() < *second->get_module_name()->get_value();
    });

    for (const auto &import : sorted) {
        import->accept(this);
    }
}

void ASTNodeFormatterVisitor::traverse_floor_inits(const std::vector<FloorBoxInitStatementNodePtr> &floor_inits)
{
    auto sorted = floor_inits;
    std::sort(sorted.begin(), sorted.end(), [](const FloorBoxInitStatementNodePtr &first, const FloorBoxInitStatementNodePtr &second) {
        return first->get_index()->get_value() < second->get_index()->get_value();
    });

    for (const auto &init : floor_inits) {
        init->accept(this);
    }
}

void ASTNodeFormatterVisitor::traverse_subroutines(const std::vector<AbstractSubroutineNodePtr> &subroutines)
{
    //
}

void ASTNodeFormatterVisitor::traverse_top_level_decls(const std::vector<VariableDeclarationStatementNodePtr> &tlds)
{
    //
}

std::string ASTNodeFormatterVisitor::strip_comment(StringPtr comment)
{
    std::string str = *comment;
    boost::algorithm::trim(str);
    return str;
}

std::string ASTNodeFormatterVisitor::get_text_from_token(lexer::TokenPtr token)
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
