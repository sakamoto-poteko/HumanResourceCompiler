#ifndef PARSE_TREE_NODE_H
#define PARSE_TREE_NODE_H

#include <memory>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

#include "HRLToken.h"
#include "ParseTreeNodeForward.h"
#include "hrl_global.h"
#include "lexer_global.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

class ParseTreeNodeVisitor;

class ParseTreeNode : public std::enable_shared_from_this<ParseTreeNode> {
public:
    ParseTreeNode(int lineno, int colno)
        : _lineno(lineno)
        , _colno(colno)
    {
    }

    virtual ~ParseTreeNode() = default;

    virtual void accept(ParseTreeNodeVisitor *visitor) = 0;

    int lineno() const { return _lineno; }

    int colno() const { return _colno; }

    virtual const char *type() = 0;

    template <typename T>
    std::shared_ptr<T> shared_from_this_casted()
    {
        return std::static_pointer_cast<T>(shared_from_this());
    }

protected:
    int _lineno;
    int _colno;
};

class AbstractExpressionPTNode : public ParseTreeNode {
public:
    AbstractExpressionPTNode(int lineno, int colno)
        : ParseTreeNode(lineno, colno)
    {
    }
};

class AbstractUnaryExpressionPTNode : public AbstractExpressionPTNode {
public:
    AbstractUnaryExpressionPTNode(int lineno, int colno)
        : AbstractExpressionPTNode(lineno, colno)
    {
    }
};

class AbstractPrimaryExpressionPTNode : public AbstractUnaryExpressionPTNode {
public:
    AbstractPrimaryExpressionPTNode(int lineno, int colno)
        : AbstractUnaryExpressionPTNode(lineno, colno)
    {
    }
};

// Terminal Nodes
class IdentifierPTNode : public AbstractPrimaryExpressionPTNode {
public:
    explicit IdentifierPTNode(const lexer::IdentifierTokenPtr &token)
        : AbstractPrimaryExpressionPTNode(token->lineno(), token->colno())
        , _name(token->get_value())
        , _token(token)
    {
    }

    const char *type() override { return "Identifier"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    StringPtr get_value() const { return _name; }

    lexer::TokenPtr get_token() const { return _token; }

private:
    StringPtr _name;
    lexer::TokenPtr _token;
};

class IntegerLiteralPTNode : public AbstractPrimaryExpressionPTNode {
public:
    explicit IntegerLiteralPTNode(const lexer::IntegerTokenPtr &token)
        : AbstractPrimaryExpressionPTNode(token->lineno(), token->colno())
        , _value(token->get_value())
        , _is_char(token->get_is_char())
        , _token(token)
    {
    }

    const char *type() override { return "IntegerLiteral"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    int get_value() const { return _value; }

    int get_is_char() const { return _is_char; }

    lexer::TokenPtr get_token() const { return _token; }

private:
    int _value;
    bool _is_char;
    lexer::TokenPtr _token;
};

class BooleanLiteralPTNode : public AbstractPrimaryExpressionPTNode {
public:
    explicit BooleanLiteralPTNode(const lexer::BooleanTokenPtr &token)
        : AbstractPrimaryExpressionPTNode(token->lineno(), token->colno())
        , _value(token->get_value())
        , _token(token)
    {
    }

    const char *type() override { return "BooleanLiteral"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    bool get_value() const { return _value; }

    lexer::TokenPtr get_token() const { return _token; }

private:
    bool _value;
    lexer::TokenPtr _token;
};

class BinaryOperatorPTNode : public ParseTreeNode {
public:
    explicit BinaryOperatorPTNode(const lexer::TokenPtr &token)
        : ParseTreeNode(token->lineno(), token->colno())
        , _op(get_binary_operator_from_token_id(token->token_id()))
        , _token(token)
    {
    }

    enum BinaryOperator {
        GE,
        LE,
        EE,
        NE,
        GT,
        LT,

        AND,
        OR,

        ADD,
        SUB,
        MUL,
        DIV,
        MOD,
    };

    static const char *get_binary_operator_string(BinaryOperator op)
    {
        switch (op) {
        case GE:
            return "GE";
        case LE:
            return "LE";
        case EE:
            return "EE";
        case NE:
            return "NE";
        case GT:
            return "GT";
        case LT:
            return "LT";
        case AND:
            return "AND";
        case OR:
            return "OR";
        case ADD:
            return "ADD";
        case SUB:
            return "SUB";
        case MUL:
            return "MUL";
        case DIV:
            return "DIV";
        case MOD:
            return "MOD";
        default:
            return "Unknown";
        }
    }

    static BinaryOperator get_binary_operator_from_token_id(lexer::TokenId token_id)
    {
        switch (token_id) {
        case lexer::TokenId::GE:
            return GE;
        case lexer::TokenId::LE:
            return LE;
        case lexer::TokenId::EE:
            return EE;
        case lexer::TokenId::NE:
            return NE;
        case lexer::TokenId::GT:
            return GT;
        case lexer::TokenId::LT:
            return LT;
        case lexer::TokenId::AND:
            return AND;
        case lexer::TokenId::OR:
            return OR;
        case lexer::TokenId::ADD:
            return ADD;
        case lexer::TokenId::SUB:
            return SUB;
        case lexer::TokenId::MUL:
            return MUL;
        case lexer::TokenId::DIV:
            return DIV;
        case lexer::TokenId::MOD:
            return MOD;
        default:
            spdlog::critical("unknown lexer TokenId {}. {}", static_cast<int>(token_id), __PRETTY_FUNCTION__);
            throw;
        }
    }

