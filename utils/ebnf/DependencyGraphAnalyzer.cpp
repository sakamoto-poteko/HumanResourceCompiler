#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/format.hpp>

#include <spdlog/spdlog.h>

#include "ASTNode.h"
#include "ASTNodeForward.h"
#include "DependencyGraphAnalyzer.h"

void DependencyGraphAnalyzer::find_unreachable()
{
    for (int i = 0; i < _graph.num_vertices(); i++) {
        Vertex vertex = boost::vertex(i, _graph);
        if (_state->visited.find(vertex) == _state->visited.end()) {
            const auto &current = _graph[vertex];
            if (auto ptr = std::dynamic_pointer_cast<ProductionNode>(current)) {
                // unreachable production only
                // child elements of unreachable are ignored as user doesn't care.
                _state->unreachable.push_back(InfoWithLoc(ptr->id, ptr->lineno(), ptr->colno(), ptr));
            }
        }
    }
}

void DependencyGraphAnalyzer::build_production_rule_map()
{
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
        spdlog::critical("Root symbol '{0}' was not found", _root_symbol_name);
        std::exit(EXIT_FAILURE);
    }
}

void DependencyGraphAnalyzer::expand_first_set()
{
    // for (auto &order : _state->reversed_topo) {
    //     if (ProductionNodePtr production = std::dynamic_pointer_cast<ProductionNode>(_graph[order])) {
    //         // in reversed topological order

    //         auto &current_first_set = _state->first_set[production->id];

    //         // Skip left recursion element
    //         if (_state->left_recursion_production_id.find(production->id) != _state->left_recursion_production_id.end()) {
    //             current_first_set.clear();
    //             continue;
    //         }

    //         std::vector<FirstSetElement> expansion_required;
    //         // get all referenced FIRST in a production
    //         for (const auto &first : current_first_set) {
    //             if (first.type == FirstSetElement::Reference) {
    //                 expansion_required.push_back(first);
    //             }
    //         }

    //         // expand referenced FIRST, and remove the `reference` node
    //         for (const auto &ref_element : expansion_required) {
    //             // if any of FIRST is referencing a left recursion, we skip this one
    //             if (_state->left_recursion_production_id.find(production->id) != _state->left_recursion_production_id.end()) {
    //                 current_first_set.clear();
    //                 break;
    //             }
    //             const auto &referenced_first_set = _state->first_set[ref_element.value];
    //             // for each FIRST in the referenced rule, construct ours, populating the produced_by field
    //             for (const auto &ref_first : referenced_first_set) {

    //                 auto our_first = FirstSetElement(ref_first.value, ref_first.type, ref_element.value);
    //                 current_first_set.insert(our_first);
    //                 current_first_set.erase(ref_element);
    //             }
    //         }
    //     }
    // }

    // for (const auto &entry : _state->first_set) {
    //     std::cout << entry.first << ": ";
    //     for (const auto &f : entry.second) {
    //         std::cout << FirstSetElement::type_str(f.type) << "(" << f.value << "-" << f.produced_by << "), ";
    //     }
    //     std::cout << std::endl;
    // }
}

bool DependencyGraphAnalyzer::analyze()
{
    // _state = std::make_unique<VisitState>();
    _state = new VisitState;

    // Fill the ProductionNode map with rule id
    build_production_rule_map();

    // Perform DFS with:
    //   circular dependency check for all nodes
    //   initial FIRST set construction
    soft_dfs(_state->root, Graph::null_vertex());
    // expand_first_set();

    // Are there any nodes untraversed?
    find_unreachable();
    delete _state;
    _state = nullptr;
    return true;
}

