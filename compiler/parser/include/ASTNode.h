#ifndef ASTNODE_H
#define ASTNODE_H

#include <memory>
#include <vector>

#include "ASTNodeForward.h"
#include "HRLToken.h"
#include "hrl_global.h"
#include "lexer_global.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

class ASTNodeVisitor;

class ASTNode : public std::enable_shared_from_this<ASTNode> {
public:
    ASTNode(int lineno, int colno)
        : _lineno(lineno)
        , _colno(colno)
    {
    }

    virtual ~ASTNode() = default;

    virtual void accept(ASTNodeVisitor *visitor) = 0;

    int lineno()
    {
        return _lineno;
    }

    int colno()
    {
        return _colno;
    }

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

class AbstractExpressionNode : public ASTNode {
public:
    AbstractExpressionNode(int lineno, int colno)
        : ASTNode(lineno, colno)
    {
    }
};

class AbstractUnaryExpressionNode : public AbstractExpressionNode {
public:
    AbstractUnaryExpressionNode(int lineno, int colno)
        : AbstractExpressionNode(lineno, colno)
    {
    }
};

class AbstractPrimaryExpressionNode : public AbstractUnaryExpressionNode {
public:
    AbstractPrimaryExpressionNode(int lineno, int colno)
        : AbstractUnaryExpressionNode(lineno, colno)
    {
    }
};

// Terminal Nodes
class IdentifierNode : public AbstractPrimaryExpressionNode {
public:
    IdentifierNode(const lexer::IdentifierTokenPtr &token)
        : AbstractPrimaryExpressionNode(token->lineno(), token->colno())
        , _name(token->get_value())
    {
    }

    const char *type() override { return "Identifier"; }

    void accept(ASTNodeVisitor *visitor) override;

    const StringPtr get_value() const { return _name; }

private:
    StringPtr _name;
};

class IntegerLiteralNode : public AbstractPrimaryExpressionNode {
public:
    IntegerLiteralNode(const lexer::IntegerTokenPtr &token)
        : AbstractPrimaryExpressionNode(token->lineno(), token->colno())
        , _value(token->get_value())
    {
    }

    const char *type() override { return "IntegerLiteral"; }

    void accept(ASTNodeVisitor *visitor) override;

    int get_value() const { return _value; }

private:
    int _value;
};

class BooleanLiteralNode : public AbstractPrimaryExpressionNode {
public:
    BooleanLiteralNode(const lexer::BooleanTokenPtr &token)
        : AbstractPrimaryExpressionNode(token->lineno(), token->colno())
        , _value(token->get_value())
    {
    }

    const char *type() override { return "BooleanLiteral"; }

    void accept(ASTNodeVisitor *visitor) override;

    bool get_value() const { return _value; }

private:
    bool _value;
};

class BinaryOperatorNode : public ASTNode {
public:
    BinaryOperatorNode(const lexer::TokenPtr &token)
        : ASTNode(token->lineno(), token->colno())
        , _op(get_binary_operator_from_token_id(token->token_id()))
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
        NOT,

        ADD,
        SUB,
        MUL,
        DIV,
        MOD,

        EQ,
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
        case NOT:
            return "NOT";
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
        case EQ:
            return "EQ";
        default:
            return "Unknown";
        }
    }

    static BinaryOperator get_binary_operator_from_token_id(lexer::TokenId token_id)
    {
        BinaryOperator op;
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
        case lexer::TokenId::NOT:
            return NOT;
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
            // FIXME: error handling
            throw;
        }
    }

    static inline int get_operator_precedence(lexer::TokenId token)
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

    static inline Associativity get_operator_associativity(lexer::TokenId token)
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

    void accept(ASTNodeVisitor *visitor) override;

    BinaryOperator get_op() const { return _op; }

private:
    BinaryOperator _op;
};

// Variable and Function Nodes
class VariableDeclarationNode : public ASTNode {
public:
    VariableDeclarationNode(int lineno, int colno, IdentifierNodePtr var_name, AbstractExpressionNodePtr expr = nullptr)
        : ASTNode(lineno, colno)
        , _var_name(var_name)
        , _expr(expr)
    {
    }

