#ifndef PARSE_TREE_NODE_GRAPHVIZBUILDER_H
#define PARSE_TREE_NODE_GRAPHVIZBUILDER_H

#include <stack>

#include <boost/graph/directed_graph.hpp>
#include <string>

#include "ParseTreeNode.h"
#include "ParseTreeNodeForward.h"
#include "ParseTreeNodeVisitor.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

class ParseTreeNodeGraphvizBuilder : public ParseTreeNodeVisitor {
public:
    ParseTreeNodeGraphvizBuilder(const CompilationUnitPTNodePtr &root);
    virtual ~ParseTreeNodeGraphvizBuilder();

    struct NodeProperty {
        std::string name;
        std::string value;
        bool terminal;
    };

    using Graph = boost::directed_graph<NodeProperty>;
    using Vertex = Graph::vertex_descriptor;
    using Edge = Graph::edge_descriptor;

    virtual std::string generate_graphviz();

    virtual void visit(IdentifierPTNodePtr node) override;
    virtual void visit(IntegerLiteralPTNodePtr node) override;
    virtual void visit(BooleanLiteralPTNodePtr node) override;
    virtual void visit(BinaryOperatorPTNodePtr node) override;
    virtual void visit(VariableDeclarationPTNodePtr node) override;
    virtual void visit(VariableAssignmentPTNodePtr node) override;
    virtual void visit(FloorAssignmentPTNodePtr node) override;
    virtual void visit(BinaryExpressionPTNodePtr node) override;
    virtual void visit(NegativeExpressionPTNodePtr node) override;
    virtual void visit(PositiveExpressionPTNodePtr node) override;
    virtual void visit(NotExpressionPTNodePtr node) override;
    virtual void visit(IncrementExpressionPTNodePtr node) override;
    virtual void visit(DecrementExpressionPTNodePtr node) override;
    virtual void visit(FloorAccessPTNodePtr node) override;
    virtual void visit(ParenthesizedExpressionPTNodePtr node) override;
    virtual void visit(InvocationExpressionPTNodePtr node) override;
    virtual void visit(IfStatementPTNodePtr node) override;
    virtual void visit(WhileStatementPTNodePtr node) override;
    virtual void visit(ForStatementPTNodePtr node) override;
    virtual void visit(ReturnStatementPTNodePtr node) override;
    virtual void visit(FloorBoxInitStatementPTNodePtr node) override;
    virtual void visit(FloorMaxInitStatementPTNodePtr node) override;
    virtual void visit(EmptyStatementPTNodePtr node) override;
    virtual void visit(StatementBlockPTNodePtr node) override;
    virtual void visit(VariableDeclarationStatementPTNodePtr node) override;
    virtual void visit(VariableAssignmentStatementPTNodePtr node) override;
    virtual void visit(FloorAssignmentStatementPTNodePtr node) override;
    virtual void visit(InvocationStatementPTNodePtr node) override;
    virtual void visit(SubprocDefinitionPTNodePtr node) override;
    virtual void visit(FunctionDefinitionPTNodePtr node) override;
    virtual void visit(ImportDirectivePTNodePtr node) override;
    virtual void visit(CompilationUnitPTNodePtr node) override;

protected:
    std::stack<Vertex> _ancestors;
    Graph _graph;
    CompilationUnitPTNodePtr _root;

    virtual Vertex enter_and_create_vertex(const std::string &name, bool terminal = false);
    virtual Vertex enter_and_create_vertex(const std::string &name, const std::string &value, bool terminal = false);
    virtual void leave();

    std::string escape_graphviz(const std::string &text);

    template <typename Container>
        requires std::ranges::range<Container> && convertible_to_ParseTreeNodePtr<std::ranges::range_value_t<Container>>
    void traverse(const Container &nodes)
    {
        for (const auto &node : nodes) {
            node->accept(this);
        }
    }

    template <typename T>
        requires convertible_to_ParseTreeNodePtr<T>
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