    static int get_operator_precedence(lexer::TokenId token)
    {
        switch (token) {
        // case INVOCATION: // a()
        //     return 12;
        // case INDEX: // floor[]
        //     return 11;
        case lexer::ADDADD: // ++a
        case lexer::SUBSUB: // --a
            return 10;
        // unary add/sub
        // case lexer::ADD: // +a
        // case lexer::SUB: // -a
        //     return 9;
        case lexer::NOT: // !
            return 8;
        case lexer::MUL: // a*b
        case lexer::DIV: // a/b
        case lexer::MOD: // a%b
            return 7;
        case lexer::ADD: // a+b
        case lexer::SUB: // a-b
            return 6;
        case lexer::GE: // >=
        case lexer::LE: // <=
        case lexer::GT: // >
        case lexer::LT: // <
            return 5;
        case lexer::EE: // ==
        case lexer::NE: // !=
            return 4;
        case lexer::AND: // &
            return 3;
        case lexer::OR: // |
            return 2;
        case lexer::EQ: // =
            return 1;
        default:
            return -1; // Unknown or unsupported operator
        }
    }

    const char *type() override
    {
        return "BinaryOperator";
    }

    enum Associativity {
        LEFT_TO_RIGHT,
        RIGHT_TO_LEFT,
        NONE
    };

    static Associativity get_operator_associativity(lexer::TokenId token)
    {
        switch (token) {
        // case INVOCATION:    // a()
        // case FLOOR_ACCESS: // floor[]
        case lexer::MUL: // a*b
        case lexer::DIV: // a/b
        case lexer::MOD: // a%b
        case lexer::ADD: // a+b
        case lexer::SUB: // a-b
        case lexer::GE: // >=
        case lexer::LE: // <=
        case lexer::GT: // >
        case lexer::LT: // <
        case lexer::EE: // ==
        case lexer::NE: // !=
        case lexer::AND: // &
        case lexer::OR: // |
            return Associativity::LEFT_TO_RIGHT;
        case lexer::ADDADD: // ++a
        case lexer::SUBSUB: // --a
        // unary
        // case lexer::ADD:   // +a
        // case lexer::SUB:  // -a
        case lexer::NOT: // !
        case lexer::EQ: // =
            return Associativity::RIGHT_TO_LEFT;
        default:
            return Associativity::NONE; // Unknown or unsupported operator
        }
    }

    void accept(ParseTreeNodeVisitor *visitor) override;

    BinaryOperator get_op() const { return _op; }

    lexer::TokenPtr get_token() const { return _token; }

private:
    BinaryOperator _op;
    lexer::TokenPtr _token;
};

// Variable and Function Nodes
class VariableDeclarationPTNode : public ParseTreeNode {
public:
    VariableDeclarationPTNode(int lineno, int colno, IdentifierPTNodePtr var_name, AbstractExpressionPTNodePtr expr, lexer::TokenPtr let_token, lexer::TokenPtr equals)
        : ParseTreeNode(lineno, colno)
        , _var_name(std::move(var_name))
        , _expr(std::move(expr))
        , _let_token(std::move(let_token))
        , _equals(std::move(equals))
    {
    }

    const char *type() override { return "VariableDeclaration"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    IdentifierPTNodePtr get_var_name() const { return _var_name; }

    // the optional equals part
    AbstractExpressionPTNodePtr get_expr() const { return _expr; }

    lexer::TokenPtr get_let_token() const { return _let_token; }

    lexer::TokenPtr get_equals() const { return _equals; }

private:
    IdentifierPTNodePtr _var_name;
    AbstractExpressionPTNodePtr _expr;
    lexer::TokenPtr _let_token;
    lexer::TokenPtr _equals;
};

class VariableAssignmentPTNode : public ParseTreeNode {
public:
    VariableAssignmentPTNode(int lineno, int colno, IdentifierPTNodePtr var_name, AbstractExpressionPTNodePtr expr, lexer::TokenPtr eq)
        : ParseTreeNode(lineno, colno)
        , _var_name(std::move(var_name))
        , _expr(std::move(expr))
        , _eq(std::move(eq))
    {
    }

    const char *type() override { return "VariableAssignment"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    IdentifierPTNodePtr get_var_name() const { return _var_name; }

    AbstractExpressionPTNodePtr get_expr() const { return _expr; }

    lexer::TokenPtr get_eq() const { return _eq; }

private:
    IdentifierPTNodePtr _var_name;
    AbstractExpressionPTNodePtr _expr;
    lexer::TokenPtr _eq;
};

class FloorAssignmentPTNode : public ParseTreeNode {
public:
    FloorAssignmentPTNode(int lineno, int colno, FloorAccessPTNodePtr floor_access, AbstractExpressionPTNodePtr expr, lexer::TokenPtr eq)
        : ParseTreeNode(lineno, colno)
        , _floor_access(std::move(floor_access))
        , _expr(std::move(expr))
        , _eq(std::move(eq))
    {
    }

