#ifndef ASTNODEGRAPHVIZBUILDER_H
#define ASTNODEGRAPHVIZBUILDER_H

#include <stack>
#include <string>
#include <vector>

#include <boost/graph/directed_graph.hpp>

#include "ASTNode.h"
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
        std::vector<std::string> attributes;
    };

    using Graph = boost::directed_graph<NodeProperty>;
    using Vertex = Graph::vertex_descriptor;
    using Edge = Graph::edge_descriptor;

    virtual std::string generate_graphviz(const std::string &filepath, const std::set<int> enabled_attributes = std::set<int>());

    int visit(const IntegerASTNodePtr &node) override;
    int visit(const BooleanASTNodePtr &node) override;
    int visit(const VariableDeclarationASTNodePtr &node) override;
    int visit(const VariableAssignmentASTNodePtr &node) override;
    int visit(const VariableAccessASTNodePtr &node) override;
    int visit(const FloorBoxInitStatementASTNodePtr &node) override;
    int visit(const FloorAssignmentASTNodePtr &node) override;
    int visit(const FloorAccessASTNodePtr &node) override;
    int visit(const NegativeExpressionASTNodePtr &node) override;
    int visit(const NotExpressionASTNodePtr &node) override;
    int visit(const IncrementExpressionASTNodePtr &node) override;
    int visit(const DecrementExpressionASTNodePtr &node) override;
    int visit(const AddExpressionASTNodePtr &node) override;
    int visit(const SubExpressionASTNodePtr &node) override;
    int visit(const MulExpressionASTNodePtr &node) override;
    int visit(const DivExpressionASTNodePtr &node) override;
    int visit(const ModExpressionASTNodePtr &node) override;
    int visit(const EqualExpressionASTNodePtr &node) override;
    int visit(const NotEqualExpressionASTNodePtr &node) override;
    int visit(const GreaterThanExpressionASTNodePtr &node) override;
    int visit(const GreaterEqualExpressionASTNodePtr &node) override;
    int visit(const LessThanExpressionASTNodePtr &node) override;
    int visit(const LessEqualExpressionASTNodePtr &node) override;
    int visit(const AndExpressionASTNodePtr &node) override;
    int visit(const OrExpressionASTNodePtr &node) override;
    int visit(const InvocationExpressionASTNodePtr &node) override;
    int visit(const EmptyStatementASTNodePtr &node) override;
    int visit(const IfStatementASTNodePtr &node) override;
    int visit(const WhileStatementASTNodePtr &node) override;
    int visit(const ForStatementASTNodePtr &node) override;
    int visit(const ReturnStatementASTNodePtr &node) override;
    int visit(const BreakStatementASTNodePtr &node) override;
    int visit(const ContinueStatementASTNodePtr &node) override;
    int visit(const StatementBlockASTNodePtr &node) override;
    int visit(const SubprocDefinitionASTNodePtr &node) override;
    int visit(const FunctionDefinitionASTNodePtr &node) override;
    int visit(const CompilationUnitASTNodePtr &node) override;

protected:
    std::stack<Vertex> _ancestors;
    std::set<int> _enabled_attributes;
    Graph _graph;
    CompilationUnitASTNodePtr _root;

    virtual Vertex enter_and_create_vertex(const std::string &label, NodeType type, const ASTNodePtr &node);
    virtual Vertex enter_and_create_vertex(const StringPtr &label, NodeType type, const ASTNodePtr &node);
    virtual int leave();

    template <typename Container>
        requires std::ranges::range<Container> && ConvertibleToASTNodePtr<std::ranges::range_value_t<Container>>
    void traverse(const Container &nodes)
    {
        for (const auto &node : nodes) {
            node->accept(this);
        }
    }

    template <typename T>
        requires ConvertibleToASTNodePtr<T>
    void traverse(const T &node)
    {
        if (node) {
            node->accept(this);
        }
    }

    static std::string escape_graphviz(const std::string &text);
    static std::string escape_graphviz_html(const std::string &text);

private:
};

CLOSE_PARSER_NAMESPACE

#endif