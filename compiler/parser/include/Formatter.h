#ifndef FORMATTER_H
#define FORMATTER_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ParseTreeNode.h"
#include "ParseTreeNodeForward.h"
#include "ParseTreeNodeVisitor.h"
#include "HRLToken.h"
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
    std::string _content;
    CommentGroup _comment_group;
    int _indentation_level;
};

class ParseTreeNodeFormatterVisitor : public ParseTreeNodeVisitor {
public:
    virtual void format(CompilationUnitNodePtr node);

    virtual void visit(IdentifierNodePtr node) override;
    virtual void visit(IntegerLiteralNodePtr node) override;
    virtual void visit(BooleanLiteralNodePtr node) override;
    virtual void visit(BinaryOperatorNodePtr node) override;
    virtual void visit(VariableDeclarationNodePtr node) override;
    virtual void visit(VariableAssignmentNodePtr node) override;
    virtual void visit(FloorAssignmentNodePtr node) override;
    virtual void visit(BinaryExpressionNodePtr node) override;
    virtual void visit(NegativeExpressionNodePtr node) override;
    virtual void visit(PositiveExpressionNodePtr node) override;
    virtual void visit(NotExpressionNodePtr node) override;
    virtual void visit(IncrementExpressionNodePtr node) override;
    virtual void visit(DecrementExpressionNodePtr node) override;
    virtual void visit(FloorAccessNodePtr node) override;
    virtual void visit(ParenthesizedExpressionNodePtr node) override;
    virtual void visit(InvocationExpressionNodePtr node) override;
    virtual void visit(IfStatementNodePtr node) override;
    virtual void visit(WhileStatementNodePtr node) override;
    virtual void visit(ForStatementNodePtr node) override;
    virtual void visit(ReturnStatementNodePtr node) override;
    virtual void visit(FloorBoxInitStatementNodePtr node) override;
    virtual void visit(FloorMaxInitStatementNodePtr node) override;
    virtual void visit(EmptyStatementNodePtr node) override;
    virtual void visit(StatementBlockNodePtr node) override;
    virtual void visit(VariableDeclarationStatementNodePtr node) override;
    virtual void visit(VariableAssignmentStatementNodePtr node) override;
    virtual void visit(FloorAssignmentStatementNodePtr node) override;
    virtual void visit(InvocationStatementNodePtr node) override;
    virtual void visit(SubprocDefinitionNodePtr node) override;
    virtual void visit(FunctionDefinitionNodePtr node) override;
    virtual void visit(ImportDirectiveNodePtr node) override;
    virtual void visit(CompilationUnitNodePtr node) override;

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

    virtual void traverse_import_directives(const std::vector<ImportDirectiveNodePtr> &imports);
    virtual void traverse_floor_inits(const std::vector<FloorBoxInitStatementNodePtr> &floor_inits);
    virtual void traverse_subroutines(const std::vector<AbstractSubroutineNodePtr> &subroutines);
    virtual void traverse_top_level_decls(const std::vector<VariableDeclarationStatementNodePtr> &tlds);
    virtual void traverse_statements(const std::vector<AbstractStatementNodePtr> &statements);
};

CLOSE_PARSER_NAMESPACE

#endif