    const char *type() override { return "VariableDeclaration"; }

    void accept(ASTNodeVisitor *visitor) override;

    IdentifierNodePtr get_var_name() const { return _var_name; }

    // the optional equals part
    AbstractExpressionNodePtr get_expr() const { return _expr; }

private:
    IdentifierNodePtr _var_name;
    AbstractExpressionNodePtr _expr;
};

class VariableAssignmentNode : public ASTNode {
public:
    VariableAssignmentNode(int lineno, int colno, IdentifierNodePtr var_name, AbstractExpressionNodePtr expr)
        : ASTNode(lineno, colno)
        , _var_name(var_name)
        , _expr(expr)
    {
    }

    const char *type() override { return "VariableAssignment"; }

    void accept(ASTNodeVisitor *visitor) override;

    IdentifierNodePtr get_var_name() const { return _var_name; }

    AbstractExpressionNodePtr get_expr() const { return _expr; }

private:
    IdentifierNodePtr _var_name;
    AbstractExpressionNodePtr _expr;
};

class FloorAssignmentNode : public ASTNode {
public:
    FloorAssignmentNode(int lineno, int colno, FloorAccessNodePtr floor_access, AbstractExpressionNodePtr expr)
        : ASTNode(lineno, colno)
        , _floor_access(floor_access)
        , _expr(expr)
    {
    }

    const char *type() override { return "FloorAssignment"; }

    void accept(ASTNodeVisitor *visitor) override;

    FloorAccessNodePtr get_floor_access() const { return _floor_access; }

    AbstractExpressionNodePtr get_expr() const { return _expr; }

private:
    FloorAccessNodePtr _floor_access;
    AbstractExpressionNodePtr _expr;
};

// Expression Nodes
class BinaryExpressionNode : public AbstractExpressionNode {
public:
    BinaryExpressionNode(int lineno, int colno, AbstractExpressionNodePtr left, BinaryOperatorNodePtr op, AbstractExpressionNodePtr right)
        : AbstractExpressionNode(lineno, colno)
        , _left(left)
        , _op(op)
        , _right(right)
    {
    }

    const char *type() override { return "BinaryExpression"; }

    void accept(ASTNodeVisitor *visitor) override;

    AbstractExpressionNodePtr get_left() const { return _left; }

    BinaryOperatorNodePtr get_op() const { return _op; }

    AbstractExpressionNodePtr get_right() const { return _right; }

private:
    AbstractExpressionNodePtr _left;
    BinaryOperatorNodePtr _op;
    AbstractExpressionNodePtr _right;
};

class NotExpressionNode : public AbstractUnaryExpressionNode {
public:
    NotExpressionNode(int lineno, int colno, AbstractPrimaryExpressionNodePtr expr)
        : AbstractUnaryExpressionNode(lineno, colno)
        , _expr(expr)
    {
    }

    const char *type() override { return "NotExpression"; }

    void accept(ASTNodeVisitor *visitor) override;

    AbstractPrimaryExpressionNodePtr get_expr() const { return _expr; }

private:
    AbstractPrimaryExpressionNodePtr _expr;
};

class PositiveExpressionNode : public AbstractUnaryExpressionNode {
public:
    PositiveExpressionNode(int lineno, int colno, AbstractPrimaryExpressionNodePtr expr)
        : AbstractUnaryExpressionNode(lineno, colno)
        , _expr(expr)
    {
    }

    const char *type() override { return "PositiveExpression"; }

    void accept(ASTNodeVisitor *visitor) override;

    AbstractPrimaryExpressionNodePtr get_expr() const { return _expr; }

private:
    AbstractPrimaryExpressionNodePtr _expr;
};

class NegativeExpressionNode : public AbstractUnaryExpressionNode {
public:
    NegativeExpressionNode(int lineno, int colno, AbstractPrimaryExpressionNodePtr expr)
        : AbstractUnaryExpressionNode(lineno, colno)
        , _expr(expr)
    {
    }

    const char *type() override { return "NegativeExpression"; }

    void accept(ASTNodeVisitor *visitor) override;

    AbstractPrimaryExpressionNodePtr get_expr() const { return _expr; }

private:
    AbstractPrimaryExpressionNodePtr _expr;
};