    const char *type() override { return "FloorAssignment"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    FloorAccessPTNodePtr get_floor_access() const { return _floor_access; }

    AbstractExpressionPTNodePtr get_expr() const { return _expr; }

    lexer::TokenPtr get_eq() const { return _eq; }

private:
    FloorAccessPTNodePtr _floor_access;
    AbstractExpressionPTNodePtr _expr;
    lexer::TokenPtr _eq;
};

// Expression Nodes
class BinaryExpressionPTNode : public AbstractExpressionPTNode {
public:
    BinaryExpressionPTNode(int lineno, int colno, AbstractExpressionPTNodePtr left, BinaryOperatorPTNodePtr op, AbstractExpressionPTNodePtr right)
        : AbstractExpressionPTNode(lineno, colno)
        , _left(std::move(left))
        , _op(std::move(op))
        , _right(std::move(right))
    {
    }

    const char *type() override { return "BinaryExpression"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    AbstractExpressionPTNodePtr get_left() const { return _left; }

    BinaryOperatorPTNodePtr get_op() const { return _op; }

    AbstractExpressionPTNodePtr get_right() const { return _right; }

private:
    AbstractExpressionPTNodePtr _left;
    BinaryOperatorPTNodePtr _op;
    AbstractExpressionPTNodePtr _right;
};

class NotExpressionPTNode : public AbstractUnaryExpressionPTNode {
public:
    NotExpressionPTNode(int lineno, int colno, AbstractPrimaryExpressionPTNodePtr expr, lexer::TokenPtr not_token)
        : AbstractUnaryExpressionPTNode(lineno, colno)
        , _expr(std::move(expr))
        , _not_token(std::move(not_token))
    {
    }

    const char *type() override { return "NotExpression"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    AbstractPrimaryExpressionPTNodePtr get_expr() const { return _expr; }

    lexer::TokenPtr get_not() const { return _not_token; }

private:
    AbstractPrimaryExpressionPTNodePtr _expr;
    lexer::TokenPtr _not_token;
};

class PositiveExpressionPTNode : public AbstractUnaryExpressionPTNode {
public:
    PositiveExpressionPTNode(int lineno, int colno, AbstractPrimaryExpressionPTNodePtr expr, lexer::TokenPtr plus_token)
        : AbstractUnaryExpressionPTNode(lineno, colno)
        , _expr(std::move(expr))
        , _plus_token(std::move(plus_token))
    {
    }

    const char *type() override { return "PositiveExpression"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    AbstractPrimaryExpressionPTNodePtr get_expr() const { return _expr; }

    lexer::TokenPtr get_plus_token() { return _plus_token; }

private:
    AbstractPrimaryExpressionPTNodePtr _expr;
    lexer::TokenPtr _plus_token;
};

class NegativeExpressionPTNode : public AbstractUnaryExpressionPTNode {
public:
    NegativeExpressionPTNode(int lineno, int colno, AbstractPrimaryExpressionPTNodePtr expr, lexer::TokenPtr minus_token)
        : AbstractUnaryExpressionPTNode(lineno, colno)
        , _expr(std::move(expr))
        , _minus_token(std::move(minus_token))
    {
    }

    const char *type() override { return "NegativeExpression"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    AbstractPrimaryExpressionPTNodePtr get_expr() const { return _expr; }

    lexer::TokenPtr get_minus_token() const { return _minus_token; }

private:
    AbstractPrimaryExpressionPTNodePtr _expr;
    lexer::TokenPtr _minus_token;
};

class IncrementExpressionPTNode : public AbstractUnaryExpressionPTNode {
public:
    IncrementExpressionPTNode(int lineno, int colno, IdentifierPTNodePtr var_name, lexer::TokenPtr increment_token)
        : AbstractUnaryExpressionPTNode(lineno, colno)
        , _var_name(std::move(var_name))
        , _increment_token(std::move(increment_token))
    {
    }

    const char *type() override { return "IncrementExpression"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    IdentifierPTNodePtr get_var_name() const { return _var_name; }

    lexer::TokenPtr get_increment_token() const { return _increment_token; }

private:
    IdentifierPTNodePtr _var_name;
    lexer::TokenPtr _increment_token;
};

class DecrementExpressionPTNode : public AbstractUnaryExpressionPTNode {
public:
    DecrementExpressionPTNode(int lineno, int colno, IdentifierPTNodePtr var_name, lexer::TokenPtr decrement_token)
        : AbstractUnaryExpressionPTNode(lineno, colno)
        , _var_name(std::move(var_name))
        , _decrement_token(std::move(decrement_token))
    {
    }

    const char *type() override { return "DecrementExpression"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    IdentifierPTNodePtr get_var_name() const { return _var_name; }

    lexer::TokenPtr get_decrement_token() const { return _decrement_token; }

private:
    IdentifierPTNodePtr _var_name;
    lexer::TokenPtr _decrement_token;
};

class FloorAccessPTNode : public AbstractPrimaryExpressionPTNode {
public:
    FloorAccessPTNode(int lineno, int colno, AbstractExpressionPTNodePtr index_expr, lexer::TokenPtr floor, lexer::TokenPtr open_bracket, lexer::TokenPtr close_bracket)
        : AbstractPrimaryExpressionPTNode(lineno, colno)
        , _index_expr(std::move(index_expr))
        , _floor(std::move(floor))
        , _open_bracket(std::move(open_bracket))
        , _close_bracket(std::move(close_bracket))
    {
    }

