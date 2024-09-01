#ifndef ASTNODEGRAPHVIZBUILDER_H
#define ASTNODEGRAPHVIZBUILDER_H

#include <stack>

#include <boost/graph/directed_graph.hpp>
#include <string>

#include "ASTNode.h"
#include "ASTNodeForward.h"
#include "ASTNodeVisitor.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

class ASTNodeGraphvizBuilder : public ASTNodeVisitor {
public:
    ASTNodeGraphvizBuilder(const CompilationUnitNodePtr &root);
    virtual ~ASTNodeGraphvizBuilder();

    struct NodeProperty {
        std::string name;
        std::string value;
        bool terminal;
    };

    using Graph = boost::directed_graph<NodeProperty>;
    using Vertex = Graph::vertex_descriptor;

    virtual std::string generate_graphviz();

    virtual void visit(IdentifierNodePtr node) override;
    virtual void visit(IntegerLiteralNodePtr node) override;
    virtual void visit(BooleanLiteralNodePtr node) override;
    virtual void visit(BinaryOperatorNodePtr node) override;
    virtual void visit(VariableDeclarationNodePtr node) override;
    virtual void visit(VariableAssignmentNodePtr node) override;
    virtual void visit(FloorAssignmentNodePtr node) override;
    virtual void visit(BinaryExpressionNodePtr node) override;
    virtual void visit(NegativeExpressionNodePtr node) override;
    virtual void visit(PositiveExpressionNodePtr node) override;
    virtual void visit(NotExpressionNodePtr node) override;
    virtual void visit(IncrementExpressionNodePtr node) override;
    virtual void visit(DecrementExpressionNodePtr node) override;
    virtual void visit(FloorAccessNodePtr node) override;
    virtual void visit(ParenthesizedExpressionNodePtr node) override;
    virtual void visit(InvocationExpressionNodePtr node) override;
    virtual void visit(IfStatementNodePtr node) override;
    virtual void visit(WhileStatementNodePtr node) override;
    virtual void visit(ForStatementNodePtr node) override;
    virtual void visit(ReturnStatementNodePtr node) override;
    virtual void visit(FloorBoxInitStatementNodePtr node) override;
    virtual void visit(FloorMaxInitStatementNodePtr node) override;
    virtual void visit(EmptyStatementNodePtr node) override;
    virtual void visit(StatementBlockNodePtr node) override;
    virtual void visit(VariableDeclarationStatementNodePtr node) override;
    virtual void visit(VariableAssignmentStatementNodePtr node) override;
    virtual void visit(FloorAssignmentStatementNodePtr node) override;
    virtual void visit(SubprocDefinitionNodePtr node) override;
    virtual void visit(FunctionDefinitionNodePtr node) override;
    virtual void visit(ImportDirectiveNodePtr node) override;
    virtual void visit(CompilationUnitNodePtr node) override;

protected:
    std::stack<Vertex> _ancestors;
    Graph _graph;
    CompilationUnitNodePtr _root;

    virtual Vertex enter_and_create_vertex(const std::string &name, bool terminal = false);
    virtual Vertex enter_and_create_vertex(const std::string &name, const std::string &value, bool terminal = false);
    virtual void leave();

    std::string escape_graphviz(const std::string &text);

    template <typename Container>
        requires std::ranges::range<Container> && convertible_to_ASTNodePtr<std::ranges::range_value_t<Container>>
    void traverse(const Container &nodes)
    {
        for (const auto &node : nodes) {
            node->accept(this);
        }
    }

private:
};

CLOSE_PARSER_NAMESPACE

#endif