class IncrementExpressionNode : public AbstractUnaryExpressionNode {
public:
    IncrementExpressionNode(int lineno, int colno, IdentifierNodePtr var_name)
        : AbstractUnaryExpressionNode(lineno, colno)
        , _var_name(var_name)
    {
    }

    const char *type() override { return "IncrementExpression"; }

    void accept(ASTNodeVisitor *visitor) override;

    IdentifierNodePtr get_var_name() const { return _var_name; }

private:
    IdentifierNodePtr _var_name;
};

class DecrementExpressionNode : public AbstractUnaryExpressionNode {
public:
    DecrementExpressionNode(int lineno, int colno, IdentifierNodePtr var_name)
        : AbstractUnaryExpressionNode(lineno, colno)
        , _var_name(var_name)
    {
    }

    const char *type() override { return "DecrementExpression"; }

    void accept(ASTNodeVisitor *visitor) override;

    IdentifierNodePtr get_var_name() const { return _var_name; }

private:
    IdentifierNodePtr _var_name;
};

class FloorAccessNode : public AbstractPrimaryExpressionNode {
public:
    FloorAccessNode(int lineno, int colno, AbstractExpressionNodePtr index_expr)
        : AbstractPrimaryExpressionNode(lineno, colno)
        , _index_expr(index_expr)
    {
    }

    const char *type() override { return "FloorAccess"; }

    void accept(ASTNodeVisitor *visitor) override;

    AbstractExpressionNodePtr get_index_expr() const { return _index_expr; }

private:
    AbstractExpressionNodePtr _index_expr;
};

class ParenthesizedExpressionNode : public AbstractPrimaryExpressionNode {
public:
    ParenthesizedExpressionNode(int lineno, int colno, AbstractExpressionNodePtr expr)
        : AbstractPrimaryExpressionNode(lineno, colno)
        , _expr(expr)
    {
    }

    const char *type() override { return "ParenthesizedExpression"; }

    void accept(ASTNodeVisitor *visitor) override;

    AbstractExpressionNodePtr get_expr() const { return _expr; }

private:
    AbstractExpressionNodePtr _expr;
};

class InvocationExpressionNode : public AbstractPrimaryExpressionNode {
public:
    InvocationExpressionNode(int lineno, int colno, IdentifierNodePtr func_name, AbstractExpressionNodePtr arg)
        : AbstractPrimaryExpressionNode(lineno, colno)
        , _func_name(func_name)
        , _arg(arg)
    {
    }

    const char *type() override { return "InvocationExpression"; }

    void accept(ASTNodeVisitor *visitor) override;

    IdentifierNodePtr get_func_name() const { return _func_name; }

    AbstractExpressionNodePtr get_arg() const { return _arg; }

private:
    IdentifierNodePtr _func_name;
    AbstractExpressionNodePtr _arg;
};

// Statement Nodes
class AbstractStatementNode : public ASTNode {
public:
    AbstractStatementNode(int lineno, int colno)
        : ASTNode(lineno, colno)
    {
    }
};

class AbstractEmbeddedStatementNode : public AbstractStatementNode {
public:
    AbstractEmbeddedStatementNode(int lineno, int colno)
        : AbstractStatementNode(lineno, colno)
    {
    }
};

class AbstractSelectionStatementNode : public AbstractEmbeddedStatementNode {
public:
    AbstractSelectionStatementNode(int lineno, int colno)
        : AbstractEmbeddedStatementNode(lineno, colno)
    {
    }
};

class AbstractIterationStatementNode : public AbstractEmbeddedStatementNode {
public:
    AbstractIterationStatementNode(int lineno, int colno)
        : AbstractEmbeddedStatementNode(lineno, colno)
    {
    }
};

class IfStatementNode : public AbstractSelectionStatementNode {
public:
    IfStatementNode(int lineno, int colno, AbstractExpressionNodePtr condition, AbstractEmbeddedStatementNodePtr then_stmt, AbstractEmbeddedStatementNodePtr else_stmt = nullptr)
        : AbstractSelectionStatementNode(lineno, colno)
        , _condition(condition)
        , _then_stmt(then_stmt)
        , _else_stmt(else_stmt)
    {
    }