    const char *type() override { return "FloorAccess"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    AbstractExpressionPTNodePtr get_index_expr() const { return _index_expr; }

    lexer::TokenPtr get_floor() const { return _floor; }

    lexer::TokenPtr get_open_bracket() const { return _open_bracket; }

    lexer::TokenPtr get_close_bracket() const { return _close_bracket; }

private:
    AbstractExpressionPTNodePtr _index_expr;
    lexer::TokenPtr _floor;
    lexer::TokenPtr _open_bracket;
    lexer::TokenPtr _close_bracket;
};

class ParenthesizedExpressionPTNode : public AbstractPrimaryExpressionPTNode {
public:
    ParenthesizedExpressionPTNode(int lineno, int colno, AbstractExpressionPTNodePtr expr, lexer::TokenPtr open_paren, lexer::TokenPtr close_paren)
        : AbstractPrimaryExpressionPTNode(lineno, colno)
        , _expr(std::move(expr))
        , _open_paren(std::move(open_paren))
        , _close_paren(std::move(close_paren))
    {
    }

    const char *type() override { return "ParenthesizedExpression"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    AbstractExpressionPTNodePtr get_expr() const { return _expr; }

    lexer::TokenPtr get_open_paren() const { return _open_paren; }

    lexer::TokenPtr get_close_paren() const { return _close_paren; }

private:
    AbstractExpressionPTNodePtr _expr;
    lexer::TokenPtr _open_paren;
    lexer::TokenPtr _close_paren;
};

class InvocationExpressionPTNode : public AbstractPrimaryExpressionPTNode {
public:
    InvocationExpressionPTNode(int lineno, int colno, IdentifierPTNodePtr func_name, AbstractExpressionPTNodePtr arg, lexer::TokenPtr open_paren, lexer::TokenPtr close_paren)
        : AbstractPrimaryExpressionPTNode(lineno, colno)
        , _func_name(std::move(func_name))
        , _arg(std::move(arg))
        , _open_paren(std::move(open_paren))
        , _close_paren(std::move(close_paren))
    {
    }

    const char *type() override { return "InvocationExpression"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    IdentifierPTNodePtr get_func_name() const { return _func_name; }

    AbstractExpressionPTNodePtr get_arg() const { return _arg; }

    lexer::TokenPtr get_open_paren() const { return _open_paren; }

    lexer::TokenPtr get_close_paren() const { return _close_paren; }

private:
    IdentifierPTNodePtr _func_name;
    AbstractExpressionPTNodePtr _arg;
    lexer::TokenPtr _open_paren;
    lexer::TokenPtr _close_paren;
};

// Statement Nodes
class AbstractStatementPTNode : public ParseTreeNode {
public:
    AbstractStatementPTNode(int lineno, int colno)
        : ParseTreeNode(lineno, colno)
    {
    }
};

class AbstractEmbeddedStatementPTNode : public AbstractStatementPTNode {
public:
    AbstractEmbeddedStatementPTNode(int lineno, int colno)
        : AbstractStatementPTNode(lineno, colno)
    {
    }
};

class AbstractSelectionStatementPTNode : public AbstractEmbeddedStatementPTNode {
public:
    AbstractSelectionStatementPTNode(int lineno, int colno)
        : AbstractEmbeddedStatementPTNode(lineno, colno)
    {
    }
};

class AbstractIterationStatementPTNode : public AbstractEmbeddedStatementPTNode {
public:
    AbstractIterationStatementPTNode(int lineno, int colno)
        : AbstractEmbeddedStatementPTNode(lineno, colno)
    {
    }
};

class IfStatementPTNode : public AbstractSelectionStatementPTNode {
public:
    IfStatementPTNode(int lineno, int colno, AbstractExpressionPTNodePtr condition, AbstractEmbeddedStatementPTNodePtr then_stmt, AbstractEmbeddedStatementPTNodePtr else_stmt,
        lexer::TokenPtr if_token, lexer::TokenPtr cond_open_paren, lexer::TokenPtr cond_close_paren, lexer::TokenPtr else_token)
        : AbstractSelectionStatementPTNode(lineno, colno)
        , _condition(std::move(condition))
        , _then_stmt(std::move(then_stmt))
        , _else_stmt(std::move(else_stmt))
        , _if_token(std::move(if_token))
        , _cond_open_paren(std::move(cond_open_paren))
        , _cond_close_paren(std::move(cond_close_paren))
        , _else_token(std::move(else_token))
    {
    }

