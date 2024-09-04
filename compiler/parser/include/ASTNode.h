#ifndef ASTNODE_H
#define ASTNODE_H

#include <memory>
#include <vector>

#include "ASTNodeForward.h"
#include "ParseTreeNodeForward.h"
#include "hrl_global.h"
#include "parser_global.h"

#define VISIT_NODE(x) \
    (void)(x);        \
    return 0; // nothing

OPEN_PARSER_NAMESPACE

class ASTNodeVisitor;

class ASTNode : public std::enable_shared_from_this<ASTNode> {
public:
    ASTNode(int lineno, int colno, int width)
        : _lineno(lineno)
        , _colno(colno)
        , _width(width)
    {
    }

    virtual ~ASTNode() { }

    virtual int accept(ASTNodeVisitor *visitor) = 0;

    int lineno() { return _lineno; }

    int colno() { return _colno; }

    int width() { return _width; }

protected:
private:
    int _lineno;
    int _colno;
    int _width;
};

// AbstractExpressionASTNode
class AbstractExpressionASTNode : public ASTNode {
public:
    AbstractExpressionASTNode(int lineno, int colno, int width)
        : ASTNode(lineno, colno, width)
    {
    }
};

// AbstractUnaryExpressionASTNode
class AbstractUnaryExpressionASTNode : public AbstractExpressionASTNode {
public:
    AbstractUnaryExpressionASTNode(int lineno, int colno, int width)
        : AbstractExpressionASTNode(lineno, colno, width)
    {
    }

protected:
private:
};

// AbstractPrimaryExpressionASTNode
class AbstractPrimaryExpressionASTNode : public AbstractExpressionASTNode {
public:
    AbstractPrimaryExpressionASTNode(int lineno, int colno, int width)
        : AbstractExpressionASTNode(lineno, colno, width)
    {
    }
};

// AbstractBinaryExpressionASTNode
class AbstractBinaryExpressionASTNode : public AbstractExpressionASTNode {
public:
    AbstractBinaryExpressionASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractExpressionASTNode(lineno, colno, width)
        , _left(left)
        , _right(right)
    {
    }

    AbstractExpressionASTNodePtr get_left() { return _left; }

    AbstractExpressionASTNodePtr get_right() { return _right; }

protected:
private:
    AbstractExpressionASTNodePtr _left;
    AbstractExpressionASTNodePtr _right;
};

class AbstractStatementASTNode : public ASTNode {
public:
    AbstractStatementASTNode(int lineno, int colno, int width)
        : ASTNode(lineno, colno, width)
    {
    }
};

class AbstractSubroutineASTNode : public ASTNode {
public:
    AbstractSubroutineASTNode(int lineno, int colno, int width)
        : ASTNode(lineno, colno, width)
    {
    }
};

class IntegerASTNode : public ASTNode {
public:
    IntegerASTNode(int lineno, int colno, int width, int value)
        : ASTNode(lineno, colno, width)
        , _value(value)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    int get_value();

protected:
private:
    int _value;
};

// BooleanASTNode
class BooleanASTNode : public ASTNode {
public:
    BooleanASTNode(int lineno, int colno, int width, bool value)
        : ASTNode(lineno, colno, width)
        , _value(value)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    bool get_value() { return _value; }

protected:
private:
    bool _value;
};

// VariableDeclarationASTNode
class VariableDeclarationASTNode : public ASTNode {
public:
    VariableDeclarationASTNode(int lineno, int colno, int width, StringPtr name, VariableAssignmentASTNodePtr assignment)
        : ASTNode(lineno, colno, width)
        , _name(name)
        , _assignment(assignment)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    StringPtr get_name() { return _name; }

    VariableAssignmentASTNodePtr get_assignment() { return _assignment; }

protected:
private:
    StringPtr _name;
    VariableAssignmentASTNodePtr _assignment;
};

