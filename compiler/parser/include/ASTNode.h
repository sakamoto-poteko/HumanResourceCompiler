#ifndef ASTNODE_H
#define ASTNODE_H

#include <concepts>
#include <map>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include "ASTNodeForward.h"
#include "hrl_global.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

class ASTNodeVisitor;
class ASTNodeAttribute;
using ASTNodeAttributePtr = std::shared_ptr<ASTNodeAttribute>;

class ASTNode : public std::enable_shared_from_this<ASTNode> {
public:
    ASTNode(int lineno, int colno, int last_lineno, int last_colno)
        : _lineno(lineno)
        , _colno(colno)
        , _last_lineno(last_lineno)
        , _last_colno(last_colno)
    {
    }

    virtual ~ASTNode() = default;

    virtual int accept(ASTNodeVisitor *visitor) = 0;

    int &lineno() { return _lineno; }

    int &colno() { return _colno; }

    int &last_lineno() { return _last_lineno; }

    int &last_colno() { return _last_colno; }

    /**
     * @brief Get the attribute attached to this node
     *
     * @param attribute_id
     * @param out
     * @return true The node has the attribute with \p attribute_id and it's not null
     * @return false The node has no attribute with \p attribute_id
     */
    bool get_attribute(int attribute_id, ASTNodeAttributePtr &out) const;

    /**
     * @brief Attach the attribute to this node. If the \p attr is null, it won't be attached.
     *
     * @param attribute_id
     * @param attr
     */
    void set_attribute(int attribute_id, ASTNodeAttributePtr attr);

    void copy_attributes_from(const ASTNodePtr &node);

protected:
    template <typename T>
    std::shared_ptr<T> shared_from_this_casted()
    {
        return std::static_pointer_cast<T>(shared_from_this());
    }

private:
    int _lineno;
    int _colno;
    int _last_lineno;
    int _last_colno;
    std::map<int, ASTNodeAttributePtr> _attributes;
};

class AbstractStatementASTNode : public ASTNode {
public:
    AbstractStatementASTNode(int lineno, int colno, int last_lineno, int last_colno)
        : ASTNode(lineno, colno, last_lineno, last_colno)
    {
    }
};

class AbstractEmbeddedStatementASTNode : public AbstractStatementASTNode {
public:
    AbstractEmbeddedStatementASTNode(int lineno, int colno, int last_lineno, int last_colno)
        : AbstractStatementASTNode(lineno, colno, last_lineno, last_colno)
    {
    }
};

// AbstractExpressionASTNode
class AbstractExpressionASTNode : public AbstractEmbeddedStatementASTNode {
public:
    AbstractExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno)
        : AbstractEmbeddedStatementASTNode(lineno, colno, last_lineno, last_colno)
    {
    }
};

// AbstractUnaryExpressionASTNode
class AbstractUnaryExpressionASTNode : public AbstractExpressionASTNode {
public:
    AbstractUnaryExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr operand)
        : AbstractExpressionASTNode(lineno, colno, last_lineno, last_colno)
        , _operand(operand)
    {
    }

    AbstractExpressionASTNodePtr &get_operand() { return _operand; }

protected:
private:
    AbstractExpressionASTNodePtr _operand;
};

// AbstractPrimaryExpressionASTNode
class AbstractPrimaryExpressionASTNode : public AbstractExpressionASTNode {
public:
    AbstractPrimaryExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno)
        : AbstractExpressionASTNode(lineno, colno, last_lineno, last_colno)
    {
    }
};

enum class ASTBinaryOperator : int {
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,

    AND,
    OR,

    GT,
    GE,
    LT,
    LE,
    EQ,
    NE,
};

// AbstractBinaryExpressionASTNode
class AbstractBinaryExpressionASTNode : public AbstractExpressionASTNode {
public:
    AbstractBinaryExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right, ASTBinaryOperator op)
        : AbstractExpressionASTNode(lineno, colno, last_lineno, last_colno)
        , _left(std::move(left))
        , _right(std::move(right))
        , _op(op)
    {
    }

    AbstractExpressionASTNodePtr &get_left() { return _left; }

    AbstractExpressionASTNodePtr &get_right() { return _right; }

    ASTBinaryOperator &get_op() { return _op; }