    const char *type() override { return "IfStatement"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    AbstractExpressionPTNodePtr get_condition() const { return _condition; }

    AbstractEmbeddedStatementPTNodePtr get_then_stmt() const { return _then_stmt; }

    AbstractEmbeddedStatementPTNodePtr get_else_stmt() const { return _else_stmt; }

    lexer::TokenPtr get_if_token() const { return _if_token; }

    lexer::TokenPtr get_cond_open_paren() const { return _cond_open_paren; }

    lexer::TokenPtr get_cond_close_paren() const { return _cond_close_paren; }

    lexer::TokenPtr get_else_token() const { return _else_token; }

private:
    AbstractExpressionPTNodePtr _condition;
    AbstractEmbeddedStatementPTNodePtr _then_stmt;
    AbstractEmbeddedStatementPTNodePtr _else_stmt;

    lexer::TokenPtr _if_token;
    lexer::TokenPtr _cond_open_paren;
    lexer::TokenPtr _cond_close_paren;
    lexer::TokenPtr _else_token;
};

class WhileStatementPTNode : public AbstractIterationStatementPTNode {
public:
    WhileStatementPTNode(int lineno, int colno, AbstractExpressionPTNodePtr condition, AbstractEmbeddedStatementPTNodePtr body,
        lexer::TokenPtr while_token, lexer::TokenPtr while_open_paren, lexer::TokenPtr while_close_paren)
        : AbstractIterationStatementPTNode(lineno, colno)
        , _condition(std::move(condition))
        , _body(std::move(body))
        , _while_token(std::move(while_token))
        , _while_open_paren(std::move(while_open_paren))
        , _while_close_paren(std::move(while_close_paren))
    {
    }

    const char *type() override { return "WhileStatement"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    AbstractExpressionPTNodePtr get_condition() const { return _condition; }

    AbstractEmbeddedStatementPTNodePtr get_body() const { return _body; }

    lexer::TokenPtr get_while_token() const { return _while_token; }

    lexer::TokenPtr get_while_open_paren() const { return _while_open_paren; }

    lexer::TokenPtr get_while_close_paren() const { return _while_close_paren; }

private:
    AbstractExpressionPTNodePtr _condition;
    AbstractEmbeddedStatementPTNodePtr _body;
    lexer::TokenPtr _while_token;
    lexer::TokenPtr _while_open_paren;
    lexer::TokenPtr _while_close_paren;
};

class ForStatementPTNode : public AbstractIterationStatementPTNode {
public:
    ForStatementPTNode(
        int lineno, int colno,
        VariableAssignmentPTNodePtr init_stmt,
        AbstractExpressionPTNodePtr condition,
        AbstractExpressionPTNodePtr update_stmt,
        AbstractEmbeddedStatementPTNodePtr body,
        lexer::TokenPtr for_token, lexer::TokenPtr open_paren, lexer::TokenPtr comma1, lexer::TokenPtr comma2, lexer::TokenPtr close_paren)
        : AbstractIterationStatementPTNode(lineno, colno)
        , _init_stmt_assignment(std::move(init_stmt))
        , _condition(std::move(condition))
        , _update_stmt(std::move(update_stmt))
        , _body(std::move(body))
        , _for_token(std::move(for_token))
        , _open_paren(std::move(open_paren))
        , _comma1(std::move(comma1))
        , _comma2(std::move(comma2))
        , _close_paren(std::move(close_paren))

    {
    }

    ForStatementPTNode(
        int lineno, int colno,
        VariableDeclarationPTNodePtr init_stmt,
        AbstractExpressionPTNodePtr condition,
        AbstractExpressionPTNodePtr update_stmt,
        AbstractEmbeddedStatementPTNodePtr body,
        lexer::TokenPtr for_token, lexer::TokenPtr open_paren, lexer::TokenPtr comma1, lexer::TokenPtr comma2, lexer::TokenPtr close_paren)
        : AbstractIterationStatementPTNode(lineno, colno)
        , _init_stmt_declaration(std::move(init_stmt))
        , _condition(std::move(condition))
        , _update_stmt(std::move(update_stmt))
        , _body(std::move(body))
        , _for_token(std::move(for_token))
        , _open_paren(std::move(open_paren))
        , _comma1(std::move(comma1))
        , _comma2(std::move(comma2))
        , _close_paren(std::move(close_paren))
    {
    }

    const char *type() override { return "ForStatement"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    ParseTreePTNodePtr get_init_stmt() const
    {
        if (_init_stmt_assignment) {
            return _init_stmt_assignment;
        } else {
            return _init_stmt_declaration;
        }
    }

    bool is_init_stmt_decl() { return _init_stmt_declaration.operator bool(); }

    AbstractExpressionPTNodePtr get_condition() const { return _condition; }

    AbstractExpressionPTNodePtr get_update_stmt() const { return _update_stmt; }

    AbstractEmbeddedStatementPTNodePtr get_body() const { return _body; }

    lexer::TokenPtr get_for_token() const { return _for_token; }

    lexer::TokenPtr get_open_paren() const { return _open_paren; }

    lexer::TokenPtr get_comma1() const { return _comma1; }

    lexer::TokenPtr get_comma2() const { return _comma2; }

    lexer::TokenPtr get_close_paren() const { return _close_paren; }

private:
    VariableAssignmentPTNodePtr _init_stmt_assignment;
    VariableDeclarationPTNodePtr _init_stmt_declaration;
    AbstractExpressionPTNodePtr _condition;
    AbstractExpressionPTNodePtr _update_stmt;
    AbstractEmbeddedStatementPTNodePtr _body;
    lexer::TokenPtr _for_token;
    lexer::TokenPtr _open_paren;
    lexer::TokenPtr _comma1;
    lexer::TokenPtr _comma2;
    lexer::TokenPtr _close_paren;
};

class BreakContinueStatementPTNode : public AbstractEmbeddedStatementPTNode {
public:
    BreakContinueStatementPTNode(int lineno, int colno, bool is_break)
        : AbstractEmbeddedStatementPTNode(lineno, colno)
        , _is_break(is_break)
    {
    }

