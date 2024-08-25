#ifndef DEPENDENCYGRAPHSANITYCHECKER_H
#define DEPENDENCYGRAPHSANITYCHECKER_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <boost/graph/directed_graph.hpp>

#include "ASTNodeForward.h"

template <class T>
struct InfoWithLoc {
    int row;
    int col;
    int row2;
    int col2;
    std::string id;
    std::string id2;
    T info;

    InfoWithLoc(int row, int col, T info)
        : row(row)
        , col(col)
        , id()
        , row2(0)
        , col2(0)
        , id2()
        , info(info)
    {
    }

    InfoWithLoc(const std::string &id, int row, int col, T info)
        : row(row)
        , col(col)
        , id(id)
        , row2(0)
        , col2(0)
        , id2()
        , info(info)
    {
    }

    InfoWithLoc(int row, int col, int row2, int col2, T info)
        : row(row)
        , col(col)
        , row2(row2)
        , col2(col2)
        , info(info)
    {
    }

    InfoWithLoc(const std::string &id, int row, int col, const std::string &id2, int row2, int col2, T info)
        : row(row)
        , col(col)
        , id(id)
        , row2(row2)
        , col2(col2)
        , id2(id2)
        , info(info)
    {
    }
};

class DependencyGraphAnalyzer{
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

    virtual bool get_non_left_cicrular_dependency(std::vector<InfoWithLoc<std::pair<ProductionNodePtr, ProductionNodePtr>>> &circular);

    virtual bool get_left_recursion(std::vector<InfoWithLoc<std::pair<ProductionNodePtr, std::vector<std::string>>>> &left_recursion);

    virtual bool get_unreachable(std::vector<InfoWithLoc<ProductionNodePtr>> &unreachable);

    virtual bool get_topological_rule_order(std::vector<ProductionNodePtr> &order);

protected:
    virtual void soft_dfs(Vertex current, Vertex parent);

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

        std::vector<ProductionNodePtr> descent_path;
        std::vector<std::pair<ProductionNodePtr, int>> descent_path_edge_indices;

        // left recursion <current node, path>
        std::vector<InfoWithLoc<std::pair<ProductionNodePtr, std::vector<std::string>>>> left_recursion;
        // circular from pair.first -> pair.second.
        std::vector<InfoWithLoc<std::pair<ProductionNodePtr, ProductionNodePtr>>> non_left_circular;
        std::vector<InfoWithLoc<ProductionNodePtr>> unreachable;
    };
    std::unique_ptr<VisitState> _state;
};

#endif