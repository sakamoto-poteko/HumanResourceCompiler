#ifndef DEPENDENCYGRAPHSANITYCHECKER_H
#define DEPENDENCYGRAPHSANITYCHECKER_H

#include <map>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <vector>

#include <boost/graph/directed_graph.hpp>

#include "ASTNodeForward.h"
#include "ASTNodeVisitor.h"

template <class T>
struct InfoWithLoc {
    int row;
    int col;
    T info;

    InfoWithLoc(int row, int col, T info)
        : row(row)
        , col(col)
        , info(info)
    {
    }
};

class DependencyGraphAnalyzer : protected ASTNodeVisitor {
public:
    using Graph = boost::directed_graph<ASTNodePtr>;
    using Vertex = Graph::vertex_descriptor;
    using Edge = Graph::edge_descriptor;

    DependencyGraphAnalyzer(const boost::directed_graph<ASTNodePtr> &graph,
        std::set<std::string> tokens,
        const std::string &root_syntax_name)
        : _graph(graph)
        , _tokens(tokens)
        , _root_symbol_name(root_syntax_name)
    {
    }

    ~DependencyGraphAnalyzer()
    {
    }

    /*
    Analyze for
    1. Unreachable nodes
    2. Left recursion
    3. FIRST/FIRST conflict
    Compute for
    1. FIRST set
    2. FOLLOW set
    */
    virtual bool analyze();

    virtual bool get_cicrular_dependency(std::vector<InfoWithLoc<std::pair<ProductionNodePtr, IdentifierNodePtr>>> &circular);

    virtual bool get_unreachable(std::vector<InfoWithLoc<ProductionNodePtr>> &unreachable);

    virtual bool get_topological_rule_order(std::vector<ProductionNodePtr> &order);

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

    enum DescentPosition {
        First,
        Follow,
        Other,
    };

    enum DescentType {
        ProductionRule,
        Repeated,
        Grouped,
        Optional,
    };

    virtual void soft_dfs(Vertex current, Vertex parent, DescentPosition descent_position);

    // virtual void left_recursion_dfs(Vertex current, Vertex parent, NodeType node_type);

    // Compute the first set of production rules. The sub rule's first set is not expanded
    virtual void compute_first_initial();
    // Compute the first set of production rules. It expands sub's first set.
    virtual void compute_first_expanded();

    const boost::directed_graph<ASTNodePtr> &_graph;
    std::set<std::string> _tokens;
    std::string _root_symbol_name;

    struct VisitState {
        std::map<std::string, ProductionNodePtr> productions;
        Vertex root;

        std::set<Vertex> visited;
        std::set<Vertex> mark;
        std::vector<Vertex> reversed_topo;

        std::vector<std::pair<ProductionNodePtr, DescentPosition>> descent_path;
        std::stack<DescentType> descent_type;

        // circular from pair.second -> pair.first.
        std::vector<InfoWithLoc<std::pair<ProductionNodePtr, IdentifierNodePtr>>> circular;
        std::vector<InfoWithLoc<ProductionNodePtr>> unreachable;
    };
    std::unique_ptr<VisitState> _state;
};

#endif