    const char *type() override { return _is_break ? "BreakStatement" : "ContinueStatement"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    bool is_break() const { return _is_break; }

    bool is_continue() const { return !_is_break; }

private:
    bool _is_break;
};

class ReturnStatementPTNode : public AbstractEmbeddedStatementPTNode {
public:
    ReturnStatementPTNode(int lineno, int colno, AbstractExpressionPTNodePtr expr, lexer::TokenPtr return_token, lexer::TokenPtr semicolon)
        : AbstractEmbeddedStatementPTNode(lineno, colno)
        , _expr(std::move(expr))
        , _return_token(std::move(return_token))
        , _semicolon(std::move(semicolon))
    {
    }

    const char *type() override { return "ReturnStatement"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    AbstractExpressionPTNodePtr get_expr() const { return _expr; }

    lexer::TokenPtr get_return_token() const { return _return_token; }

    lexer::TokenPtr get_semicolon() const { return _semicolon; }

private:
    AbstractExpressionPTNodePtr _expr;
    lexer::TokenPtr _return_token;
    lexer::TokenPtr _semicolon;
};

class InvocationStatementPTNode : public AbstractEmbeddedStatementPTNode {
public:
    InvocationStatementPTNode(int lineno, int colno, InvocationExpressionPTNodePtr expr, lexer::TokenPtr semicolon)
        : AbstractEmbeddedStatementPTNode(lineno, colno)
        , _expr(std::move(expr))
        , _semicolon(std::move(semicolon))
    {
    }

    const char *type() override { return "InvocationStatement"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    InvocationExpressionPTNodePtr get_expr() const { return _expr; }

    lexer::TokenPtr get_semicolon() const { return _semicolon; }

private:
    InvocationExpressionPTNodePtr _expr;
    lexer::TokenPtr _semicolon;
};

class FloorAssignmentStatementPTNode : public AbstractEmbeddedStatementPTNode {
public:
    FloorAssignmentStatementPTNode(int lineno, int colno, FloorAssignmentPTNodePtr assignment, lexer::TokenPtr semicolon)
        : AbstractEmbeddedStatementPTNode(lineno, colno)
        , _assignment(std::move(assignment))
        , _semicolon(std::move(semicolon))
    {
    }

    const char *type() override { return "FloorAssignmentStatement"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    FloorAssignmentPTNodePtr get_floor_assignment() const { return _assignment; }

    lexer::TokenPtr get_semicolon() const { return _semicolon; }

private:
    FloorAssignmentPTNodePtr _assignment;
    lexer::TokenPtr _semicolon;
};

class VariableAssignmentStatementPTNode : public AbstractEmbeddedStatementPTNode {
public:
    VariableAssignmentStatementPTNode(int lineno, int colno, VariableAssignmentPTNodePtr assignment, lexer::TokenPtr semicolon)
        : AbstractEmbeddedStatementPTNode(lineno, colno)
        , _assignment(std::move(assignment))
        , _semicolon(std::move(semicolon))
    {
    }

    const char *type() override { return "VariableAssignmentStatement"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    VariableAssignmentPTNodePtr get_variable_assignment() const { return _assignment; }

    lexer::TokenPtr get_semicolon() const { return _semicolon; }

private:
    VariableAssignmentPTNodePtr _assignment;
    lexer::TokenPtr _semicolon;
};

class VariableDeclarationStatementPTNode : public AbstractStatementPTNode {
public:
    VariableDeclarationStatementPTNode(int lineno, int colno, VariableDeclarationPTNodePtr decl, lexer::TokenPtr semicolon)
        : AbstractStatementPTNode(lineno, colno)
        , decl(std::move(decl))
        , _semicolon(std::move(semicolon))
    {
    }

    const char *type() override { return "VariableDeclarationStatement"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    VariableDeclarationPTNodePtr get_variable_decl() const { return decl; }

    lexer::TokenPtr get_semicolon() const { return _semicolon; }

private:
    VariableDeclarationPTNodePtr decl;
    lexer::TokenPtr _semicolon;
};

class FloorBoxInitStatementPTNode : public AbstractStatementPTNode {
public:
    FloorBoxInitStatementPTNode(int lineno, int colno, IntegerLiteralPTNodePtr index, IntegerLiteralPTNodePtr value,
        lexer::TokenPtr init_token, lexer::TokenPtr floor_token,
        lexer::TokenPtr open_bracket, lexer::TokenPtr floor_index_token, lexer::TokenPtr close_bracket,
        lexer::TokenPtr equal_token, lexer::TokenPtr value_token, lexer::TokenPtr semicolon)
        : AbstractStatementPTNode(lineno, colno)
        , _index(std::move(index))
        , _value(std::move(value))
        , _init_token(std::move(init_token))
        , _floor_token(std::move(floor_token))
        , _open_bracket(std::move(open_bracket))
        , _floor_index_token(std::move(floor_index_token))
        , _close_bracket(std::move(close_bracket))
        , _equal_token(std::move(equal_token))
        , _value_token(std::move(value_token))
        , _semicolon(std::move(semicolon))
    {
    }