// VariableAssignmentASTNode
class VariableAssignmentASTNode : public ASTNode {
public:
    VariableAssignmentASTNode(int lineno, int colno, int width, StringPtr name, AbstractExpressionASTNodePtr value)
        : ASTNode(lineno, colno, width)
        , _name(name)
        , _value(value)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    StringPtr get_name() { return _name; }

    AbstractExpressionASTNodePtr get_value() { return _value; }

protected:
private:
    StringPtr _name;
    AbstractExpressionASTNodePtr _value;
};

// VariableAccessASTNode
class VariableAccessASTNode : public AbstractPrimaryExpressionASTNode {
public:
    VariableAccessASTNode(int lineno, int colno, int width, StringPtr name)
        : AbstractPrimaryExpressionASTNode(lineno, colno, width)
        , _name(name)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    StringPtr get_name() { return _name; }

protected:
private:
    StringPtr _name;
};

// FloorAssignmentASTNode
class FloorAssignmentASTNode : public ASTNode {
public:
    FloorAssignmentASTNode(int lineno, int colno, int width, int floor_number, AbstractExpressionASTNodePtr value)
        : ASTNode(lineno, colno, width)
        , _floor_number(floor_number)
        , _value(value)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    int get_floor_number() { return _floor_number; }

    AbstractExpressionASTNodePtr get_value() { return _value; }

protected:
private:
    int _floor_number;
    AbstractExpressionASTNodePtr _value;
};

// FloorAccessASTNode
class FloorAccessASTNode : public ASTNode {
public:
    FloorAccessASTNode(int lineno, int colno, int width, int floor_number)
        : ASTNode(lineno, colno, width)
        , _floor_number(floor_number)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    int get_floor_number() { return _floor_number; }

protected:
private:
    int _floor_number;
};

// NegativeExpressionASTNode
class NegativeExpressionASTNode : public AbstractUnaryExpressionASTNode {
public:
    NegativeExpressionASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr operand)
        : AbstractUnaryExpressionASTNode(lineno, colno, width)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    AbstractExpressionASTNodePtr get_operand() { return _operand; }

private:
    AbstractExpressionASTNodePtr _operand;
};

// NotExpressionASTNode
class NotExpressionASTNode : public AbstractUnaryExpressionASTNode {
public:
    NotExpressionASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr operand)
        : AbstractUnaryExpressionASTNode(lineno, colno, width)
    {
    }

    int accept(ASTNodeVisitor *visitor) override { VISIT_NODE(visitor); }

    AbstractExpressionASTNodePtr get_operand() { return _operand; }

private:
    AbstractExpressionASTNodePtr _operand;
};

// IncrementExpressionASTNode
class IncrementExpressionASTNode : public AbstractUnaryExpressionASTNode {
public:
    IncrementExpressionASTNode(int lineno, int colno, int width, StringPtr var_name)
        : AbstractUnaryExpressionASTNode(lineno, colno, width)
        , _var_name(var_name)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    StringPtr get_var_name() const { return _var_name; }

private:
    StringPtr _var_name;
};

// DecrementExpressionASTNode
class DecrementExpressionASTNode : public AbstractUnaryExpressionASTNode {
public:
    DecrementExpressionASTNode(int lineno, int colno, int width, StringPtr var_name)
        : AbstractUnaryExpressionASTNode(lineno, colno, width)
        , _var_name(var_name)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    StringPtr get_var_name() const { return _var_name; }

private:
    StringPtr _var_name;
};

