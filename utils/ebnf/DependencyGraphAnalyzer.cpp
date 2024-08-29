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
        if (!_state->visited.contains(vertex)) {
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
    for (auto &order : _state->reversed_topo) {
        if (ProductionNodePtr production = std::dynamic_pointer_cast<ProductionNode>(_graph[order])) {
            // in reversed topological order

            auto &current_first_set = _state->first_set[production->id];

            // Skip left recursion element
            if (_state->left_recursion_production_id.contains(production->id)) {
                current_first_set.clear();
                continue;
            }

            std::vector<FirstSetElement> expansion_required;
            // get all referenced FIRST in a production
            for (const auto &first : current_first_set) {
                if (first.type == FirstSetElement::Reference) {
                    expansion_required.push_back(first);
                }
            }

            // expand referenced FIRST, and remove the `reference` node
            for (const auto &ref_element : expansion_required) {
                // if any of FIRST is referencing a left recursion, we skip this one
                if (_state->left_recursion_production_id.contains(ref_element.value)) {
                    continue;
                }
                const auto &referenced_first_set = _state->first_set[ref_element.value];
                // for each FIRST in the referenced rule, construct ours, populating the produced_by field
                for (const auto &ref_first : referenced_first_set) {
                    auto our_first = FirstSetElement(ref_first.value, ref_first.type, ref_element.value);
                    current_first_set.insert(our_first);
                    current_first_set.erase(ref_element);
                }
            }
        }
    }
}

void DependencyGraphAnalyzer::analyze_first_first_conflict()
{
    std::map<std::string, std::set<std::string>> visited;

    // look at every production rule's first set
    for (const auto &production_first_set : _state->first_set) {
        // for every element in first set
        for (const auto &first : production_first_set.second) {
            if (first.type == first.Token || first.type == first.Literal) {
                // did we see this FIRST element previouly?
                auto seen = visited.contains(first.value);
                visited[first.value].insert(production_first_set.first);
                // it's FIRST/FIRST conflict if we've seen it
                if (seen) {
                    _state->first_first_conflicts[first.value].insert(production_first_set.first);
                }
            }
        }
    }
}

bool DependencyGraphAnalyzer::analyze()
{
    _state = std::make_unique<VisitState>();

    // Fill the ProductionNode map with rule id
    build_production_rule_map();

    // Perform DFS with:
    //   circular dependency check for all nodes
    //   initial FIRST set construction
    soft_dfs(_state->root, Graph::null_vertex());

    // Expand the FIRST set
    expand_first_set();

    compute_follow_set();

    // Look for FIRST/FIRST conflict
    analyze_first_first_conflict();

    // Are there any nodes untraversed?
    find_unreachable();
    return true;
}