    const char *type() override { return "IfStatement"; }

    void accept(ASTNodeVisitor *visitor) override;

    AbstractExpressionNodePtr get_condition() const { return _condition; }

    AbstractEmbeddedStatementNodePtr get_then_stmt() const { return _then_stmt; }

    AbstractEmbeddedStatementNodePtr get_else_stmt() const { return _else_stmt; }

private:
    AbstractExpressionNodePtr _condition;
    AbstractEmbeddedStatementNodePtr _then_stmt;
    AbstractEmbeddedStatementNodePtr _else_stmt;
};

class WhileStatementNode : public AbstractIterationStatementNode {
public:
    WhileStatementNode(int lineno, int colno, AbstractExpressionNodePtr condition, AbstractEmbeddedStatementNodePtr body)
        : AbstractIterationStatementNode(lineno, colno)
        , _condition(condition)
        , _body(body)
    {
    }

    const char *type() override { return "WhileStatement"; }

    void accept(ASTNodeVisitor *visitor) override;

    AbstractExpressionNodePtr get_condition() const { return _condition; }

    AbstractEmbeddedStatementNodePtr get_body() const { return _body; }

private:
    AbstractExpressionNodePtr _condition;
    AbstractEmbeddedStatementNodePtr _body;
};

class ForStatementNode : public AbstractIterationStatementNode {
public:
    ForStatementNode(
        int lineno, int colno,
        VariableAssignmentNodePtr init_stmt,
        AbstractExpressionNodePtr condition,
        AbstractExpressionNodePtr update_stmt,
        AbstractEmbeddedStatementNodePtr body)
        : AbstractIterationStatementNode(lineno, colno)
        , _init_stmt_assignment(init_stmt)
        , _condition(condition)
        , _update_stmt(update_stmt)
        , _body(body)
    {
    }

    ForStatementNode(
        int lineno, int colno,
        VariableDeclarationNodePtr init_stmt,
        AbstractExpressionNodePtr condition,
        AbstractExpressionNodePtr update_stmt,
        AbstractEmbeddedStatementNodePtr body)
        : AbstractIterationStatementNode(lineno, colno)
        , _init_stmt_declaration(init_stmt)
        , _condition(condition)
        , _update_stmt(update_stmt)
        , _body(body)
    {
    }

    const char *type() override { return "ForStatement"; }

    void accept(ASTNodeVisitor *visitor) override;

    ASTNodePtr get_init_stmt() const
    {
        if (_init_stmt_assignment) {
            return _init_stmt_assignment;
        } else {
            return _init_stmt_declaration;
        }
    }

    AbstractExpressionNodePtr get_condition() const { return _condition; }

    AbstractExpressionNodePtr get_update_stmt() const { return _update_stmt; }

    AbstractEmbeddedStatementNodePtr get_body() const { return _body; }

private:
    VariableAssignmentNodePtr _init_stmt_assignment;
    VariableDeclarationNodePtr _init_stmt_declaration;
    AbstractExpressionNodePtr _condition;
    AbstractExpressionNodePtr _update_stmt;
    AbstractEmbeddedStatementNodePtr _body;
};

class ReturnStatementNode : public AbstractEmbeddedStatementNode {
public:
    ReturnStatementNode(int lineno, int colno, AbstractExpressionNodePtr expr = nullptr)
        : AbstractEmbeddedStatementNode(lineno, colno)
        , _expr(expr)
    {
    }

    const char *type() override { return "ReturnStatement"; }

    void accept(ASTNodeVisitor *visitor) override;

    AbstractExpressionNodePtr get_expr() const { return _expr; }

private:
    AbstractExpressionNodePtr _expr;
};

class InvocationStatementNode : public AbstractEmbeddedStatementNode {
public:
    InvocationStatementNode(int lineno, int colno, InvocationExpressionNodePtr expr = nullptr)
        : AbstractEmbeddedStatementNode(lineno, colno)
        , _expr(expr)
    {
    }

    const char *type() override { return "InvocationStatement"; }

    void accept(ASTNodeVisitor *visitor) override;