    const char *type() override { return "FloorBoxInitStatement"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    IntegerLiteralPTNodePtr get_index() const { return _index; }

    IntegerLiteralPTNodePtr get_value() const { return _value; }

    lexer::TokenPtr get_init_token() const { return _init_token; }

    lexer::TokenPtr get_floor_token() const { return _floor_token; }

    lexer::TokenPtr get_open_bracket() const { return _open_bracket; }

    lexer::TokenPtr get_floor_index() const { return _floor_index_token; }

    lexer::TokenPtr get_close_bracket() const { return _close_bracket; }

    lexer::TokenPtr get_equal_token() const { return _equal_token; }

    lexer::TokenPtr get_value_token() const { return _value_token; }

    lexer::TokenPtr get_semicolon() const { return _semicolon; }

private:
    IntegerLiteralPTNodePtr _index;
    IntegerLiteralPTNodePtr _value;
    lexer::TokenPtr _init_token;
    lexer::TokenPtr _floor_token;
    lexer::TokenPtr _open_bracket;
    lexer::TokenPtr _floor_index_token;
    lexer::TokenPtr _close_bracket;
    lexer::TokenPtr _equal_token;
    lexer::TokenPtr _value_token;
    lexer::TokenPtr _semicolon;
};

class FloorMaxInitStatementPTNode : public AbstractStatementPTNode {
public:
    FloorMaxInitStatementPTNode(int lineno, int colno, IntegerLiteralPTNodePtr value,
        lexer::TokenPtr init_token, lexer::TokenPtr floor_max_token, lexer::TokenPtr equals, lexer::TokenPtr value_token, lexer::TokenPtr semicolon)
        : AbstractStatementPTNode(lineno, colno)
        , _value(std::move(value))
        , _init_token(std::move(init_token))
        , _floor_max_token(std::move(floor_max_token))
        , _equals(std::move(equals))
        , _value_token(std::move(value_token))
        , _semicolon(std::move(semicolon))
    {
    }

    const char *type() override { return "FloorMaxInitStatement"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    IntegerLiteralPTNodePtr get_value() const { return _value; }

    lexer::TokenPtr get_init_token() const { return _init_token; }

    lexer::TokenPtr get_floor_max_token() const { return _floor_max_token; }

    lexer::TokenPtr get_value_token() const { return _value_token; }

    lexer::TokenPtr get_equals() const { return _equals; }

    lexer::TokenPtr get_semicolon() const { return _semicolon; }

private:
    IntegerLiteralPTNodePtr _value;
    lexer::TokenPtr _init_token;
    lexer::TokenPtr _floor_max_token;
    lexer::TokenPtr _equals;
    lexer::TokenPtr _value_token;
    lexer::TokenPtr _semicolon;
};

class EmptyStatementPTNode : public AbstractEmbeddedStatementPTNode {
public:
    EmptyStatementPTNode(int lineno, int colno, lexer::TokenPtr semicolon)
        : AbstractEmbeddedStatementPTNode(lineno, colno)
        , _semicolon(std::move(semicolon))
    {
    }

    const char *type() override { return "EmptyStatement"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    lexer::TokenPtr get_semicolon() const { return _semicolon; }

private:
    lexer::TokenPtr _semicolon;
};

class StatementBlockPTNode : public AbstractEmbeddedStatementPTNode {
public:
    StatementBlockPTNode(int lineno, int colno, const std::vector<AbstractStatementPTNodePtr> &statements, lexer::TokenPtr open_brace, lexer::TokenPtr close_brace)
        : AbstractEmbeddedStatementPTNode(lineno, colno)
        , _statements(statements)
        , _open_brace(std::move(open_brace))
        , _close_brace(std::move(close_brace))
    {
    }

    const char *type() override { return "StatementBlock"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    const std::vector<AbstractStatementPTNodePtr> &get_statements() const { return _statements; }

    lexer::TokenPtr get_open_brace() const { return _open_brace; }

    lexer::TokenPtr get_close_brace() const { return _close_brace; }

private:
    std::vector<AbstractStatementPTNodePtr> _statements;
    lexer::TokenPtr _open_brace;
    lexer::TokenPtr _close_brace;
};

// Function and Subprocedure Nodes
class AbstractSubroutinePTNode : public ParseTreeNode {
public:
    AbstractSubroutinePTNode(int lineno, int colno, IdentifierPTNodePtr function_name, IdentifierPTNodePtr formal_parameter, StatementBlockPTNodePtr body)
        : ParseTreeNode(lineno, colno)
        , _function_name(std::move(function_name))
        , _formal_parameter(std::move(formal_parameter))
        , _body(std::move(body))
    {
    }

    IdentifierPTNodePtr get_function_name() const { return _function_name; }

    IdentifierPTNodePtr get_formal_parameter() const { return _formal_parameter; }