// AddExpressionASTNode
class AddExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    AddExpressionASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, width, left, right)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// SubExpressionASTNode
class SubExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    SubExpressionASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, width, left, right)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// MulExpressionASTNode
class MulExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    MulExpressionASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, width, left, right)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// DivExpressionASTNode
class DivExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    DivExpressionASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, width, left, right)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// ModExpressionASTNode
class ModExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    ModExpressionASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, width, left, right)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// EqualExpressionASTNode
class EqualExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    EqualExpressionASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, width, left, right)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// NotEqualExpressionASTNode
class NotEqualExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    NotEqualExpressionASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, width, left, right)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// GreaterThanExpressionASTNode
class GreaterThanExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    GreaterThanExpressionASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, width, left, right)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// GreaterEqualExpressionASTNode
class GreaterEqualExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    GreaterEqualExpressionASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, width, left, right)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// LessThanExpressionASTNode
class LessThanExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    LessThanExpressionASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, width, left, right)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// LessEqualExpressionASTNode
class LessEqualExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    LessEqualExpressionASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, width, left, right)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// AndExpressionASTNode
class AndExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    AndExpressionASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, width, left, right)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// OrExpressionASTNode
class OrExpressionASTNode : public AbstractBinaryExpressionASTNode {
public:
    OrExpressionASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr left, AbstractExpressionASTNodePtr right)
        : AbstractBinaryExpressionASTNode(lineno, colno, width, left, right)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;
};

// InvocationExpressionASTNode
class InvocationExpressionASTNode : public AbstractPrimaryExpressionASTNode {
public:
    InvocationExpressionASTNode(int lineno, int colno, int width, StringPtr func_name, AbstractExpressionASTNodePtr arg)
        : AbstractPrimaryExpressionASTNode(lineno, colno, width)
        , _func_name(func_name)
        , _args(arg)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    StringPtr get_func_name() { return _func_name; }

    AbstractExpressionASTNodePtr get_argument() { return _args; }

protected:
private:
    StringPtr _func_name;
    AbstractExpressionASTNodePtr _args;
};

// IfStatementASTNode
class IfStatementASTNode : public AbstractStatementASTNode {
public:
    IfStatementASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr condition, StatementsVector then_branch, StatementsVector else_branch)
        : AbstractStatementASTNode(lineno, colno, width)
        , _condition(condition)
        , _then_branch(then_branch)
        , _else_branch(else_branch)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    AbstractExpressionASTNodePtr get_condition() { return _condition; }

    const StatementsVector &get_then_branch() const { return _then_branch; }

    const StatementsVector &get_else_branch() const { return _else_branch; }

protected:
private:
    AbstractExpressionASTNodePtr _condition;
    StatementsVector _then_branch;
    StatementsVector _else_branch;
};

// WhileStatementASTNode
class WhileStatementASTNode : public AbstractStatementASTNode {
public:
    WhileStatementASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr condition, StatementsVector body)
        : AbstractStatementASTNode(lineno, colno, width)
        , _condition(condition)
        , _body(body)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    AbstractExpressionASTNodePtr get_condition() { return _condition; }

    StatementsVector get_body() { return _body; }

protected:
private:
    AbstractExpressionASTNodePtr _condition;
    StatementsVector _body;
};

// ForStatementASTNode
class ForStatementASTNode : public AbstractStatementASTNode {
public:
    ForStatementASTNode(int lineno, int colno, int width, AbstractStatementASTNodePtr initializer, AbstractExpressionASTNodePtr condition, AbstractStatementASTNodePtr update, StatementsVector body)
        : AbstractStatementASTNode(lineno, colno, width)
        , _initializer(initializer)
        , _condition(condition)
        , _update(update)
        , _body(body)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    AbstractStatementASTNodePtr get_initializer() { return _initializer; }

    AbstractExpressionASTNodePtr get_condition() { return _condition; }

    AbstractStatementASTNodePtr get_update() { return _update; }

    StatementsVector get_body() { return _body; }

protected:
private:
    AbstractStatementASTNodePtr _initializer;
    AbstractExpressionASTNodePtr _condition;
    AbstractStatementASTNodePtr _update;
    StatementsVector _body;
};

