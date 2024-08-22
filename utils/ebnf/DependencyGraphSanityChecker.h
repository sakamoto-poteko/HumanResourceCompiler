#ifndef DEPENDENCYGRAPHSANITYCHECKER_H
#define DEPENDENCYGRAPHSANITYCHECKER_H

#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>

#include <boost/graph/directed_graph.hpp>

#include "ASTNodeVisitor.h"
#include "DependencyGraphBuilder.h"

template <class T> struct InfoWithLoc {
    int row;
    int col;
    T info;

    InfoWithLoc(int row, int col, T info) : row(row), col(col), info(info)
    {
    }
};

class DependencyGraphSanityChecker : protected ASTNodeVisitor {
  public:
    using Graph = boost::directed_graph<ASTNodePtr>;
    using Vertex = Graph::vertex_descriptor;
    using Edge = Graph::edge_descriptor;

    DependencyGraphSanityChecker(const boost::directed_graph<ASTNodePtr> &graph,
                                 std::set<std::string> tokens,
                                 const std::string &root_syntax_name)
        : _graph(graph), _tokens(tokens), _root_symbol_name(root_syntax_name)
    {
    }

    ~DependencyGraphSanityChecker()
    {
    }

    /*
    Checks for
    1. Unreachable nodes
    2. Left recursion
    3. FIRST/FIRST conflict
    */
    virtual bool check();

    bool get_cicrular_dependency(
        std::vector<InfoWithLoc<std::pair<ProductionNodePtr, IdentifierNodePtr>>> &circular)
    {
        if (_state) {
            circular = _state->circular;
            return true;
        }
        else {
            return false;
        }
    }

    bool get_unreachable(std::vector<InfoWithLoc<ProductionNodePtr>> &unreachable)
    {
        if (_state) {
            unreachable = _state->unreachable;
            return true;
        }
        else {
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

    virtual void soft_dfs(Vertex current, Vertex parent);

    const boost::directed_graph<ASTNodePtr> &_graph;
    std::set<std::string> _tokens;
    std::string _root_symbol_name;

    struct VisitState {
        std::map<std::string, Vertex> productions;
        Vertex root;

        std::set<Vertex> visited;
        std::set<Vertex> mark;
        std::vector<Vertex> topo;

        // circular from pair.second -> pair.first.
        std::vector<InfoWithLoc<std::pair<ProductionNodePtr, IdentifierNodePtr>>> circular;
        std::vector<InfoWithLoc<ProductionNodePtr>> unreachable;
    };
    std::unique_ptr<VisitState> _state;
};

#endif