protected:
private:
    AbstractExpressionASTNodePtr _left;
    AbstractExpressionASTNodePtr _right;
    ASTBinaryOperator _op;
};

class AbstractSubroutineASTNode : public ASTNode {
public:
    AbstractSubroutineASTNode(int lineno, int colno, int last_lineno, int last_colno, StringPtr name, VariableDeclarationASTNodePtr parameter, StatementBlockASTNodePtr body)
        : ASTNode(lineno, colno, last_lineno, last_colno)
        , _name(std::move(name))
        , _parameter(std::move(parameter))
        , _body(std::move(body))
    {
    }

    StringPtr &get_name() { return _name; }

    VariableDeclarationASTNodePtr &get_parameter() { return _parameter; }

    StatementBlockASTNodePtr &get_body() { return _body; }

protected:
private:
    StringPtr _name;
    VariableDeclarationASTNodePtr _parameter;
    StatementBlockASTNodePtr _body;
};

class EmptyStatementASTNode : public AbstractEmbeddedStatementASTNode {
public:
    EmptyStatementASTNode(int lineno, int colno, int last_lineno, int last_colno)
        : AbstractEmbeddedStatementASTNode(lineno, colno, last_lineno, last_colno)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

class IntegerASTNode : public AbstractPrimaryExpressionASTNode {
public:
    IntegerASTNode(int lineno, int colno, int last_lineno, int last_colno, int value)
        : AbstractPrimaryExpressionASTNode(lineno, colno, last_lineno, last_colno)
        , _value(value)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    int &get_value() { return _value; }

protected:
private:
    int _value;
};

// BooleanASTNode
class BooleanASTNode : public AbstractPrimaryExpressionASTNode {
public:
    BooleanASTNode(int lineno, int colno, int last_lineno, int last_colno, bool value)
        : AbstractPrimaryExpressionASTNode(lineno, colno, last_lineno, last_colno)
        , _value(value)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    bool &get_value() { return _value; }

protected:
private:
    bool _value;
};

// VariableDeclarationASTNode
class VariableDeclarationASTNode : public AbstractStatementASTNode {
public:
    VariableDeclarationASTNode(int lineno, int colno, int last_lineno, int last_colno, StringPtr name, VariableAssignmentASTNodePtr assignment)
        : AbstractStatementASTNode(lineno, colno, last_lineno, last_colno)
        , _name(std::move(name))
        , _assignment(std::move(assignment))
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    StringPtr &get_name() { return _name; }

    VariableAssignmentASTNodePtr &get_assignment() { return _assignment; }

protected:
private:
    StringPtr _name;
    VariableAssignmentASTNodePtr _assignment;
};

// VariableAssignmentASTNode
class VariableAssignmentASTNode : public AbstractEmbeddedStatementASTNode {
public:
    VariableAssignmentASTNode(int lineno, int colno, int last_lineno, int last_colno, StringPtr name, AbstractExpressionASTNodePtr value)
        : AbstractEmbeddedStatementASTNode(lineno, colno, last_lineno, last_colno)
        , _name(std::move(name))
        , _value(std::move(value))
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    StringPtr &get_name() { return _name; }

    AbstractExpressionASTNodePtr &get_value() { return _value; }

protected:
private:
    StringPtr _name;
    AbstractExpressionASTNodePtr _value;
};

// VariableAccessASTNode
class VariableAccessASTNode : public AbstractPrimaryExpressionASTNode {
public:
    VariableAccessASTNode(int lineno, int colno, int last_lineno, int last_colno, StringPtr name)
        : AbstractPrimaryExpressionASTNode(lineno, colno, last_lineno, last_colno)
        , _name(name)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    StringPtr &get_name() { return _name; }

protected:
private:
    StringPtr _name;
};

class FloorBoxInitStatementASTNode : public ASTNode {
public:
    FloorBoxInitStatementASTNode(int lineno, int colno, int last_lineno, int last_colno, FloorAssignmentASTNodePtr assignment)
        : ASTNode(lineno, colno, last_lineno, last_colno)
        , _assignment(std::move(assignment))
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    FloorAssignmentASTNodePtr &get_assignment() { return _assignment; }

private:
    FloorAssignmentASTNodePtr _assignment;
};

// FloorAssignmentASTNode
class FloorAssignmentASTNode : public AbstractEmbeddedStatementASTNode {
public:
    FloorAssignmentASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr floor_number, AbstractExpressionASTNodePtr value)
        : AbstractEmbeddedStatementASTNode(lineno, colno, last_lineno, last_colno)
        , _floor_number(std::move(floor_number))
        , _value(std::move(value))
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    AbstractExpressionASTNodePtr &get_floor_number() { return _floor_number; }

