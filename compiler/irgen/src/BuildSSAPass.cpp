#include <cassert>

#include <boost/algorithm/string.hpp>
#include <boost/graph/dominator_tree.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptors.hpp>
#include <spdlog/spdlog.h>
#include <string>

#include "BuildSSAPass.h"
#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

BuildSSAPass::~BuildSSAPass()
{
}

bool BuildSSAPass::verify_dominance_frontiers(
    const ControlFlowGraph &cfg,
    const std::map<ControlFlowVertex, ControlFlowVertex> &dom_tree_map,
    const std::map<ControlFlowVertex, std::set<ControlFlowVertex>> &dominance_frontiers)
{
    bool valid = true;

    // Iterate over all nodes and their Dominator Frontiers
    for (const auto &[b, df_set] : dominance_frontiers) {
        for (const auto &v : df_set) {
            // 1. Check that b dominates at least one predecessor of v
            bool dominates_predecessor = false;

            for (const auto &in_edge : boost::make_iterator_range(boost::in_edges(v, cfg))) {
                ControlFlowVertex pred = boost::source(in_edge, cfg);
                // Check if b dominates pred
                ControlFlowVertex current = pred;
                while (current != dom_tree_map.at(current)) { // Traverse up the dominator tree
                    if (current == b) {
                        dominates_predecessor = true;
                        break;
                    }
                    current = dom_tree_map.at(current);
                }
                if (current == b) {
                    dominates_predecessor = true;
                    break;
                }
            }

            if (!dominates_predecessor) {
                spdlog::error(
                    "Property Violation: Node {} does not dominate any predecessor of node {} but is in DF({}).",
                    cfg[b]->get_label(),
                    cfg[v]->get_label(),
                    cfg[b]->get_label());
                valid = false;
            }

            // 2. Check that b does not strictly dominate v
            auto it = dom_tree_map.find(v);
            if (it != dom_tree_map.end()) {
                ControlFlowVertex idom = it->second;
                if (idom == b) {
                    spdlog::error(
                        "Property Violation: Node {} strictly dominates node {} but is in DF({}).",
                        cfg[b]->get_label(),
                        cfg[v]->get_label(),
                        cfg[b]->get_label());
                    valid = false;
                }
            }
        }

        // 3. Ensure no node has itself in its DF
        if (dominance_frontiers.at(b).find(b) != dominance_frontiers.at(b).end()) {
            spdlog::error("Property Violation: Node {} has itself in its DF.", cfg[b]->get_label());
            valid = false;
        }
    }

    return valid;
}

std::map<ControlFlowVertex, std::set<ControlFlowVertex>> BuildSSAPass::build_dominance_frontiers(const ControlFlowGraphPtr &graph, const ControlFlowVertex &start_block)
{
    ControlFlowGraph &cfg = *graph;

    // Step 1: Compute Dominator Tree

    // map <vert, imm dom of vert>
    std::map<ControlFlowVertex, ControlFlowVertex> dom_tree_map;
    boost::associative_property_map<std::map<ControlFlowVertex, ControlFlowVertex>> dom_tree_pmap(dom_tree_map);
    boost::lengauer_tarjan_dominator_tree(cfg, start_block, dom_tree_pmap);

    for (const auto &[vert, idom] : dom_tree_map) {
        const auto &node = cfg[vert];
        spdlog::debug("BB '{}' is imm dominated by '{}'", node->get_label(), cfg[idom]->get_label());
    }

    // Step 2: Build Dominator Tree Adjacency List
    // Map each node to its children in the dominator tree
    std::map<ControlFlowVertex, std::vector<ControlFlowVertex>> dom_tree_children;

    // Iterate over all vertices to populate dom_tree_children
    for (const ControlFlowVertex &vertex : boost::make_iterator_range(boost::vertices(cfg))) {
        // Skip the start node which has no immediate dominator
        if (vertex == start_block)
            continue;

        // Find the immediate dominator of v
        auto it = dom_tree_map.find(vertex);
        if (it != dom_tree_map.end()) {
            ControlFlowVertex idom = it->second;
            // Avoid self-loop in dominator tree
            if (idom != vertex) {
                dom_tree_children[idom].push_back(vertex);
            }
        }
    }

    // Step 3: Initialize Dominator Frontiers
    std::map<ControlFlowVertex, std::set<ControlFlowVertex>> dominator_frontiers;
    for (const ControlFlowVertex &vertex : boost::make_iterator_range(boost::vertices(cfg))) {
        dominator_frontiers[vertex] = std::set<ControlFlowVertex>();
    }

    // Step 4: Implement the Dominator Frontier Algorithm

    /*
    DFS dominator tree, starting with entry block.
    For each node `b`:
      - Iterate over its successors. If a successor `s` is not immediately dominated by `b` (map[s] != b),
        then `s` is added to the dominator frontier of `b`.
      - Recursively compute the dominator frontiers for its children in the dominator tree.
      - Merge the dominator frontiers of its children into its own,
        adding nodes that are not immediately dominated by `b`.
    */

    // We'll use a depth-first traversal of the dominator tree
    std::function<void(ControlFlowVertex)> compute_df = [&](ControlFlowVertex b) {
        // Step 4a: For each successor s of b
        // keep this way of iteration. it's recursive.
        const auto [out_begin_it, out_end_it] = boost::out_edges(b, cfg);
        for (auto out_it = out_begin_it; out_it != out_begin_it; ++out_it) {
            ControlFlowVertex s = boost::target(*out_it, cfg);
            // If b does not strictly dominate s, then s is in DF[b]
            auto idom_it = dom_tree_map.find(s);
            if (idom_it != dom_tree_map.end() && idom_it->second != b) {
                dominator_frontiers[b].insert(s);
            }
        }

        // Step 4b: For each child c of b in the dominator tree
        auto children_it = dom_tree_children.find(b);
        if (children_it != dom_tree_children.end()) {
            for (ControlFlowVertex c : children_it->second) {
                compute_df(c); // Recursive call

                // Step 4c: For each w in DF[c]
                for (ControlFlowVertex w : dominator_frontiers[c]) {
                    // Check if b does not strictly dominate w
                    auto idom_w_it = dom_tree_map.find(w);
                    if (idom_w_it != dom_tree_map.end() && idom_w_it->second != b) {
                        dominator_frontiers[b].insert(w);
                    }
                    // Else, do not add to DF[b] (according to the algorithm)
                }
            }
        }
    };

    // Start the computation from the start node
    compute_df(start_block);

    for (const auto &[vert, df] : dominator_frontiers) {
        std::string df_lbls = boost::join(
            df
                | boost::adaptors::transformed([&cfg](const ControlFlowVertex &vert) {
                      return cfg[vert]->get_label();
                  }),
            ", ");

        spdlog::debug("BB '{}' has DF: {}", cfg[vert]->get_label(), df_lbls);
    }

    assert(verify_dominance_frontiers(cfg, dom_tree_map, dominator_frontiers));

    return dominator_frontiers;
}

int BuildSSAPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    auto dom_frontiers = build_dominance_frontiers(subroutine->get_cfg(), subroutine->get_start_block());
    return 0;
}

CLOSE_IRGEN_NAMESPACE

// end
