#ifndef ASTNODE_H
#define ASTNODE_H

#include <memory>
#include <string>
#include <vector>

#include "ASTNodeForward.h" // IWYU pragma: export

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

class SyntaxNode : public ASTNode {
public:
    std::vector<ProductionNodePtr> productions;

    explicit SyntaxNode(int lineno, int colno)
        : ASTNode(lineno, colno)
    {
    }

    virtual void accept(ASTNodeVisitor *visitor) override;

    const char *name() override
    {
        return "Syntax";
    }
};

// Production node
class ProductionNode : public ASTNode {
public:
    std::string id;
    ExpressionNodePtr expression;

    ProductionNode(std::string id, ExpressionNodePtr expr, int lineno,
        int colno)
        : ASTNode(lineno, colno)
        , id(std::move(id))
        , expression(std::move(expr))
    {
    }

    virtual void accept(ASTNodeVisitor *visitor) override;

    const char *name() override
    {
        return "Production";
    }
};

// Expression node
class ExpressionNode : public ASTNode {
public:
    std::vector<TermNodePtr> terms;
    explicit ExpressionNode(int lineno, int colno)
        : ASTNode(lineno, colno)
    {
    }

    void addTerm(TermNodePtr term)
    {
        terms.push_back(std::move(term));
    }

    virtual void accept(ASTNodeVisitor *visitor) override;

    const char *name() override
    {
        return "Expression";
    }
};

// Term node
class TermNode : public ASTNode {
public:
    std::vector<FactorNodePtr> factors;
    explicit TermNode(int lineno, int colno)
        : ASTNode(lineno, colno)
    {
    }

    void addFactor(FactorNodePtr factor)
    {
        factors.push_back(std::move(factor));
    }

    virtual void accept(ASTNodeVisitor *visitor) override;

    const char *name() override
    {
        return "Term";
    }
};

// Factor node
class FactorNode : public ASTNode {
public:
    ASTNodePtr node;
    LiteralNodePtr literal;
    IdentifierNodePtr identifier;

    explicit FactorNode(IdentifierNodePtr id, int lineno, int colno)
        : ASTNode(lineno, colno)
        , identifier(id)
    {
    }
    explicit FactorNode(LiteralNodePtr lit, int lineno, int colno)
        : ASTNode(lineno, colno)
        , literal(lit)
    {
    }
    explicit FactorNode(ASTNodePtr value, int lineno, int colno)
        : ASTNode(lineno, colno)
        , node(value)
    {
    }

    virtual void accept(ASTNodeVisitor *visitor) override;

    const char *name() override
    {
        return "Factor";
    }
};

class IdentifierNode : public ASTNode {
public:
    std::string value;

    explicit IdentifierNode(std::string value, int lineno, int colno)
        : ASTNode(lineno, colno)
        , value(std::move(value))
    {
    }

    virtual void accept(ASTNodeVisitor *visitor) override;

    const char *name() override
    {
        return "Identifier";
    }
};

class LiteralNode : public ASTNode {
public:
    std::string value;

    explicit LiteralNode(std::string value, int lineno, int colno)
        : ASTNode(lineno, colno)
        , value(std::move(value))
    {
    }

    virtual void accept(ASTNodeVisitor *visitor) override;

    const char *name() override
    {
        return "Literal";
    }
};

// Optional node
class OptionalNode : public ASTNode {
public:
    ASTNodePtr expression;

    explicit OptionalNode(ASTNodePtr expr, int lineno, int colno)
        : ASTNode(lineno, colno)
        , expression(std::move(expr))
    {
    }

    virtual void accept(ASTNodeVisitor *visitor) override;

    const char *name() override
    {
        return "Optional";
    }
};

// Repeated node
class RepeatedNode : public ASTNode {
public:
    ASTNodePtr expression;

    explicit RepeatedNode(ASTNodePtr expr, int lineno, int colno)
        : ASTNode(lineno, colno)
        , expression(std::move(expr))
    {
    }

    virtual void accept(ASTNodeVisitor *visitor) override;

    const char *name() override
    {
        return "Optional";
    }
};

// Grouped node
class GroupedNode : public ASTNode {
public:
    ASTNodePtr expression;

    explicit GroupedNode(ASTNodePtr expr, int lineno, int colno)
        : ASTNode(lineno, colno)
        , expression(std::move(expr))
    {
    }

    virtual void accept(ASTNodeVisitor *visitor) override;

    const char *name() override
    {
        return "Grouped";
    }
};

#endif
