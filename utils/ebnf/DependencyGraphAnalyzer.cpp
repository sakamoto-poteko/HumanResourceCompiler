#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "ASTNode.h"
#include "ASTNodeForward.h"
#include "DependencyGraphAnalyzer.h"

bool DependencyGraphAnalyzer::analyze()
{
    _state = std::make_unique<VisitState>();

    for (int i = 0; i < _graph.num_vertices(); i++) {
        Vertex vertex = boost::vertex(i, _graph);
        const auto &current = _graph[vertex];
        if (auto ptr = std::dynamic_pointer_cast<ProductionNode>(current)) {
            _state->productions[ptr->id] = ptr;

            if (ptr->id == _root_symbol_name) {
                _state->root = vertex;
            }
        }
    }

    if (_state->root == Graph::null_vertex()) {
        throw; // TODO: err msg root syntax does not exist
    }

    // perform DFS with circular dependency check for all nodes
    soft_dfs(_state->root, Graph::null_vertex(), DescentPosition::Other);

    // is there any nodes untraversed?
    for (int i = 0; i < _graph.num_vertices(); i++) {
        Vertex vertex = boost::vertex(i, _graph);
        if (_state->visited.find(vertex) == _state->visited.end()) {
            const auto &current = _graph[vertex];
            if (auto ptr = std::dynamic_pointer_cast<ProductionNode>(current)) {
                // unreachable production only. child elements of unreachable
                // are ignored as user doesn't care.
                _state->unreachable.push_back(
                    InfoWithLoc(current->lineno(), current->colno(), ptr));
            }
        }
    }

    // find left recursion
    // left_recursion_dfs(_state->root, Graph::null_vertex(), NodeType::None);

    return false;
}

int DependencyGraphAnalyzer::accept(SyntaxNodePtr node)
{
    return 0;
}

int DependencyGraphAnalyzer::accept(ProductionNodePtr node)
{
    return 0;
}

int DependencyGraphAnalyzer::accept(ExpressionNodePtr node)
{
    return 0;
}

int DependencyGraphAnalyzer::accept(TermNodePtr node)
{
    return 0;
}

int DependencyGraphAnalyzer::accept(RepeatedNodePtr node)
{
    return 0;
}

int DependencyGraphAnalyzer::accept(FactorNodePtr node)
{
    return 0;
}

int DependencyGraphAnalyzer::accept(OptionalNodePtr node)
{
    return 0;
}

int DependencyGraphAnalyzer::accept(GroupedNodePtr node)
{
    return 0;
}

int DependencyGraphAnalyzer::accept(IdentifierNodePtr node)
{
    return 0;
}

int DependencyGraphAnalyzer::accept(LiteralNodePtr node)
{
    return 0;
}