void DependencyGraphAnalyzer::soft_dfs(Vertex current, Vertex parent)
{
    // Comments for Left Recursion Finder starts with *LRF*

    const auto &current_node = _graph[current];

    // Already visited
    if (_state->visited.contains(current)) {
        return;
    }

    // Circular dependency, including left recursion
    if (_state->mark.contains(current)) {
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

            auto info = InfoWithLoc(
                last_production->id, last_production->lineno(), last_production->colno(),
                current_production->id, current_production->lineno(), current_production->colno(),
                std::make_pair(current_production, path));

            if (!_state->left_recursion_dedup.contains(info)) {
                _state->left_recursion_dedup.insert(info);
                _state->left_recursion.push_back(info);
                _state->left_recursion_production_id.insert(current_production->id);
            }
        } else {
            auto info = InfoWithLoc(
                current_production->id, current_production->lineno(), current_production->colno(),
                last_production->id, last_production->lineno(), last_production->colno(),
                std::make_pair(current_production, last_production));

            if (!_state->non_left_circular_dedup.contains(info)) {
                _state->non_left_circular_dedup.insert(info);
                _state->non_left_circular.push_back(info);
            }
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
    if (auto production = std::dynamic_pointer_cast<ProductionNode>(current_node)) {
        _state->descent_path.push_back(production);
    } else if (auto term = std::dynamic_pointer_cast<TermNode>(current_node)) {
        _state->descent_path_edge_indices.push_back(std::make_pair(_state->descent_path.back(), 0));
    } else if (auto optional = std::dynamic_pointer_cast<OptionalNode>(current_node)) {
    } else if (auto repeated = std::dynamic_pointer_cast<RepeatedNode>(current_node)) {
    } else if (auto grouped = std::dynamic_pointer_cast<GroupedNode>(current_node)) {
    }

    /*  FIRST
        Calculate the first set of the production rule. Add the element to map if it's 0th edge for all Terms until the production.
        The set is unexploaded which contains references to others' FIRST set.
     */
    // WARNING: the use of current_production should not leave these ifs. Pushing elements into vector may result in the reallocation of vector
    // hence invalidation of the reference
    const auto &current_production = _state->descent_path.back();

    auto determine_is_first_set_element = [&]() {
        // Must be searched bottom-to-top. There might me multiple elements with same production id when ecounter Repeated/Grouped/Optional.
        auto first_prod_occurence = std::find_if(
            _state->descent_path_edge_indices.cbegin(),
            _state->descent_path_edge_indices.cend(),
            [&current_production](auto &rule_idx_pair) {
                return rule_idx_pair.first->id == current_production->id;
            });
        bool all_have_index_of_0 = std::all_of(
            first_prod_occurence,
            _state->descent_path_edge_indices.cend(),
            [](auto &rule_idx_pair) {
                return rule_idx_pair.second == 0;
            });

        return all_have_index_of_0;
    };

    if (auto c = std::dynamic_pointer_cast<LiteralNode>(current_node)) {
        if (determine_is_first_set_element()) {
            _state->first_set[current_production->id].insert(FirstSetElement(c->value, FirstSetElement::Literal));
        }
    } else if (auto c = std::dynamic_pointer_cast<IdentifierNode>(current_node)) {
        if (determine_is_first_set_element()) {
            if (_tokens.contains(c->value)) {
                _state->first_set[current_production->id].insert(FirstSetElement(c->value, FirstSetElement::Token));
            } else {
                _state->first_set[current_production->id].insert(FirstSetElement(c->value, FirstSetElement::Reference));
            }
        }
    } else if (auto c = std::dynamic_pointer_cast<EpsilonNode>(current_node)) {
        if (determine_is_first_set_element()) {
            _state->first_set[current_production->id].insert(FirstSetElement(std::string(), FirstSetElement::Epsilon));
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
    // WARNING: NO MORE USE of current_production after this line. Same reason as above.
    if (auto c = std::dynamic_pointer_cast<ProductionNode>(current_node)) {
        _state->descent_path.pop_back();
    } else if (auto c = std::dynamic_pointer_cast<TermNode>(current_node)) {
        _state->descent_path_edge_indices.pop_back();
    } else if (auto c = std::dynamic_pointer_cast<OptionalNode>(current_node)) {
    } else if (auto c = std::dynamic_pointer_cast<RepeatedNode>(current_node)) {
    } else if (auto c = std::dynamic_pointer_cast<GroupedNode>(current_node)) {
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
        left_recursion = _state->left_recursion;
        return true;
    } else {
        return false;
    }
}

bool DependencyGraphAnalyzer::get_first_set(std::map<std::string, std::set<FirstSetElement>> &firsts)
{
    if (_state) {
        firsts = _state->first_set;
        return true;
    } else {
        return false;
    }
}

bool DependencyGraphAnalyzer::get_first_first_conflicts(std::map<std::string, std::set<std::string>> &conflicts)
{
    if (_state) {
        conflicts = _state->first_first_conflicts;
        return true;
    } else {
        return false;
    }
}

void DependencyGraphAnalyzer::compute_follow_set()
{
    // FIXME: Impl
    /*
    Algorithm:
    
    1. Initialize the FOLLOW set for each non-terminal as an empty set.
    2. Add the end-of-input marker ('$') to the FOLLOW set of the start symbol.
    3. Apply the following rules iteratively until no changes occur:
        - Rule 1: For a production A → αBβ, add FIRST(β) - {ε} to FOLLOW(B).
        - Rule 2: For a production A → αB or if β can derive ε, add FOLLOW(A) to FOLLOW(B).
    4. Return the FOLLOW sets after convergence.

    Ref: Dragon book Section 4.4.2
    */

    // iterate until no changes occur
    bool expanded = false;
    while (expanded) {
        for (const auto &production : _state->productions) {

        }
    }
}