    AbstractExpressionASTNodePtr &get_value() { return _value; }

protected:
private:
    AbstractExpressionASTNodePtr _floor_number;
    AbstractExpressionASTNodePtr _value;
};

// FloorAccessASTNode
class FloorAccessASTNode : public AbstractPrimaryExpressionASTNode {
public:
    FloorAccessASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr index_expr)
        : AbstractPrimaryExpressionASTNode(lineno, colno, last_lineno, last_colno)
        , _index_expr(std::move(index_expr))
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    AbstractExpressionASTNodePtr &get_index_expr() { return _index_expr; }

protected:
private:
    AbstractExpressionASTNodePtr _index_expr;
};

// NegativeExpressionASTNode
class NegativeExpressionASTNode : public AbstractUnaryExpressionASTNode {
public:
    NegativeExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr operand)
        : AbstractUnaryExpressionASTNode(lineno, colno, last_lineno, last_colno, std::move(operand))
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

private:
};

// NotExpressionASTNode
class NotExpressionASTNode : public AbstractUnaryExpressionASTNode {
public:
    NotExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr operand)
        : AbstractUnaryExpressionASTNode(lineno, colno, last_lineno, last_colno, std::move(operand))
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

private:
};

// IncrementExpressionASTNode
class IncrementExpressionASTNode : public AbstractUnaryExpressionASTNode {
public:
    IncrementExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, StringPtr var_name)
        : AbstractUnaryExpressionASTNode(lineno, colno, last_lineno, last_colno, nullptr)
        , _var_name(std::move(var_name))
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    StringPtr &get_var_name() { return _var_name; }

private:
    StringPtr _var_name;
};

// DecrementExpressionASTNode
class DecrementExpressionASTNode : public AbstractUnaryExpressionASTNode {
public:
    DecrementExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, StringPtr var_name)
        : AbstractUnaryExpressionASTNode(lineno, colno, last_lineno, last_colno, nullptr)
        , _var_name(std::move(var_name))
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    StringPtr &get_var_name() { return _var_name; }

private:
    StringPtr _var_name;
};

// AddExpressionASTNode
class AddExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    AddExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, last_lineno, last_colno, std::move(left), std::move(right), ASTBinaryOperator::ADD)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// SubExpressionASTNode
class SubExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    SubExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, last_lineno, last_colno, std::move(left), std::move(right), ASTBinaryOperator::SUB)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// MulExpressionASTNode
class MulExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    MulExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, last_lineno, last_colno, std::move(left), std::move(right), ASTBinaryOperator::MUL)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// DivExpressionASTNode
class DivExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    DivExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, last_lineno, last_colno, std::move(left), std::move(right), ASTBinaryOperator::DIV)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// ModExpressionASTNode
class ModExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    ModExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, last_lineno, last_colno, std::move(left), std::move(right), ASTBinaryOperator::MOD)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// EqualExpressionASTNode
class EqualExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    EqualExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, last_lineno, last_colno, std::move(left), std::move(right), ASTBinaryOperator::EQ)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// NotEqualExpressionASTNode
class NotEqualExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    NotEqualExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, last_lineno, last_colno, std::move(left), std::move(right), ASTBinaryOperator::NE)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// GreaterThanExpressionASTNode
class GreaterThanExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    GreaterThanExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, last_lineno, last_colno, std::move(left), std::move(right), ASTBinaryOperator::GT)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// GreaterEqualExpressionASTNode
class GreaterEqualExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    GreaterEqualExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, last_lineno, last_colno, std::move(left), std::move(right), ASTBinaryOperator::GE)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// LessThanExpressionASTNode
class LessThanExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    LessThanExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, last_lineno, last_colno, std::move(left), std::move(right), ASTBinaryOperator::LT)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// LessEqualExpressionASTNode
class LessEqualExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    LessEqualExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, last_lineno, last_colno, std::move(left), std::move(right), ASTBinaryOperator::LE)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// AndExpressionASTNode
class AndExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    AndExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, last_lineno, last_colno, std::move(left), std::move(right), ASTBinaryOperator::AND)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// OrExpressionASTNode
class OrExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    OrExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, last_lineno, last_colno, std::move(left), std::move(right), ASTBinaryOperator::OR)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// InvocationExpressionASTNode
class InvocationExpressionASTNode : public AbstractPrimaryExpressionASTNode {
public:
    InvocationExpressionASTNode(int lineno, int colno, int last_lineno, int last_colno, StringPtr func_name, AbstractExpressionASTNodePtr arg)
        : AbstractPrimaryExpressionASTNode(lineno, colno, last_lineno, last_colno)
        , _func_name(std::move(func_name))
        , _args(std::move(arg))
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    StringPtr &get_func_name() { return _func_name; }