    StatementBlockPTNodePtr get_body() const { return _body; }

protected:
    IdentifierPTNodePtr _function_name;
    IdentifierPTNodePtr _formal_parameter;
    StatementBlockPTNodePtr _body;
};

class FunctionDefinitionPTNode : public AbstractSubroutinePTNode {
public:
    FunctionDefinitionPTNode(int lineno, int colno, IdentifierPTNodePtr function_name, IdentifierPTNodePtr formal_parameter, StatementBlockPTNodePtr body, lexer::TokenPtr function_token, lexer::TokenPtr open_brace, lexer::TokenPtr close_brace)
        : AbstractSubroutinePTNode(lineno, colno, std::move(function_name), std::move(formal_parameter), std::move(body))
        , _function_token(std::move(function_token))
        , _open_brace(std::move(open_brace))
        , _close_brace(std::move(close_brace))
    {
    }

    const char *type() override { return "AbstractSubroutineNode"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    lexer::TokenPtr get_function_token() const { return _function_token; }

    lexer::TokenPtr get_open_brace() const { return _open_brace; }

    lexer::TokenPtr get_close_brace() const { return _close_brace; }

private:
    lexer::TokenPtr _function_token;
    lexer::TokenPtr _open_brace;
    lexer::TokenPtr _close_brace;
};

class SubprocDefinitionPTNode : public AbstractSubroutinePTNode {
public:
    SubprocDefinitionPTNode(int lineno, int colno, IdentifierPTNodePtr function_name, IdentifierPTNodePtr formal_parameter, StatementBlockPTNodePtr body,
        lexer::TokenPtr sub_token, lexer::TokenPtr open_brace, lexer::TokenPtr close_brace)
        : AbstractSubroutinePTNode(lineno, colno, std::move(function_name), std::move(formal_parameter), std::move(body))
        , _sub_token(std::move(sub_token))
        , _open_brace(std::move(open_brace))
        , _close_brace(std::move(close_brace))
    {
    }

    const char *type() override { return "SubprocDefinition"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    lexer::TokenPtr get_sub_token() const { return _sub_token; }

    lexer::TokenPtr get_open_brace() const { return _open_brace; }

    lexer::TokenPtr get_close_brace() const { return _close_brace; }

private:
    lexer::TokenPtr _sub_token;
    lexer::TokenPtr _open_brace;
    lexer::TokenPtr _close_brace;
};

class ImportDirectivePTNode : public ParseTreeNode {
public:
    ImportDirectivePTNode(int lineno, int colno, IdentifierPTNodePtr module_name, lexer::TokenPtr import_token, lexer::TokenPtr semicolon)
        : ParseTreeNode(lineno, colno)
        , _module_name(std::move(module_name))
        , _import_token(std::move(import_token))
        , _semicolon(std::move(semicolon))
    {
    }

    const char *type() override { return "ImportDirective"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    IdentifierPTNodePtr get_module_name() const { return _module_name; }

    lexer::TokenPtr get_import_token() const { return _import_token; }

    lexer::TokenPtr get_semicolon() const { return _semicolon; }

private:
    IdentifierPTNodePtr _module_name;
    lexer::TokenPtr _import_token;
    lexer::TokenPtr _semicolon;
};

// Compilation Unit Node
class CompilationUnitPTNode : public ParseTreeNode {
public:
    CompilationUnitPTNode(
        int lineno, int colno,
        std::vector<ImportDirectivePTNodePtr> imports,
        std::vector<FloorBoxInitStatementPTNodePtr> floor_inits,
        FloorMaxInitStatementPTNodePtr floor_max,
        std::vector<VariableDeclarationStatementPTNodePtr> top_level_decls,
        std::vector<AbstractSubroutinePTNodePtr> subroutines)
        : ParseTreeNode(lineno, colno)
        , _imports(std::move(imports))
        , _floor_inits(std::move(floor_inits))
        , _floor_max(std::move(floor_max))
        , _top_level_decls(std::move(top_level_decls))
        , _subroutines(std::move(subroutines))
    {
    }

    const char *type() override { return "CompilationUnit"; }

    void accept(ParseTreeNodeVisitor *visitor) override;

    const std::vector<ImportDirectivePTNodePtr> &get_imports() const { return _imports; }

    const std::vector<FloorBoxInitStatementPTNodePtr> &get_floor_inits() const { return _floor_inits; }

    const FloorMaxInitStatementPTNodePtr &get_floor_max() const { return _floor_max; }

    const std::vector<VariableDeclarationStatementPTNodePtr> &get_top_level_decls() const { return _top_level_decls; }

    const std::vector<AbstractSubroutinePTNodePtr> &get_subroutines() const { return _subroutines; }

private:
    std::vector<ImportDirectivePTNodePtr> _imports;
    std::vector<FloorBoxInitStatementPTNodePtr> _floor_inits;
    FloorMaxInitStatementPTNodePtr _floor_max;
    std::vector<VariableDeclarationStatementPTNodePtr> _top_level_decls;
    std::vector<AbstractSubroutinePTNodePtr> _subroutines;
};

template <typename T>
concept ConvertibleToParseTreeNodePtr = requires {
    typename T::element_type;
    requires std::convertible_to<T, ParseTreePTNodePtr> && std::is_same_v<T, std::shared_ptr<typename T::element_type>>;
};

CLOSE_PARSER_NAMESPACE

#endif