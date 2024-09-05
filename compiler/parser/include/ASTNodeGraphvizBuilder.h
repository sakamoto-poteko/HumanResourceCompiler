#ifndef ASTNODEGRAPHVIZBUILDER_H
#define ASTNODEGRAPHVIZBUILDER_H

#include <stack>
#include <string>

#include <boost/graph/directed_graph.hpp>

#include "ASTNode.h"
#include "ASTNodeForward.h"
#include "ASTNodeVisitor.h"
#include "hrl_global.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

class ASTNodeGraphvizBuilder : public ASTNodeVisitor {
public:
    explicit ASTNodeGraphvizBuilder(CompilationUnitASTNodePtr ast);
    ~ASTNodeGraphvizBuilder() override = default;

    enum NodeType {
        Literal,
        Expression,
        Statement,
        Structure,
        Operator,
        Flow,
    };

    struct NodeProperty {
        std::string label;
        NodeType type;
    };

    using Graph = boost::directed_graph<NodeProperty>;
    using Vertex = Graph::vertex_descriptor;
    using Edge = Graph::edge_descriptor;

    virtual std::string generate_graphviz();

    int visit(IntegerASTNodePtr node) override;
    int visit(BooleanASTNodePtr node) override;
    int visit(VariableDeclarationASTNodePtr node) override;
    int visit(VariableAssignmentASTNodePtr node) override;
    int visit(VariableAccessASTNodePtr node) override;
    int visit(FloorBoxInitStatementASTNodePtr node) override;
    int visit(FloorAssignmentASTNodePtr node) override;
    int visit(FloorAccessASTNodePtr node) override;
    int visit(NegativeExpressionASTNodePtr node) override;
    int visit(NotExpressionASTNodePtr node) override;
    int visit(IncrementExpressionASTNodePtr node) override;
    int visit(DecrementExpressionASTNodePtr node) override;
    int visit(AddExpressionASTNodePtr node) override;
    int visit(SubExpressionASTNodePtr node) override;
    int visit(MulExpressionASTNodePtr node) override;
    int visit(DivExpressionASTNodePtr node) override;
    int visit(ModExpressionASTNodePtr node) override;
    int visit(EqualExpressionASTNodePtr node) override;
    int visit(NotEqualExpressionASTNodePtr node) override;
    int visit(GreaterThanExpressionASTNodePtr node) override;
    int visit(GreaterEqualExpressionASTNodePtr node) override;
    int visit(LessThanExpressionASTNodePtr node) override;
    int visit(LessEqualExpressionASTNodePtr node) override;
    int visit(AndExpressionASTNodePtr node) override;
    int visit(OrExpressionASTNodePtr node) override;
    int visit(InvocationExpressionASTNodePtr node) override;
    int visit(EmptyStatementASTNodePtr node) override;
    int visit(IfStatementASTNodePtr node) override;
    int visit(WhileStatementASTNodePtr node) override;
    int visit(ForStatementASTNodePtr node) override;
    int visit(ReturnStatementASTNodePtr node) override;
    int visit(BreakStatementASTNodePtr node) override;
    int visit(ContinueStatementASTNodePtr node) override;
    int visit(StatementBlockASTNodePtr node) override;
    int visit(SubprocDefinitionASTNodePtr node) override;
    int visit(FunctionDefinitionASTNodePtr node) override;
    int visit(CompilationUnitASTNodePtr node) override;

protected:
    std::stack<Vertex> _ancestors;
    Graph _graph;
    CompilationUnitASTNodePtr _root;

    virtual Vertex enter_and_create_vertex(const std::string &label, NodeType type);
    virtual Vertex enter_and_create_vertex(const StringPtr &label, NodeType type);
    virtual int leave();

    template <typename Container>
        requires std::ranges::range<Container> && convertible_to_ASTNodePtr<std::ranges::range_value_t<Container>>
    void traverse(const Container &nodes)
    {
        for (const auto &node : nodes) {
            node->accept(this);
        }
    }

    template <typename T>
        requires convertible_to_ASTNodePtr<T>
    void traverse(const T &node)
    {
        if (node) {
            node->accept(this);
        }
    }

    static std::string escape_graphviz(const std::string &text);

private:
};

CLOSE_PARSER_NAMESPACE

#endif