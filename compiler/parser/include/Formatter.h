#ifndef FORMATTER_H
#define FORMATTER_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "HRLToken.h"
#include "ParseTreeNode.h"
#include "ParseTreeNodeForward.h"
#include "ParseTreeNodeVisitor.h"
#include "hrl_global.h"
#include "lexer_global.h"
#include "lexer_helper.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

// it's either newline or comments
struct CommentGroup {
    std::vector<StringPtr> comments;
    bool is_newline = false;
};

// If the comment is a new-line comment, it'll be a FormatterLine
// If the comment is immediately following something, it'll be hooked as a comment group
// The comment group should have the same comment indentation
class FormatterLine {
public:
    FormatterLine(int indentation_level, const std::string &content)
        : _indentation_level(indentation_level)
        , _content(content)
    {
    }

    int get_indentation_level() const { return _indentation_level; }

    void set_indentation_level(int level) { _indentation_level = level; }

    const std::string &get_content() const { return _content; }

    void set_content(const std::string &content) { _content = content; }

    void append_content(const std::string &content) { _content.append(content); }

    void append_space() { _content.append(" "); }

    const CommentGroup &get_trailing_comments() const { return _comment_group; }

    void set_trailing_comments(const CommentGroup &comments) { _comment_group = comments; }

protected:
    int _indentation_level;
    std::string _content;
    CommentGroup _comment_group;
};

class ParseTreeNodeFormatterVisitor : public ParseTreeNodeVisitor {
public:
    virtual void format(CompilationUnitPTNodePtr node);

    virtual void visit(IdentifierPTNodePtr node) override;
    virtual void visit(IntegerLiteralPTNodePtr node) override;
    virtual void visit(BooleanLiteralPTNodePtr node) override;
    virtual void visit(BinaryOperatorPTNodePtr node) override;
    virtual void visit(VariableDeclarationPTNodePtr node) override;
    virtual void visit(VariableAssignmentPTNodePtr node) override;
    virtual void visit(FloorAssignmentPTNodePtr node) override;
    virtual void visit(BinaryExpressionPTNodePtr node) override;
    virtual void visit(NegativeExpressionPTNodePtr node) override;
    virtual void visit(PositiveExpressionPTNodePtr node) override;
    virtual void visit(NotExpressionPTNodePtr node) override;
    virtual void visit(IncrementExpressionPTNodePtr node) override;
    virtual void visit(DecrementExpressionPTNodePtr node) override;
    virtual void visit(FloorAccessPTNodePtr node) override;
    virtual void visit(ParenthesizedExpressionPTNodePtr node) override;
    virtual void visit(InvocationExpressionPTNodePtr node) override;
    virtual void visit(IfStatementPTNodePtr node) override;
    virtual void visit(WhileStatementPTNodePtr node) override;
    virtual void visit(ForStatementPTNodePtr node) override;
    virtual void visit(ReturnStatementPTNodePtr node) override;
    virtual void visit(FloorBoxInitStatementPTNodePtr node) override;
    virtual void visit(FloorMaxInitStatementPTNodePtr node) override;
    virtual void visit(EmptyStatementPTNodePtr node) override;
    virtual void visit(StatementBlockPTNodePtr node) override;
    virtual void visit(VariableDeclarationStatementPTNodePtr node) override;
    virtual void visit(VariableAssignmentStatementPTNodePtr node) override;
    virtual void visit(FloorAssignmentStatementPTNodePtr node) override;
    virtual void visit(InvocationStatementPTNodePtr node) override;
    virtual void visit(SubprocDefinitionPTNodePtr node) override;
    virtual void visit(FunctionDefinitionPTNodePtr node) override;
    virtual void visit(ImportDirectivePTNodePtr node) override;
    virtual void visit(CompilationUnitPTNodePtr node) override;

protected:
    std::vector<FormatterLine> _lines;
    int _indent_level = 0;

    std::string strip_comment(StringPtr comment);
    std::string get_text_from_token(lexer::TokenPtr token);

    virtual void create_line();
    virtual void create_line(lexer::TokenPtr token, bool add_space_after);
    virtual void append_line(lexer::TokenPtr token, bool add_space_before, bool add_space_after);
    virtual void create_line(const CommentGroup &comment_group);

    /**
     * @brief For preceding comments and newlines in token metadata, combine consecutive newlines, then add newlines and comments to the line buffer
     * @param token
     * @return true There are comments added
     * @return false There is no comments
     */
    virtual bool process_preceding_metadata(lexer::TokenPtr token);

    virtual void traverse_import_directives(const std::vector<ImportDirectivePTNodePtr> &imports);
    virtual void traverse_floor_inits(const std::vector<FloorBoxInitStatementPTNodePtr> &floor_inits);
    virtual void traverse_subroutines(const std::vector<AbstractSubroutinePTNodePtr> &subroutines);
    virtual void traverse_top_level_decls(const std::vector<VariableDeclarationStatementPTNodePtr> &tlds);
    virtual void traverse_statements(const std::vector<AbstractStatementPTNodePtr> &statements);
};

CLOSE_PARSER_NAMESPACE

#endif