// ReturnStatementASTNode
class ReturnStatementASTNode : public AbstractStatementASTNode {
public:
    ReturnStatementASTNode(int lineno, int colno, int width, AbstractExpressionASTNodePtr expression)
        : AbstractStatementASTNode(lineno, colno, width)
        , _expression(expression)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    AbstractExpressionASTNodePtr get_expression() { return _expression; }

protected:
private:
    AbstractExpressionASTNodePtr _expression;
};

// StatementBlockASTNode
class StatementBlockASTNode : public AbstractStatementASTNode {
public:
    StatementBlockASTNode(int lineno, int colno, int width, StatementsVector statements)
        : AbstractStatementASTNode(lineno, colno, width)
        , _statements(statements)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    StatementsVector get_statements() { return _statements; }

protected:
private:
    StatementsVector _statements;
};

class SubprocDefinitionASTNode : public AbstractSubroutineASTNode {
public:
    SubprocDefinitionASTNode(int lineno, int colno, int width, StringPtr name, StringPtr parameter, StatementBlockASTNodePtr body)
        : AbstractSubroutineASTNode(lineno, colno, width)
        , _name(name)
        , _parameter(parameter)
        , _body(body)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    StringPtr get_name() { return _name; }

    StringPtr get_parameters() { return _parameter; }

    StatementBlockASTNodePtr get_body() { return _body; }

protected:
private:
    StringPtr _name;
    StringPtr _parameter;
    StatementBlockASTNodePtr _body;
};

class FunctionDefinitionASTNode : public AbstractSubroutineASTNode {
public:
    FunctionDefinitionASTNode(int lineno, int colno, int width, StringPtr name, StringPtr return_type, StringPtr parameter, StatementBlockASTNodePtr body)
        : AbstractSubroutineASTNode(lineno, colno, width)
        , _name(name)
        , _return_type(return_type)
        , _parameter(parameter)
        , _body(body)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    StringPtr get_name() { return _name; }

    StringPtr get_return_type() { return _return_type; }

    StringPtr get_parameter() { return _parameter; }

    StatementBlockASTNodePtr get_body() { return _body; }

protected:
private:
    StringPtr _name;
    StringPtr _return_type;
    StringPtr _parameter;
    StatementBlockASTNodePtr _body;
};

// CompilationUnitASTNode
class CompilationUnitASTNode : public ASTNode {
public:
    CompilationUnitASTNode(int lineno, int colno, int width,
        std::vector<ImportDirectivePTNodePtr> imports,
        std::vector<FloorBoxInitStatementASTNodePtr> floor_inits,
        FloorMaxInitStatementASTNodePtr floor_max,
        std::vector<VariableDeclarationASTNodePtr> var_decls,
        std::vector<AbstractSubroutineASTNodePtr> subroutines)
        : ASTNode(lineno, colno, width)
        , _imports(imports)
        , _floor_inits(floor_inits)
        , _floor_max(floor_max)
        , _var_decls(var_decls)
        , _subroutines(subroutines)
    {
    }

    int accept(ASTNodeVisitor *visitor) override;

    const std::vector<ImportDirectivePTNodePtr> &get_imports() const { return _imports; }

    const std::vector<FloorBoxInitStatementASTNodePtr> &get_floor_inits() const { return _floor_inits; }

    FloorMaxInitStatementASTNodePtr get_floor_max() { return _floor_max; }

    const std::vector<VariableDeclarationASTNodePtr> &get_var_decls() const { return _var_decls; }

    const std::vector<AbstractSubroutineASTNodePtr> &get_subroutines() const { return _subroutines; }

protected:
private:
    std::vector<ImportDirectivePTNodePtr> _imports;
    std::vector<FloorBoxInitStatementASTNodePtr> _floor_inits;
    FloorMaxInitStatementASTNodePtr _floor_max;
    std::vector<VariableDeclarationASTNodePtr> _var_decls;
    std::vector<AbstractSubroutineASTNodePtr> _subroutines;
};

CLOSE_PARSER_NAMESPACE

#endif