void DependencyGraphAnalyzer::soft_dfs(Vertex current, Vertex parent)
{
    // Comments for Left Recursion Finder starts with *LRF*

    const auto &current_node = _graph[current];

    // Already visited
    if (_state->visited.find(current) != _state->visited.end()) {
        return;
    }

    // Circular dependency, including left recursion
    if (_state->mark.find(current) != _state->mark.end()) {
        auto current_production = std::dynamic_pointer_cast<ProductionNode>(current_node);
        auto previous_identifier = std::dynamic_pointer_cast<IdentifierNode>(_graph[parent]);

        if (!current_production || !previous_identifier) {
            // This limit exists because that's how builder works.
            spdlog::critical("ProductionNode can only have an IdentifierNode parent. This is a bug! Please consider report it.");
            std::exit(EXIT_FAILURE);
        }

        // It's *last* production rule because we haven't push our current production into it yet
        const auto &last_production = _state->descent_path.back();

        /*  LRF
            Determine whether it's left recursion. Traverse our descent path backwards.
            It's a left recursion iif all path (loop part only) is derived from leftmost-possible element (i.e., value is 0)
        */
        /*  LRF
            If there's a non-zero exits between the last and circular node, it's non-left recursion, otherwise it is
            There might be multiple path element with the same id, since we're pushing nodes even under Optional/Repeated/Grouped's Term node
        */
        std::vector<std::string> path;
        auto first_in_path_with_current_id = std::find_if(
            _state->descent_path_edge_indices.cbegin(),
            _state->descent_path_edge_indices.cend(),
            [&current_production](const std::pair<ProductionNodePtr, int> &element) {
                return element.first->id == current_production->id;
            });

        bool is_left_recursion = true;
        for (auto it = first_in_path_with_current_id; it != _state->descent_path_edge_indices.cend(); ++it) {
            if (it->second != 0) {
                is_left_recursion = false; // Non-left recursion, break the loop
                break;
            }
            if (path.empty() || path.back() != it->first->id) {
                path.push_back(it->first->id);
            }
        }

        if (is_left_recursion) {
            path.push_back(current_production->id);
            _state->left_recursion.push_back(InfoWithLoc(
                last_production->id, last_production->lineno(), last_production->colno(),
                current_production->id, current_production->lineno(), current_production->colno(),
                std::make_pair(current_production, path)));
            _state->left_recursion_production_id.insert(current_production->id);
        } else {
            _state->non_left_circular.push_back(InfoWithLoc(
                current_production->id, current_production->lineno(), current_production->colno(),
                last_production->id, last_production->lineno(), last_production->colno(),
                std::make_pair(current_production, last_production)));
        }
        return;
    }

    _state->mark.insert(current);

    /*  LRF
        Push a 0 onto the stack when entering a TermNode
        Term nodes have five possible children: LiteralNode, IdentifierNode, GroupedNode, RepeatedNode, or OptionalNode.
        When encountering LiteralNode, IdentifierNode, or GroupedNode, increment the stack top to keep track of the position.
        For RepeatedNode and OptionalNode, the stack top remains unchanged.
        If we encounter a left recursion, the stack will only contain 0s.
    */
    if (auto c = std::dynamic_pointer_cast<ProductionNode>(current_node)) {
        _state->descent_path.push_back(c);
    } else if (auto c = std::dynamic_pointer_cast<TermNode>(current_node)) {
        _state->descent_path_edge_indices.push_back(std::make_pair(_state->descent_path.back(), 0));
    } else if (auto c = std::dynamic_pointer_cast<OptionalNode>(current_node)) {
    } else if (auto c = std::dynamic_pointer_cast<RepeatedNode>(current_node)) {
    } else if (auto c = std::dynamic_pointer_cast<GroupedNode>(current_node)) {
    }

    /*  FIRST
        Calculate the first set of the production rule. Add the element to map if it's 0th edge.
        The set is unexploaded which contains references to others' FIRST set.
     */
    const auto &current_production = _state->descent_path.back();
    int &descent_index_from_parent = _state->descent_path_edge_indices.back().second;
    if (auto c = std::dynamic_pointer_cast<LiteralNode>(current_node)) {
        if (descent_index_from_parent == 0) {
            // _state->first_set[current_production->id].insert(FirstSetElement(c->value, FirstSetElement::Literal));
        }
    } else if (auto c = std::dynamic_pointer_cast<IdentifierNode>(current_node)) {
        if (descent_index_from_parent == 0) {
            if (_tokens.find(c->value) != _tokens.end()) {
                _state->first_set[current_production->id].insert(FirstSetElement(c->value, FirstSetElement::Token));
            } else {
                // _state->first_set[current_production->id].insert(FirstSetElement(c->value, FirstSetElement::Reference));
            }
        }
    } else if (auto c = std::dynamic_pointer_cast<EpsilonNode>(current_node)) {
        if (descent_index_from_parent == 0) {
            // _state->first_set[current_production->id].insert(FirstSetElement(std::string(), FirstSetElement::Epsilon));
        }
    }

    auto out_edges_pair = boost::out_edges(current, _graph);
    int count = 0;

    // NOTE: it's assumed that BGL guarantee the order of edge traversal - in the order we added them
    for (auto it = out_edges_pair.first; it != out_edges_pair.second; ++it, ++count) {
        Vertex target = boost::target(*it, _graph);

        // Term node is responsible to hold first, second, third... elements in an alternative
        soft_dfs(target, current);
    }

    /*  LRF
        Pop the stack when leaving a TermNode.
        Increment the stack top on exit if the node is a LiteralNode, IdentifierNode, or GroupedNode.
    */
    if (auto c = std::dynamic_pointer_cast<ProductionNode>(current_node)) {
        _state->descent_path.pop_back();
    } else if (auto c = std::dynamic_pointer_cast<TermNode>(current_node)) {
        _state->descent_path_edge_indices.pop_back();
    } else if (auto c = std::dynamic_pointer_cast<OptionalNode>(current_node)) {
    } else if (auto c = std::dynamic_pointer_cast<RepeatedNode>(current_node)) {
    } else if (auto c = std::dynamic_pointer_cast<GroupedNode>(current_node)) {
        // Not using descent_index_from_parent since we don't know if recursion changed the stack or not
        // and the topmost recursion the stack could be empty so we cannot move the declaration out of this if neither
        ++_state->descent_path_edge_indices.back().second;
    } else if (auto c = std::dynamic_pointer_cast<IdentifierNode>(current_node)) {
        ++_state->descent_path_edge_indices.back().second;
    } else if (auto c = std::dynamic_pointer_cast<LiteralNode>(current_node)) {
        ++_state->descent_path_edge_indices.back().second;
    } else if (auto c = std::dynamic_pointer_cast<EpsilonNode>(current_node)) {
    }

    _state->mark.erase(current);
    _state->visited.insert(current);
    _state->reversed_topo.push_back(current);
}

bool DependencyGraphAnalyzer::get_non_left_cicrular_dependency(std::vector<InfoWithLoc<std::pair<ProductionNodePtr, ProductionNodePtr>>> &circular)
{
    if (_state) {
        circular = _state->non_left_circular;
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

bool DependencyGraphAnalyzer::get_left_recursion(std::vector<InfoWithLoc<std::pair<ProductionNodePtr, std::vector<std::string>>>> &left_recursion)
{
    if (_state) {
        left_recursion = decltype(_state->left_recursion)(_state->left_recursion);
        return true;
    } else {
        return false;
    }
}