    AbstractExpressionASTNodePtr &get_argument() { return _args; }

protected:
private:
    StringPtr _func_name;
    AbstractExpressionASTNodePtr _args;
};

// IfStatementASTNode
class IfStatementASTNode : public AbstractEmbeddedStatementASTNode {
public:
    IfStatementASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr condition, AbstractEmbeddedStatementASTNodePtr then_branch, AbstractEmbeddedStatementASTNodePtr else_branch)
        : AbstractEmbeddedStatementASTNode(lineno, colno, last_lineno, last_colno)
        , _condition(std::move(condition))
        , _then_branch(std::move(then_branch))
        , _else_branch(std::move(else_branch))
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    AbstractExpressionASTNodePtr &get_condition() { return _condition; }

    AbstractEmbeddedStatementASTNodePtr &get_then_branch() { return _then_branch; }

    AbstractEmbeddedStatementASTNodePtr &get_else_branch() { return _else_branch; }

protected:
private:
    AbstractExpressionASTNodePtr _condition;
    AbstractEmbeddedStatementASTNodePtr _then_branch;
    AbstractEmbeddedStatementASTNodePtr _else_branch;
};

// WhileStatementASTNode
class WhileStatementASTNode : public AbstractEmbeddedStatementASTNode {
public:
    WhileStatementASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr condition, AbstractEmbeddedStatementASTNodePtr body)
        : AbstractEmbeddedStatementASTNode(lineno, colno, last_lineno, last_colno)
        , _condition(std::move(condition))
        , _body(std::move(body))
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    AbstractExpressionASTNodePtr &get_condition() { return _condition; }

    AbstractEmbeddedStatementASTNodePtr &get_body() { return _body; }

protected:
private:
    AbstractExpressionASTNodePtr _condition;
    AbstractEmbeddedStatementASTNodePtr _body;
};

// ForStatementASTNode
class ForStatementASTNode : public AbstractEmbeddedStatementASTNode {
public:
    ForStatementASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractStatementASTNodePtr init, AbstractExpressionASTNodePtr condition, AbstractStatementASTNodePtr update, AbstractEmbeddedStatementASTNodePtr body)
        : AbstractEmbeddedStatementASTNode(lineno, colno, last_lineno, last_colno)
        , _init(std::move(init))
        , _condition(std::move(condition))
        , _update(std::move(update))
        , _body(std::move(body))
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    AbstractStatementASTNodePtr &get_init() { return _init; }

    AbstractExpressionASTNodePtr &get_condition() { return _condition; }

    AbstractStatementASTNodePtr &get_update() { return _update; }

    AbstractEmbeddedStatementASTNodePtr &get_body() { return _body; }

protected:
private:
    VariableDeclarationASTNodePtr _init_decl;
    VariableAssignmentASTNodePtr _init_assign;
    AbstractStatementASTNodePtr _init;
    AbstractExpressionASTNodePtr _condition;
    AbstractStatementASTNodePtr _update;
    AbstractEmbeddedStatementASTNodePtr _body;
};

