#ifndef ASTDEPGRAPHBUILDERVISITOR_H
#define ASTDEPGRAPHBUILDERVISITOR_H

#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>

#include <boost/graph/directed_graph.hpp>

#include "ASTNodeForward.h"
#include "ASTNodeVisitor.h"

class DependencyGraphBuilder : protected ASTNodeVisitor {
public:
    using Vertex = boost::directed_graph<ASTNodePtr>::vertex_descriptor;
    using Edge = boost::directed_graph<ASTNodePtr>::edge_descriptor;

    DependencyGraphBuilder(SyntaxNodePtr node, std::set<std::string> tokens,
        const std::string &root_syntax_name)
        : _root_node(node)
        , _tokens(tokens)
        , _root_syntax_name(root_syntax_name) {};

    virtual ~DependencyGraphBuilder()
    {
    }

    virtual bool build();
    virtual bool write_graphviz(const std::string &path);
    virtual bool get_dependency_graph(boost::directed_graph<ASTNodePtr> &graph)
    {
        if (_state) {
            graph = _state->dependency_graph;
            return true;
        } else {
            return false;
        }
    }

protected:
    virtual int accept(SyntaxNodePtr node) override;
    virtual int accept(ProductionNodePtr node) override;
    virtual int accept(ExpressionNodePtr node) override;
    virtual int accept(TermNodePtr node) override;
    virtual int accept(FactorNodePtr node) override;
    virtual int accept(OptionalNodePtr node) override;
    virtual int accept(RepeatedNodePtr node) override;
    virtual int accept(GroupedNodePtr node) override;
    virtual int accept(IdentifierNodePtr node) override;
    virtual int accept(LiteralNodePtr node) override;
    virtual int accept(EpsilonNodePtr node) override;

    SyntaxNodePtr _root_node;
    std::set<std::string> _tokens;
    std::string _root_syntax_name;

    struct VisitState {
        std::stack<Vertex> vertices;
        std::map<std::string, Vertex> productions;
        std::vector<std::string> warnings;
        std::string current_rule;
        boost::directed_graph<ASTNodePtr> dependency_graph;
    };

    std::unique_ptr<VisitState> _state;
};

#endif