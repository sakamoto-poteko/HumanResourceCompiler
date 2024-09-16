#ifndef PARSE_TREE_NODE_GRAPHVIZBUILDER_H
#define PARSE_TREE_NODE_GRAPHVIZBUILDER_H

#include <stack>
#include <string>

#include <boost/graph/directed_graph.hpp>

#include "ParseTreeNode.h"
#include "ParseTreeNodeForward.h"
#include "ParseTreeNodeVisitor.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

class ParseTreeNodeGraphvizBuilder : public ParseTreeNodeVisitor {
public:
    explicit ParseTreeNodeGraphvizBuilder(CompilationUnitPTNodePtr root);
    ~ParseTreeNodeGraphvizBuilder() override = default;

    struct NodeProperty {
        std::string name;
        std::string value;
        bool terminal;
    };

    using Graph = boost::directed_graph<NodeProperty>;
    using Vertex = Graph::vertex_descriptor;
    using Edge = Graph::edge_descriptor;

    virtual std::string generate_graphviz(const std::string &path);

    void visit(IdentifierPTNodePtr node) override;
    void visit(IntegerLiteralPTNodePtr node) override;
    void visit(BooleanLiteralPTNodePtr node) override;
    void visit(BinaryOperatorPTNodePtr node) override;
    void visit(VariableDeclarationPTNodePtr node) override;
    void visit(VariableAssignmentPTNodePtr node) override;
    void visit(FloorAssignmentPTNodePtr node) override;
    void visit(BinaryExpressionPTNodePtr node) override;
    void visit(NegativeExpressionPTNodePtr node) override;
    void visit(PositiveExpressionPTNodePtr node) override;
    void visit(NotExpressionPTNodePtr node) override;
    void visit(IncrementExpressionPTNodePtr node) override;
    void visit(DecrementExpressionPTNodePtr node) override;
    void visit(FloorAccessPTNodePtr node) override;
    void visit(ParenthesizedExpressionPTNodePtr node) override;
    void visit(InvocationExpressionPTNodePtr node) override;
    void visit(IfStatementPTNodePtr node) override;
    void visit(WhileStatementPTNodePtr node) override;
    void visit(ForStatementPTNodePtr node) override;
    void visit(ReturnStatementPTNodePtr node) override;
    void visit(BreakContinueStatementPTNodePtr node) override;
    void visit(FloorBoxInitStatementPTNodePtr node) override;
    void visit(FloorMaxInitStatementPTNodePtr node) override;
    void visit(EmptyStatementPTNodePtr node) override;
    void visit(StatementBlockPTNodePtr node) override;
    void visit(VariableDeclarationStatementPTNodePtr node) override;
    void visit(VariableAssignmentStatementPTNodePtr node) override;
    void visit(FloorAssignmentStatementPTNodePtr node) override;
    void visit(InvocationStatementPTNodePtr node) override;
    void visit(SubprocDefinitionPTNodePtr node) override;
    void visit(FunctionDefinitionPTNodePtr node) override;
    void visit(ImportDirectivePTNodePtr node) override;
    void visit(CompilationUnitPTNodePtr node) override;

protected:
    std::stack<Vertex> _ancestors;
    Graph _graph;
    CompilationUnitPTNodePtr _root;

    Vertex enter_and_create_vertex(const std::string &name, bool terminal = false);
    Vertex enter_and_create_vertex(const std::string &name, const std::string &value, bool terminal = false);
    virtual void leave();

    static std::string escape_graphviz(const std::string &text);

    template <typename Container>
        requires std::ranges::range<Container> && ConvertibleToParseTreeNodePtr<std::ranges::range_value_t<Container>>
    void traverse(const Container &nodes)
    {
        for (const auto &node : nodes) {
            node->accept(this);
        }
    }

    template <typename T>
        requires ConvertibleToParseTreeNodePtr<T>
    void traverse(const T &node)
    {
        if (node) {
            node->accept(this);
        }
    }

private:
};

CLOSE_PARSER_NAMESPACE

#endif