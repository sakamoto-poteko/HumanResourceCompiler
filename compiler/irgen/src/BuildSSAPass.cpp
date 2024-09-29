#include <cassert>
#include <map>
#include <string>
#include <tuple>
#include <utility>

#include <boost/algorithm/string.hpp>
#include <boost/graph/dominator_tree.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptors.hpp>
#include <spdlog/spdlog.h>

#include "BuildSSAPass.h"
#include "IRProgramStructure.h"
#include "Operand.h"
#include "ThreeAddressCode.h"
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
                while (dom_tree_map.contains(current) && current != dom_tree_map.at(current)) { // Traverse up the dominator tree
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

std::map<ControlFlowVertex, std::set<ControlFlowVertex>> BuildSSAPass::build_dominance_frontiers(const ControlFlowGraph &cfg, const ControlFlowVertex &start_block)
{
    // Step 1: Compute Dominator Tree

    // map <vert, imm dom of vert>
    std::map<ControlFlowVertex, ControlFlowVertex> immediate_dom_tree_map;
    boost::associative_property_map<std::map<ControlFlowVertex, ControlFlowVertex>> dom_tree_pmap(immediate_dom_tree_map);
    boost::lengauer_tarjan_dominator_tree(cfg, start_block, dom_tree_pmap);

    for (const auto &[vert, idom] : immediate_dom_tree_map) {
        const auto &node = cfg[vert];
        spdlog::debug("[IDOM] '{}': by '{}'", node->get_label(), cfg[idom]->get_label());
    }

    // Step 2: Build Dominator Tree Adjacency List
    // Map each node to its children in the dominator tree.
    // The set doesn't include the vert itself so it's strict dom
    std::map<ControlFlowVertex, std::set<ControlFlowVertex>> strict_dom_tree_children;

    for (const ControlFlowVertex &vertex : boost::make_iterator_range(boost::vertices(cfg))) {
        // Skip the start node which has no immediate dominator
        if (vertex == start_block)
            continue;

        // Find the immediate dominator of v
        auto it = immediate_dom_tree_map.find(vertex);
        if (it != immediate_dom_tree_map.end()) {
            ControlFlowVertex idom = it->second;
            // Avoid self-loop in dominator tree
            if (idom != vertex) {
                strict_dom_tree_children[idom].insert(vertex);
            }
        }
    }

    for (const auto &[vertex, vertex_children] : strict_dom_tree_children) {
        spdlog::debug(
            "[DOM] '{}': {}",
            cfg[vertex]->get_label(),
            boost::join(
                vertex_children | boost::adaptors::transformed([&cfg](const ControlFlowVertex &v) {
                    return "'" + cfg[v]->get_label() + "'";
                }),
                ", "));
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
      - Iterate over its successors. If a successor `s` is not immediately dominated by `b`,
        then `s` is added to the dominator frontier of `b`.
      - Recursively compute the dominator frontiers for its children in the dominator tree.
      - Merge the dominator frontiers of its children into its own,
        adding nodes that are not immediately dominated by `b`.
    */

    // We'll use a depth-first traversal of the dominator tree
    std::function<void(ControlFlowVertex)> compute_df = [&](ControlFlowVertex b) {
        // Step 4a: For each successor s of b
        // keep this way of iteration. it's recursive.
        for (const auto &out_edge : boost::make_iterator_range(boost::out_edges(b, cfg))) {
            ControlFlowVertex s = boost::target(out_edge, cfg);
            // If b does not strictly dominate s (s is b's successor so it's not immediate neither), then s is in DF[b]
            auto idom_it = immediate_dom_tree_map.find(s);
            if (idom_it != immediate_dom_tree_map.end() && idom_it->second != b) {
                dominator_frontiers[b].insert(s);
            }
        }

        // Step 4b: For each child c of b in the dominator tree
        auto children_it = strict_dom_tree_children.find(b);
        if (children_it != strict_dom_tree_children.end()) {
            for (ControlFlowVertex c : children_it->second) {
                compute_df(c); // Recursive call

                // Step 4c: For each w in DF[c]
                for (ControlFlowVertex w : dominator_frontiers[c]) {
                    // Check if b does not strictly dominate w
                    auto idom_w_it = immediate_dom_tree_map.find(w);
                    if (idom_w_it != immediate_dom_tree_map.end() && idom_w_it->second != b) {
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

        spdlog::debug("[DF] '{}': {}", cfg[vert]->get_label(), df_lbls);
    }

    assert(verify_dominance_frontiers(cfg, immediate_dom_tree_map, dominator_frontiers));

    return dominator_frontiers;
}

int BuildSSAPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    const ControlFlowGraph &cfg = *subroutine->get_cfg();
    auto dom_frontiers_vert = build_dominance_frontiers(cfg, subroutine->get_start_block());
    std::map<BasicBlockPtr, std::set<BasicBlockPtr>> dom_frontiers;
    for (const auto &[vert, df_set_vert] : dom_frontiers_vert) {
        assert(!dom_frontiers.contains(cfg[vert]));
        std::set<BasicBlockPtr> &df_set = dom_frontiers[cfg[vert]];
        for (const auto &v : df_set_vert) {
            df_set.insert(cfg[v]);
        }
    }

    // build def-use chain
    std::map<int, std::set<std::tuple<InstructionListIter, BasicBlockPtr>>> def_map;
    std::map<int, std::set<std::tuple<InstructionListIter, BasicBlockPtr>>> use_map;
    const auto &bbs = subroutine->get_basic_blocks();
    for (const auto &bb : bbs) {
        auto &instrs = bb->get_instructions();
        for (auto instr_it = instrs.begin(); instr_it != instrs.end(); ++instr_it) {
            const auto &instr = *instr_it;
            auto def = instr->get_variable_def();
            auto use = instr->get_variable_uses();
            if (def) {
                // static_assert(std::is_same_v<InstructionListIter, decltype(instr_it)>, "not same type");
                def_map[def->get_register_id()].insert({ instr_it, bb });
            }
            for (const auto &operand : use) {
                use_map[operand.get_register_id()].insert({ instr_it, bb });
            }
        }
    }

    // build BBPtr to Vertex map
    std::map<BasicBlockPtr, ControlFlowVertex> bb_vert_map;
    for (ControlFlowVertex vert : boost::make_iterator_range(boost::vertices(cfg))) {
        bb_vert_map[cfg[vert]] = vert;
    }

    insert_phi_functions(subroutine, def_map, dom_frontiers);

    return 0;
}

void BuildSSAPass::insert_phi_functions(
    const SubroutinePtr &subroutine,
    const std::map<int, std::set<std::tuple<InstructionListIter, BasicBlockPtr>>> &def_map,
    const std::map<BasicBlockPtr, std::set<BasicBlockPtr>> &dominance_frontiers_map)
{
    /*
        Cytron et al. algorithm for inserting phi functions:

        1. For each variable `v`:
            - Let `Def(v)` be the set of basic blocks where `v` is defined.
            - Initialize `Work(v)` as a copy of `Def(v)`.
            - Initialize `Phi(v)` as empty.

        2. Iterate until `Work(v)` is empty:
            - Remove a block `X` from `Work(v)`.
            - For each block `Y` in DF(X):
                - If `v` does not already have a phi function in `Y`:
                    - Insert a phi function for `v` in `Y`.
                    - Add `Y` to `Phi(v)`.
                    - If `v` is not already in `Def(v)` for `Y`, add `Y` to `Work(v)`.

        3. Repeat for all variables.
    */
    using VariableDefSet = std::set<std::tuple<InstructionListIter, BasicBlockPtr>>;

    // Repeat for all variables
    for (const auto &[v_id, v_def_raw] : def_map) {
        // v_id < 0 is global. it's not supposed to be defined nor accessed
        assert(v_id >= 0);
        auto v_def_info = v_def_raw | boost::adaptors::transformed([](const std::tuple<InstructionListIter, BasicBlockPtr> &def_info) {
            return std::get<1>(def_info);
        });

        // Let `Def(v)` be the set of basic blocks where `v` is defined.
        std::set<BasicBlockPtr> def(v_def_info.begin(), v_def_info.end());
        // Initialize `Work(v)` as a copy of `Def(v)`.
        std::set<BasicBlockPtr> work(def);
        // Initialize `Phi(v)` as empty.
        // map<bb, phi instr iter>
        std::map<BasicBlockPtr, InstructionListIter> phi;

        // Iterate until `Work(v)` is empty
        while (!work.empty()) {
            BasicBlockPtr x_basic_block = *work.begin();
            // Remove a block `X` from `Work(v)`
            work.erase(work.begin());

            // For each block `Y` in DF(X)
            auto dominance_frontiers = dominance_frontiers_map.find(x_basic_block);
            assert(dominance_frontiers != dominance_frontiers_map.end()); // may not contains. let's assert for now
            for (const BasicBlockPtr &y_basic_block : dominance_frontiers->second) {
                std::list<TACPtr> &instr_list_bb_y = y_basic_block->get_instructions();

                auto phi_instr_it = phi.find(y_basic_block);
                // If `v` does not already have a phi function in `Y`
                if (phi_instr_it == phi.end()) {
                    // Insert a phi function for `v` in `Y`.
                    TACPtr phi_instr = ThreeAddressCode::create_phi(v_id);
                    instr_list_bb_y.push_front(phi_instr);

                    // Add `Y` to `Phi(v)`.
                    auto [inserted_it, _] = phi.insert({ y_basic_block, instr_list_bb_y.begin() });
                    phi_instr_it = inserted_it;

                    // If `v` is not already in `Def(v)` for `Y`, add `Y` to `Work(v)`.
                    if (!def.contains(y_basic_block)) {
                        work.insert(y_basic_block);
                    }
                }

                (*phi_instr_it->second)->set_phi_incoming(x_basic_block, v_id);
            }
        }
    }
}

CLOSE_IRGEN_NAMESPACE

// end
