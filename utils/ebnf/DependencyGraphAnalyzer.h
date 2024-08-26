#ifndef DEPENDENCYGRAPHSANITYCHECKER_H
#define DEPENDENCYGRAPHSANITYCHECKER_H

#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
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

struct FirstSetElement {
    enum Type {
        Literal,
        Epsilon,
        Token,
        Reference,
    };

    // ~FirstSetElement() { std::cout << "Destructing " << value << std::endl; }
    Type type;

    // value is literal value, token value referenced name of another production
    std::string value = "fuck";
    // std::string wwwww1 = "fuck";
    std::string wwwww = "fuck";
    std::string wwwwws = "fuck";
    std::string weird = "fuck";
    // produced_by is the production id after expansion
    std::string produced_by = "fuck";

    explicit FirstSetElement(const std::string &value, Type type)
        : value(value)
        , type(type)
    {
        std::cout << "f" << (void *)produced_by.c_str() << std::endl;
    }

    explicit FirstSetElement(const std::string &value, Type type, const std::string &produced_by)
        : value(value)
        , type(type)
        , produced_by(produced_by)
    {
        std::cout << "f" << std::endl;
    }

    FirstSetElement(const FirstSetElement &&v)
        : value(std::move(v.value))
        , type(v.type)
        , produced_by(std::move(v.produced_by))
    {
    }

    static std::string type_str(Type type)
    {
        switch (type) {
        case Literal:
            return "Literal";
        case Epsilon:
            return "Epsilon";
        case Reference:
            return "Reference";
        case Token:
            return "Token";
        }
        return "ERROR";
    }

    bool operator<(const FirstSetElement &other) const
    {
        // if (type == other.type) {
        //     return value < other.value;
        // }
        return type < other.type;
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

protected:
    virtual void soft_dfs(Vertex current, Vertex parent);

    virtual void find_unreachable();
    virtual void build_production_rule_map();
    virtual void expand_first_set();

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
        // circular from pair.first -> pair.second.
        std::vector<InfoWithLoc<std::pair<ProductionNodePtr, ProductionNodePtr>>> non_left_circular;
        std::vector<InfoWithLoc<ProductionNodePtr>> unreachable;
    };

    VisitState *_state = nullptr;
    // std::shared_ptr<VisitState> _state;
    // std::unique_ptr<VisitState> _state;
};

#endif