// ReturnStatementASTNode
class ReturnStatementASTNode : public AbstractEmbeddedStatementASTNode {
public:
    ReturnStatementASTNode(int lineno, int colno, int last_lineno, int last_colno, AbstractExpressionASTNodePtr expression)
        : AbstractEmbeddedStatementASTNode(lineno, colno, last_lineno, last_colno)
        , _expression(std::move(expression))
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    AbstractExpressionASTNodePtr &get_expression() { return _expression; }

protected:
private:
    AbstractExpressionASTNodePtr _expression;
};

class BreakStatementASTNode : public AbstractEmbeddedStatementASTNode {
public:
    BreakStatementASTNode(int lineno, int colno, int last_lineno, int last_colno)
        : AbstractEmbeddedStatementASTNode(lineno, colno, last_lineno, last_colno)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

protected:
private:
};

class ContinueStatementASTNode : public AbstractEmbeddedStatementASTNode {
public:
    ContinueStatementASTNode(int lineno, int colno, int last_lineno, int last_colno)
        : AbstractEmbeddedStatementASTNode(lineno, colno, last_lineno, last_colno)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

protected:
private:
};

// StatementBlockASTNode
class StatementBlockASTNode : public AbstractEmbeddedStatementASTNode {
public:
    StatementBlockASTNode(int lineno, int colno, int last_lineno, int last_colno, StatementsVector statements)
        : AbstractEmbeddedStatementASTNode(lineno, colno, last_lineno, last_colno)
        , _statements(std::move(statements))
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    StatementsVector &get_statements() { return _statements; }

protected:
private:
    StatementsVector _statements;
};

class SubprocDefinitionASTNode : public AbstractSubroutineASTNode {
public:
    SubprocDefinitionASTNode(int lineno, int colno, int last_lineno, int last_colno, StringPtr name, VariableDeclarationASTNodePtr parameter, StatementBlockASTNodePtr body)
        : AbstractSubroutineASTNode(lineno, colno, last_lineno, last_colno, name, parameter, body)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

protected:
private:
};

class FunctionDefinitionASTNode : public AbstractSubroutineASTNode {
public:
    FunctionDefinitionASTNode(int lineno, int colno, int last_lineno, int last_colno, StringPtr name, VariableDeclarationASTNodePtr parameter, StatementBlockASTNodePtr body)
        : AbstractSubroutineASTNode(lineno, colno, last_lineno, last_colno, name, parameter, body)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

protected:
private:
};

// CompilationUnitASTNode
class CompilationUnitASTNode : public ASTNode {
public:
    CompilationUnitASTNode(int lineno, int colno, int last_lineno, int last_colno,
        std::vector<StringPtr> imports,
        std::vector<FloorBoxInitStatementASTNodePtr> floor_inits,
        std::optional<int> floor_max,
        std::vector<VariableDeclarationASTNodePtr> var_decls,
        std::vector<AbstractSubroutineASTNodePtr> subroutines)
        : ASTNode(lineno, colno, last_lineno, last_colno)
        , _imports(std::move(imports))
        , _floor_inits(std::move(floor_inits))
        , _floor_max(floor_max)
        , _var_decls(std::move(var_decls))
        , _subroutines(std::move(subroutines))
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    std::vector<StringPtr> &get_imports() { return _imports; }

    std::vector<FloorBoxInitStatementASTNodePtr> &get_floor_inits() { return _floor_inits; }

    std::optional<int> &get_floor_max() { return _floor_max; }

    std::vector<VariableDeclarationASTNodePtr> &get_var_decls() { return _var_decls; }

    std::vector<AbstractSubroutineASTNodePtr> &get_subroutines() { return _subroutines; }

protected:
private:
    std::vector<StringPtr> _imports;
    std::vector<FloorBoxInitStatementASTNodePtr> _floor_inits;
    std::optional<int> _floor_max;
    std::vector<VariableDeclarationASTNodePtr> _var_decls;
    std::vector<AbstractSubroutineASTNodePtr> _subroutines;
};

template <typename T>
concept convertible_to_ASTNodePtr = requires {
    typename T::element_type;
    requires std::convertible_to<T, ASTNodePtr> && std::is_same_v<T, std::shared_ptr<typename T::element_type>>;
};

template <typename T>
    requires std::is_base_of_v<ASTNode, T>
bool is_ptr_type(const ASTNodePtr &ptr)
{
    const auto &ref = *ptr.get();
    return typeid(ref) == typeid(T);
}

CLOSE_PARSER_NAMESPACE

#endif