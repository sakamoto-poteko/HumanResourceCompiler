#include <iostream>
#include <map>
#include <stack>
#include <string>

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
    soft_dfs(_state->root, Graph::null_vertex());

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
    left_recursion_dfs(_state->root, Graph::null_vertex(), NodeType::None);

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

void DependencyGraphAnalyzer::soft_dfs(Vertex current, Vertex parent)
{
    const auto &node = _graph[current];

    if (_state->visited.find(current) != _state->visited.end()) {
        // already visited
        return;
    }

    if (_state->mark.find(current) != _state->mark.end()) {
        auto c = std::dynamic_pointer_cast<ProductionNode>(_graph[current]);
        auto p = std::dynamic_pointer_cast<IdentifierNode>(_graph[parent]);
        // circular dependency
        if (!c || !p) {
            throw;
            // TODO: this should not happen. our codepath only allow
            // current=ProductionNodePtr and parent=IdentifierNodePtr.
            // This limit is set in builder, because I built this dependency
            // myself!
        }
        // use the lineno for rule that's referencing it may be better
        _state->circular.push_back(InfoWithLoc(p->lineno(), p->colno(), std::make_pair(c, p)));
        return;
    }

    _state->mark.insert(current);

    auto out_edges_pair = boost::out_edges(current, _graph);
    for (auto it = out_edges_pair.first; it != out_edges_pair.second; ++it) {
        Vertex target = boost::target(*it, _graph);

        soft_dfs(target, current);
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

std::set<DependencyGraphAnalyzer::Vertex> _left_visited;
std::set<DependencyGraphAnalyzer::Vertex> _left_mark;
std::set<std::string> _leftmost_path_set;
std::stack<ProductionNodePtr> _path;

void DependencyGraphAnalyzer::left_recursion_dfs(Vertex current, Vertex parent, NodeType node_type)
{
    const auto &node = _graph[current];

    if (_left_visited.find(current) != _left_visited.end()) {
        // already visited
        return;
    }

    if (_left_mark.find(current) != _left_mark.end()) {
        auto c = std::dynamic_pointer_cast<ProductionNode>(_graph[current]);
        auto p = std::dynamic_pointer_cast<IdentifierNode>(_graph[parent]);
        // circular dependency but not left
        // current (Id)'s prod (Prod) is referencing visited (Prod)
        if (!c || !p) {
            throw;
            // TODO: this should not happen. our codepath only allow
            // current=ProductionNodePtr and parent=IdentifierNodePtr.
            // This limit is set in builder, because I built this dependency
            // myself!
        }

        // use the lineno for rule that's referencing it may be better
        _state->circular.push_back(InfoWithLoc(p->lineno(), p->colno(), std::make_pair(c, p)));

        const auto &last_prod = _path.top();
        // it's last production rule because we haven't push our current prod into the stack
        std::cerr << "LRD: circular (non-left)" << last_prod->id << ":" << c->id << " referencing " << c->id << " on line " << last_prod->lineno() << std::endl;
        return;
    }

    _left_mark.insert(current);

    // let's push current node info
    if (auto c = std::dynamic_pointer_cast<ProductionNode>(_graph[current])) {
        _path.push(c);
    }
    _path_type.push(node_type);

    auto out_edges_pair = boost::out_edges(current, _graph);

    int count = 0;
    // FIXME: make sure this order is correct. Does BGL guarantee this?
    for (auto it = out_edges_pair.first; it != out_edges_pair.second; ++it, ++count) {
        Vertex target = boost::target(*it, _graph);

        NodeType descendent_type = NodeType::None;
        if (auto c = std::dynamic_pointer_cast<TermNode>(_graph[current])) {
            // Term node is responsible to hold first, second, third... elements in an alternative
            if (count == 0) {
                descendent_type = NodeType::First;
            } else if (count == 1) {
                descendent_type = NodeType::Follow;
            }
        }

        if (auto id = std::dynamic_pointer_cast<IdentifierNode>(_graph[current])) {
            // Before descending into the production rule at the identifier.
            // If the identifier is leftmost
            //      Add current prod rule's name into the set<leftmost path>
            //      Check if id's target is in the set<leftmost path> already?
            //      If in the set, it's left recursion. For dir left rec, we added the rule just now. For indir, it's added earlier.
            //      if not in the set, descend
            // After descent, remove id from set

            const auto &current_prod = _path.top();
            std::cerr << "LRD DBG: id " << id->value << " prod " << current_prod->id << std::endl;

            if (node_type == NodeType::First) {
                _leftmost_path_set.insert(current_prod->id);
            } else if (node_type == NodeType::Follow) {
                // Follow
            } else {
                // None
            }

            // we have a production rule's name same as this identifier, whose leftmost element led the way here
            if (_leftmost_path_set.find(id->value) != _leftmost_path_set.end()) {
                // this is left recursion
                // TODO: it's possible to print the whole path here.
                std::cerr << "LRD: left recursion detected " << current_prod->id << ":" << id->value
                          << " to " << id->value << "@ln " << _state->productions[id->value]->lineno() << std::endl;
            } else {
                // descend
                left_recursion_dfs(target, current, descendent_type);
            }

            if (node_type == NodeType::First) {
                _leftmost_path_set.erase(current_prod->id);
            } else if (node_type == NodeType::Follow) {
                // Follow
            } else {
                // None
            }

        } else {
            left_recursion_dfs(target, current, descendent_type);
        }
    }

    if (auto c = std::dynamic_pointer_cast<ProductionNode>(_graph[current])) {
        _path.pop();
    }
    _path_type.pop();

    _left_mark.erase(current);
    _left_visited.insert(current);
}

void DependencyGraphAnalyzer::compute_first_initial() { }

void DependencyGraphAnalyzer::compute_first_expanded() { }
