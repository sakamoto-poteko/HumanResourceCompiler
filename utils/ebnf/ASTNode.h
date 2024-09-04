#ifndef ParseTreeNODE_H
#define ParseTreeNODE_H

#include <memory>
#include <string>
#include <vector>

#include "ParseTreeNodeForward.h" // IWYU pragma: export

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

    int lineno()
    {
        return _lineno;
    }

    int colno()
    {
        return _colno;
    }

    virtual const char *name() = 0;

    template <typename T>
    std::shared_ptr<T> shared_from_this_casted()
    {
        return std::static_pointer_cast<T>(shared_from_this());
    }

protected:
    int _lineno;
    int _colno;
};

class SyntaxNode : public ParseTreeNode {
public:
    std::vector<ProductionNodePtr> productions;

    explicit SyntaxNode(int lineno, int colno)
        : ParseTreeNode(lineno, colno)
    {
    }

    virtual void accept(ParseTreeNodeVisitor *visitor) override;

    const char *name() override;
};

// Production node
class ProductionNode : public ParseTreeNode {
public:
    std::string id;
    ExpressionNodePtr expression;

    ProductionNode(std::string id, ExpressionNodePtr expr, int lineno,
        int colno)
        : ParseTreeNode(lineno, colno)
        , id(std::move(id))
        , expression(std::move(expr))
    {
    }

    virtual void accept(ParseTreeNodeVisitor *visitor) override;

    const char *name() override;
};

// Expression node
class ExpressionNode : public ParseTreeNode {
public:
    std::vector<TermNodePtr> terms;

    explicit ExpressionNode(int lineno, int colno)
        : ParseTreeNode(lineno, colno)
    {
    }

    void addTerm(TermNodePtr term)
    {
        terms.push_back(std::move(term));
    }

    virtual void accept(ParseTreeNodeVisitor *visitor) override;

    const char *name() override;
};

// Term node
class TermNode : public ParseTreeNode {
public:
    std::vector<FactorNodePtr> factors;

    explicit TermNode(int lineno, int colno)
        : ParseTreeNode(lineno, colno)
    {
    }

    void addFactor(FactorNodePtr factor)
    {
        factors.push_back(std::move(factor));
    }

    virtual void accept(ParseTreeNodeVisitor *visitor) override;

    const char *name() override;
};

// Factor node
class FactorNode : public ParseTreeNode {
public:
    ParseTreeNodePtr node;
    LiteralNodePtr literal;
    IdentifierNodePtr identifier;

    explicit FactorNode(IdentifierNodePtr id, int lineno, int colno)
        : ParseTreeNode(lineno, colno)
        , identifier(id)
    {
    }

    explicit FactorNode(LiteralNodePtr lit, int lineno, int colno)
        : ParseTreeNode(lineno, colno)
        , literal(lit)
    {
    }

    explicit FactorNode(ParseTreeNodePtr value, int lineno, int colno)
        : ParseTreeNode(lineno, colno)
        , node(value)
    {
    }

    virtual void accept(ParseTreeNodeVisitor *visitor) override;

    const char *name() override;
};

class IdentifierNode : public ParseTreeNode {
public:
    std::string value;

    explicit IdentifierNode(std::string value, int lineno, int colno)
        : ParseTreeNode(lineno, colno)
        , value(std::move(value))
    {
    }

    virtual void accept(ParseTreeNodeVisitor *visitor) override;

    const char *name() override;
};

class LiteralNode : public ParseTreeNode {
public:
    std::string value;

    explicit LiteralNode(std::string value, int lineno, int colno)
        : ParseTreeNode(lineno, colno)
        , value(std::move(value))
    {
    }

    virtual void accept(ParseTreeNodeVisitor *visitor) override;

    const char *name() override;
};

// Optional node
class OptionalNode : public ParseTreeNode {
public:
    ParseTreeNodePtr expression;

    explicit OptionalNode(ParseTreeNodePtr expr, int lineno, int colno)
        : ParseTreeNode(lineno, colno)
        , expression(std::move(expr))
    {
    }

    virtual void accept(ParseTreeNodeVisitor *visitor) override;

    const char *name() override;
};

// Repeated node
class RepeatedNode : public ParseTreeNode {
public:
    ParseTreeNodePtr expression;

    explicit RepeatedNode(ParseTreeNodePtr expr, int lineno, int colno)
        : ParseTreeNode(lineno, colno)
        , expression(std::move(expr))
    {
    }

    virtual void accept(ParseTreeNodeVisitor *visitor) override;

    const char *name() override;
};

// Grouped node
class GroupedNode : public ParseTreeNode {
public:
    ParseTreeNodePtr expression;

    explicit GroupedNode(ParseTreeNodePtr expr, int lineno, int colno)
        : ParseTreeNode(lineno, colno)
        , expression(std::move(expr))
    {
    }

    virtual void accept(ParseTreeNodeVisitor *visitor) override;

    const char *name() override;
};

class EpsilonNode : public ParseTreeNode {
public:
    explicit EpsilonNode(int lineno, int colno)
        : ParseTreeNode(lineno, colno)
    {
    }

    virtual void accept(ParseTreeNodeVisitor *visitor) override;

    const char *name() override;
};

#endif
