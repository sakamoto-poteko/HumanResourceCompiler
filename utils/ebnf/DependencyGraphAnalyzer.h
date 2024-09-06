#ifndef DEPENDENCYGRAPHSANITYCHECKER_H
#define DEPENDENCYGRAPHSANITYCHECKER_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <boost/graph/directed_graph.hpp>

#include "ASTNodeForward.h"
#include "FirstFollowElement.h"

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
        , row2(0)
        , col2(0)
        , id(id)
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

    bool operator<(const InfoWithLoc &other) const
    {
        // Compare based on row first
        if (row != other.row) {
            return row < other.row;
        }
        // Then compare based on col
        if (col != other.col) {
            return col < other.col;
        }
        // Then compare based on id
        if (id != other.id) {
            return id < other.id;
        }

        // Then compare based on row2
        if (row2 != other.row2) {
            return row2 < other.row2;
        }
        // Then compare based on col2
        if (col2 != other.col2) {
            return col2 < other.col2;
        }
        // Then compare based on id2
        if (id2 != other.id2) {
            return id2 < other.id2;
        }

        // Finally compare based on info
        return info < other.info;
    }
};

class DependencyGraphAnalyzer {
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

    virtual bool get_first_set(std::map<std::string, std::set<FirstSetElement>> &firsts);

    virtual bool get_first_first_conflicts(std::map<std::string, std::set<std::string>> &conflicts);

protected:
    virtual void soft_dfs(Vertex current, Vertex parent);

    virtual void find_unreachable();
    virtual void build_production_rule_map();
    virtual void expand_first_set();
    virtual void analyze_first_first_conflict();
    virtual void compute_follow_set();

    const boost::directed_graph<ASTNodePtr> &_graph;
    std::set<std::string> _tokens;
    std::string _root_symbol_name;

    struct VisitState {
        std::map<std::string, ProductionNodePtr> productions;
        Vertex root;

        std::set<Vertex> visited;
        std::set<Vertex> mark;
        std::vector<Vertex> reversed_topo;
        std::set<std::string> left_recursion_production_id;

        std::vector<ProductionNodePtr> descent_path;
        std::vector<std::pair<ProductionNodePtr, int>> descent_path_edge_indices;

        // map<production id, [element]>
        std::map<std::string, std::set<FirstSetElement>> first_set;

        // left recursion <current node, path>
        std::vector<InfoWithLoc<std::pair<ProductionNodePtr, std::vector<std::string>>>> left_recursion;
        std::set<InfoWithLoc<std::pair<ProductionNodePtr, std::vector<std::string>>>> left_recursion_dedup;
        // circular from pair.first -> pair.second.
        std::vector<InfoWithLoc<std::pair<ProductionNodePtr, ProductionNodePtr>>> non_left_circular;
        std::set<InfoWithLoc<std::pair<ProductionNodePtr, ProductionNodePtr>>> non_left_circular_dedup;
        std::vector<InfoWithLoc<ProductionNodePtr>> unreachable;

        // <FIRST, [rule id]>
        std::map<std::string, std::set<std::string>> first_first_conflicts;
    };

    std::unique_ptr<VisitState> _state;
};

#endif