    InvocationExpressionNodePtr get_expr() const { return _expr; }

private:
    InvocationExpressionNodePtr _expr;
};

class FloorAssignmentStatementNode : public AbstractEmbeddedStatementNode {
public:
    FloorAssignmentStatementNode(int lineno, int colno, FloorAssignmentNodePtr assignment)
        : AbstractEmbeddedStatementNode(lineno, colno)
        , _assignment(assignment)
    {
    }

    const char *type() override { return "FloorAssignmentStatement"; }

    void accept(ASTNodeVisitor *visitor) override;

    FloorAssignmentNodePtr get_floor_assignment() const { return _assignment; }

private:
    FloorAssignmentNodePtr _assignment;
};

class VariableAssignmentStatementNode : public AbstractEmbeddedStatementNode {
public:
    VariableAssignmentStatementNode(int lineno, int colno, VariableAssignmentNodePtr assignment)
        : AbstractEmbeddedStatementNode(lineno, colno)
        , _assignment(assignment)
    {
    }

    const char *type() override { return "VariableAssignmentStatement"; }

    void accept(ASTNodeVisitor *visitor) override;

    VariableAssignmentNodePtr get_variable_assignment() const { return _assignment; }

private:
    VariableAssignmentNodePtr _assignment;
};

class VariableDeclarationStatementNode : public AbstractStatementNode {
public:
    VariableDeclarationStatementNode(int lineno, int colno, VariableDeclarationNodePtr decl)
        : AbstractStatementNode(lineno, colno)
        , decl(decl)
    {
    }

    const char *type() override { return "VariableDeclarationStatement"; }

    void accept(ASTNodeVisitor *visitor) override;

    VariableDeclarationNodePtr get_variable_decl() const { return decl; }

private:
    VariableDeclarationNodePtr decl;
};

class FloorBoxInitStatementNode : public AbstractStatementNode {
public:
    FloorBoxInitStatementNode(int lineno, int colno, IntegerLiteralNodePtr index, IntegerLiteralNodePtr value)
        : AbstractStatementNode(lineno, colno)
        , _index(index)
        , _value(value)
    {
    }

    const char *type() override { return "FloorBoxInitStatement"; }

    void accept(ASTNodeVisitor *visitor) override;

    IntegerLiteralNodePtr get_index() const { return _index; }

    IntegerLiteralNodePtr get_value() const { return _value; }

private:
    IntegerLiteralNodePtr _index;
    IntegerLiteralNodePtr _value;
};

class FloorMaxInitStatementNode : public AbstractStatementNode {
public:
    FloorMaxInitStatementNode(int lineno, int colno, IntegerLiteralNodePtr value)
        : AbstractStatementNode(lineno, colno)
        , _value(value)
    {
    }

    const char *type() override { return "FloorMaxInitStatement"; }

    void accept(ASTNodeVisitor *visitor) override;

    IntegerLiteralNodePtr get_value() const { return _value; }

private:
    IntegerLiteralNodePtr _value;
};

class EmptyStatementNode : public AbstractEmbeddedStatementNode {
public:
    EmptyStatementNode(int lineno, int colno)
        : AbstractEmbeddedStatementNode(lineno, colno)
    {
    }

    const char *type() override { return "EmptyStatement"; }

    void accept(ASTNodeVisitor *visitor) override;
};

class StatementBlockNode : public AbstractEmbeddedStatementNode {
public:
    StatementBlockNode(int lineno, int colno, const std::vector<AbstractStatementNodePtr> &statements)
        : AbstractEmbeddedStatementNode(lineno, colno)
        , _statements(statements)
    {
    }

    const char *type() override { return "StatementBlock"; }

    void accept(ASTNodeVisitor *visitor) override;

    const std::vector<AbstractStatementNodePtr> &get_statements() const { return _statements; }

private:
    std::vector<AbstractStatementNodePtr> _statements;
};

// Function and Subprocedure Nodes
class RoutineDefinitionCommonNode : public ASTNode {
public:
    RoutineDefinitionCommonNode(int lineno, int colno, IdentifierNodePtr function_name, IdentifierNodePtr formal_parameter, StatementBlockNodePtr body)
        : ASTNode(lineno, colno)
        , _function_name(function_name)
        , _formal_parameter(formal_parameter)
        , _body(body)
    {
    }