void DependencyGraphAnalyzer::soft_dfs(Vertex current, Vertex parent, DescentPosition descent_position)
{
    const auto &current_node = _graph[current];

    // already visited
    if (_state->visited.find(current) != _state->visited.end()) {
        return;
    }

    // circular dependency, including left recursion
    if (_state->mark.find(current) != _state->mark.end()) {
        auto current_production = std::dynamic_pointer_cast<ProductionNode>(current_node);
        auto previous_identifier = std::dynamic_pointer_cast<IdentifierNode>(_graph[parent]);

        if (!current_production || !previous_identifier) {
            throw;
            // TODO: this should not happen. our codepath only allow
            // current=ProductionNodePtr and parent=IdentifierNodePtr.
            // This limit is set in builder, because I built this dependency
            // myself!
        }
        // use the lineno for rule that's referencing it may be better
        _state->circular.push_back(InfoWithLoc(previous_identifier->lineno(), previous_identifier->colno(), std::make_pair(current_production, previous_identifier)));

        const auto &last_descent = _state->descent_path.back();
        const auto &last_production = last_descent.first; // it's last production rule because we haven't push our current production into it yet

        // determine whether it's left recursion
        // let's traverse our descent path backwards.
        // it's a left recursion iif all path (loop part only) is derived from leftmost element
        bool is_left_recursion = true;
        std::vector<std::string> path;
        // don't forget our current descent! it's not pushed yet
        if (descent_position == DescentPosition::First) {
            for (auto it = _state->descent_path.crbegin(); it != _state->descent_path.crend() && it->first->id != current_production->id; ++it) {
                if (it->second != DescentPosition::First) {
                    // found a non-leftmost derivation on our path
                    is_left_recursion = false;
                    break;
                }

                path.push_back(it->first->id);
            }
        } else {
            is_left_recursion = false;
        }

        if (is_left_recursion) {
            std::cerr << "LRD LLLLLLLL: left recursion:" << last_production->id << ":" << current_production->id << " referencing " << current_production->id << " on line " << last_production->lineno() << std::endl;
            std::cerr << "path: " << current_production->id;
            for (const std::string &p : path) {
                std::cerr << "<-" << p;
            }
            std::cerr << "<-" << current_production->id << std::endl;
        } else {
            std::cerr << "LRD: circular (non-left):" << last_production->id << ":" << current_production->id << " referencing " << current_production->id << " on line " << last_production->lineno() << std::endl;
        }

        return;
    }

    _state->mark.insert(current);

    // let's push current production rule info
    if (auto c = std::dynamic_pointer_cast<ProductionNode>(current_node)) {
        _state->descent_path.push_back(std::make_pair(c, descent_position));
        _state->descent_type.push(DescentType::ProductionRule);
    } else if (auto c = std::dynamic_pointer_cast<OptionalNode>(current_node)) {
        _state->descent_type.push(DescentType::Optional);
    } else if (auto c = std::dynamic_pointer_cast<RepeatedNode>(current_node)) {
        _state->descent_type.push(DescentType::Repeated);
    } else if (auto c = std::dynamic_pointer_cast<GroupedNode>(current_node)) {
        _state->descent_type.push(DescentType::Grouped);
    }

    auto out_edges_pair = boost::out_edges(current, _graph);
    int count = 0;

    // NOTE: it's assumed that BGL guarantee the order of edge traversal - in the order we added them
    for (auto it = out_edges_pair.first; it != out_edges_pair.second; ++it, ++count) {
        Vertex target = boost::target(*it, _graph);

        // FIXME: special handling of Repeated/Optional/Grouped is missing
        // the descending order is Expr -> Term [-> Repeated/Optional/Grouped -> Expr -> Term] -> Identifier -> Production
        // Term node is responsible to hold first, second, third... elements in an alternative
        // the Production node requires the descent position info
        // so Identifier node has to inherit the info from Term and pass it to Production
        DescentPosition descendent_position = descent_position;
        // if we encountered a special descent like Repeated/Optional/Grouped, we shoud not look up terms order (DescentPosition) of it
        // instead, we need to use the position before entering special descent
        if (_state->descent_type.top() == DescentType::ProductionRule) {
            // only set the position if it's Production Rule derived. if it's Repeated/Optional/Grouped derived, we're inheriting previous.
            if (auto c = std::dynamic_pointer_cast<TermNode>(current_node)) {
                if (count == 0) {
                    descendent_position = DescentPosition::First;
                } else if (count == 1) {
                    descendent_position = DescentPosition::Follow;
                } else {
                    descendent_position = DescentPosition::Other;
                }
            }
            // else if (auto c = std::dynamic_pointer_cast<IdentifierNode>(current_node)) {
            //     descendent_type = descent_type;
            // }
        }

        soft_dfs(target, current, descendent_position);
    }

    if (auto c = std::dynamic_pointer_cast<ProductionNode>(current_node)) {
        _state->descent_path.pop_back();
        _state->descent_type.pop();
    } else if (auto c = std::dynamic_pointer_cast<OptionalNode>(current_node)) {
        _state->descent_type.pop();
    } else if (auto c = std::dynamic_pointer_cast<RepeatedNode>(current_node)) {
        _state->descent_type.pop();
    } else if (auto c = std::dynamic_pointer_cast<GroupedNode>(current_node)) {
        _state->descent_type.pop();
    }

    _state->mark.erase(current);
    _state->visited.insert(current);
    _state->reversed_topo.push_back(current);
}

bool DependencyGraphAnalyzer::get_cicrular_dependency(std::vector<InfoWithLoc<std::pair<ProductionNodePtr, IdentifierNodePtr>>> &circular)
{
    if (_state) {
        circular = _state->circular;
        return true;
    } else {
        return false;
    }
}

bool DependencyGraphAnalyzer::get_unreachable(std::vector<InfoWithLoc<ProductionNodePtr>> &unreachable)
{
    if (_state) {
        unreachable = _state->unreachable;
        return true;
    } else {
        return false;
    }
}

bool DependencyGraphAnalyzer::get_topological_rule_order(std::vector<ProductionNodePtr> &order)
{
    if (_state) {
        order.clear();
        for (auto it = _state->reversed_topo.rbegin(); it != _state->reversed_topo.rend(); ++it) {
            const ASTNodePtr &node = _graph[*it];
            if (ProductionNodePtr production = std::dynamic_pointer_cast<ProductionNode>(node)) {
                order.push_back(production);
            }
        }
        return true;
    } else {
        return false;
    }
}

void DependencyGraphAnalyzer::compute_first_initial() { }

void DependencyGraphAnalyzer::compute_first_expanded() { }