    IdentifierNodePtr get_function_name() const { return _function_name; }

    IdentifierNodePtr get_formal_parameter() const { return _formal_parameter; }

    StatementBlockNodePtr get_body() const { return _body; }

protected:
    IdentifierNodePtr _function_name;
    IdentifierNodePtr _formal_parameter;
    StatementBlockNodePtr _body;
};

class FunctionDefinitionNode : public RoutineDefinitionCommonNode {
public:
    FunctionDefinitionNode(int lineno, int colno, IdentifierNodePtr function_name, IdentifierNodePtr &formal_parameter, StatementBlockNodePtr body)
        : RoutineDefinitionCommonNode(lineno, colno, function_name, formal_parameter, body)
    {
    }

    const char *type() override { return "FunctionDefinition"; }

    void accept(ASTNodeVisitor *visitor) override;
};

class SubprocDefinitionNode : public RoutineDefinitionCommonNode {
public:
    SubprocDefinitionNode(int lineno, int colno, IdentifierNodePtr function_name, IdentifierNodePtr formal_parameter, StatementBlockNodePtr body)
        : RoutineDefinitionCommonNode(lineno, colno, function_name, formal_parameter, body)
    {
    }

    const char *type() override { return "SubprocDefinition"; }

    void accept(ASTNodeVisitor *visitor) override;
};

class ImportDirectiveNode : public ASTNode {
public:
    ImportDirectiveNode(int lineno, int colno, IdentifierNodePtr module_name)
        : ASTNode(lineno, colno)
        , _module_name(module_name)
    {
    }

    const char *type() override { return "ImportDirective"; }

    void accept(ASTNodeVisitor *visitor) override;

    IdentifierNodePtr get_module_name() const { return _module_name; }

private:
    IdentifierNodePtr _module_name;
};

// Compilation Unit Node
class CompilationUnitNode : public ASTNode {
public:
    CompilationUnitNode(
        int lineno, int colno,
        const std::vector<ImportDirectiveNodePtr> &imports,
        const std::vector<FloorBoxInitStatementNodePtr> &floor_inits,
        const FloorMaxInitStatementNodePtr &floor_max,
        const std::vector<VariableDeclarationStatementNodePtr> &top_level_decls,
        const std::vector<FunctionDefinitionNodePtr> &functions,
        const std::vector<SubprocDefinitionNodePtr> &subprocs)
        : ASTNode(lineno, colno)
        , _imports(imports)
        , _floor_inits(floor_inits)
        , _floor_max(floor_max)
        , _top_level_decls(top_level_decls)
        , _functions(functions)
        , _subprocs(subprocs)
    {
    }

    const char *type() override { return "CompilationUnit"; }

    void accept(ASTNodeVisitor *visitor) override;

    const std::vector<ImportDirectiveNodePtr> &get_imports() const { return _imports; }

    const std::vector<FloorBoxInitStatementNodePtr> &get_floor_inits() const { return _floor_inits; }

    const FloorMaxInitStatementNodePtr &get_floor_max() const { return _floor_max; }

    const std::vector<VariableDeclarationStatementNodePtr> &get_top_level_decls() const { return _top_level_decls; }

    const std::vector<FunctionDefinitionNodePtr> &get_functions() const { return _functions; }

    const std::vector<SubprocDefinitionNodePtr> &get_subprocs() const { return _subprocs; }

private:
    std::vector<ImportDirectiveNodePtr> _imports;
    std::vector<FloorBoxInitStatementNodePtr> _floor_inits;
    FloorMaxInitStatementNodePtr _floor_max;
    std::vector<VariableDeclarationStatementNodePtr> _top_level_decls;
    std::vector<FunctionDefinitionNodePtr> _functions;
    std::vector<SubprocDefinitionNodePtr> _subprocs;
};

template <typename T>
concept convertible_to_ASTNodePtr = std::convertible_to<T, ASTNodePtr>;

CLOSE_PARSER_NAMESPACE

#endif