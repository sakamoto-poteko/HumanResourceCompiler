#include <cassert>
#include <functional>
#include <list>
#include <map>
#include <ranges>
#include <stack>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/graph/dominator_tree.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptors.hpp>
#include <spdlog/spdlog.h>

#include "BuildSSAPass.h"
#include "IROps.h"
#include "IRProgramStructure.h"
#include "Operand.h"
#include "ThreeAddressCode.h"
#include "hrl_global.h"
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
        // GPT made a mistake here. DF(x) can be x.
        // if (dominance_frontiers.at(b).find(b) != dominance_frontiers.at(b).end()) {
        //     spdlog::error("Property Violation: Node {} has itself in its DF.", cfg[b]->get_label());
        //     valid = false;
        // }
    }

    return valid;
}

std::pair<std::map<ControlFlowVertex, ControlFlowVertex>, std::map<ControlFlowVertex, std::set<ControlFlowVertex>>> BuildSSAPass::build_dominance_tree(const ControlFlowGraph &cfg, const ControlFlowVertex &start_block)
{
    // Step 1: Compute Dominator Tree

    // map <vert, imm dom of vert>
    std::map<ControlFlowVertex, ControlFlowVertex> immediate_dom_by_tree_map;
    boost::associative_property_map<std::map<ControlFlowVertex, ControlFlowVertex>> dom_tree_pmap(immediate_dom_by_tree_map);
    boost::lengauer_tarjan_dominator_tree(cfg, start_block, dom_tree_pmap);

    for (const auto &[vert, idom] : immediate_dom_by_tree_map) {
        const auto &node = cfg[vert];
        spdlog::debug("[SSA IDOM] '{}': by '{}'", node->get_label(), cfg[idom]->get_label());
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
        auto it = immediate_dom_by_tree_map.find(vertex);
        if (it != immediate_dom_by_tree_map.end()) {
            ControlFlowVertex idom = it->second;
            // Avoid self-loop in dominator tree
            if (idom != vertex) {
                strict_dom_tree_children[idom].insert(vertex);
            }
        }
    }

    for (const auto &[vertex, vertex_children] : strict_dom_tree_children) {
        spdlog::debug(
            "[SSA DOM] '{}': {{{}}}",
            cfg[vertex]->get_label(),
            boost::join(
                vertex_children | boost::adaptors::transformed([&cfg](const ControlFlowVertex &v) {
                    return "'" + cfg[v]->get_label() + "'";
                }),
                ", "));
    }

    return std::make_pair(immediate_dom_by_tree_map, strict_dom_tree_children);
}

std::map<ControlFlowVertex, std::set<ControlFlowVertex>> BuildSSAPass::build_dominance_frontiers(
    const ControlFlowGraph &cfg,
    const ControlFlowVertex &start_block,
    std::map<ControlFlowVertex, ControlFlowVertex> immediate_dom_by_tree_map,
    std::map<ControlFlowVertex, std::set<ControlFlowVertex>> strict_dom_tree_children)
{
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
            auto idom_it = immediate_dom_by_tree_map.find(s);
            if (idom_it != immediate_dom_by_tree_map.end() && idom_it->second != b) {
                spdlog::trace("[SSA ComputeDF1] Adding '{}' to '{}'", cfg[s]->get_label(), cfg[b]->get_label());
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
                    auto idom_w_it = immediate_dom_by_tree_map.find(w);
                    if (idom_w_it == immediate_dom_by_tree_map.end()) {
                        spdlog::trace("[SSA ComputeDF2] Immediate dominator for node '{}' was not found", cfg[w]->get_label());
                        continue;
                    }

                    if (idom_w_it->second != b) {
                        spdlog::trace("[SSA ComputeDF2] Adding '{}' to '{}'", cfg[w]->get_label(), cfg[b]->get_label());
                        dominator_frontiers[b].insert(w);
                    } else {
                        spdlog::trace("[SSA ComputeDF2] Skipping inserting '{}' to '{}' because idom(w) == b", cfg[w]->get_label(), cfg[b]->get_label());
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

        spdlog::debug("[SSA DF] '{}': {}", cfg[vert]->get_label(), df_lbls);
    }

    assert(verify_dominance_frontiers(cfg, immediate_dom_by_tree_map, dominator_frontiers));

    return dominator_frontiers;
}

int BuildSSAPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    UNUSED(metadata);
    UNUSED(program);

    const ControlFlowGraph &cfg = *subroutine->get_cfg();

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
                def_map[def->get_register_id()].insert({ instr_it, bb });
            }
            for (const auto &operand : use) {
                use_map[operand.get_register_id()].insert({ instr_it, bb });
            }
        }
    }

    auto [immediate_dom_tree_map, strict_dom_tree_children] = build_dominance_tree(cfg, subroutine->get_start_block());
    auto dom_frontiers_vert = build_dominance_frontiers(cfg, subroutine->get_start_block(), immediate_dom_tree_map, strict_dom_tree_children);
    // convert Vertex to BBPtr
    std::map<BasicBlockPtr, std::set<BasicBlockPtr>> dom_frontiers;
    for (const auto &[vert, df_set_vert] : dom_frontiers_vert) {
        assert(!dom_frontiers.contains(cfg[vert]));
        std::set<BasicBlockPtr> &df_set = dom_frontiers[cfg[vert]];
        for (const auto &v : df_set_vert) {
            df_set.insert(cfg[v]);
        }
    }

    // build BBPtr to Vertex map
    std::map<BasicBlockPtr, ControlFlowVertex> bb_vert_map;
    for (ControlFlowVertex vert : boost::make_iterator_range(boost::vertices(cfg))) {
        bb_vert_map[cfg[vert]] = vert;
    }

    insert_phi_functions(def_map, dom_frontiers);
    populate_phi_function(def_map, cfg);
    remove_redundant_phi(subroutine->get_basic_blocks());
    rename_registers(subroutine, immediate_dom_tree_map);
    // rename registers may clean up some phi incoming
    remove_redundant_phi(subroutine->get_basic_blocks());
    return 0;
}

void BuildSSAPass::insert_phi_functions(
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

                // If `v` does not already have a phi function in `Y`
                if (!phi.contains(y_basic_block)) {
                    // Insert a phi function for `v` in `Y`.
                    TACPtr phi_instr = ThreeAddressCode::create_phi(v_id);
                    instr_list_bb_y.push_front(phi_instr);

                    // Add `Y` to `Phi(v)`.
                    phi[y_basic_block] = instr_list_bb_y.begin();

                    // If `v` is not already in `Def(v)` for `Y`, add `Y` to `Work(v)`.
                    if (!def.contains(y_basic_block)) {
                        work.insert(y_basic_block);
                    }
                }

                // We're not populating the phi branch here. Will do that in a separate method
            }
        }
    }
}

void BuildSSAPass::remove_redundant_phi(const std::list<BasicBlockPtr> &basic_blocks)
{
    for (const BasicBlockPtr &basic_block : basic_blocks) {
        std::list<TACPtr> &instructions = basic_block->get_instructions();
        instructions.remove_if([](const TACPtr &instr) {
            if (instr->get_op() == IROperation::PHI) {
                return instr->get_phi_incomings().size() <= 1;
            }
            return false;
        });
    }
}

void BuildSSAPass::populate_phi_function(
    const std::map<int, std::set<std::tuple<InstructionListIter, BasicBlockPtr>>> &def_map,
    const ControlFlowGraph &cfg)
{
    std::map<BasicBlockPtr, ControlFlowVertex> basic_block_to_vertex;
    for (const auto &vert : boost::make_iterator_range(boost::vertices(cfg))) {
        basic_block_to_vertex[cfg[vert]] = vert;
    }

    std::map<int, std::set<BasicBlockPtr>> def_map_without_phi;
    std::map<int, std::set<BasicBlockPtr>> def_map_phi;

    for (const auto &[v_id, v_def_raw] : def_map) {
        // Ensure v_id is non-negative
        assert(v_id >= 0);

        // Make a map of <var id, var def blocks> of non phi nodes
        // FIXME: exclude phi?
        auto v_def_info = v_def_raw
            | std::views::transform([](const std::tuple<InstructionListIter, BasicBlockPtr> &def_info) {
                  return std::get<1>(def_info);
              });
        def_map_without_phi.emplace(v_id, std::set<BasicBlockPtr>(v_def_info.begin(), v_def_info.end()));

        // Make a map of <var id, var def blocks> of phi nodes
        std::set<BasicBlockPtr> phi_bb_set;
        // for every BB, find if this node has phi defined in it.
        for (ControlFlowVertex vert : boost::make_iterator_range(boost::vertices(cfg))) {
            const BasicBlockPtr &basic_block_to_check = cfg[vert];
            for (const TACPtr &instruction : basic_block_to_check->get_instructions()) {
                if (instruction->get_op() == IROperation::PHI) {
                    const Operand &tgt = instruction->get_tgt();
                    assert(tgt.get_type() == Operand::OperandType::VariableId);
                    // this variable is redefined in phi. add it to variable definition
                    if (tgt.get_register_id() == v_id) {
                        phi_bb_set.insert(basic_block_to_check);
                    }
                }
            }
        }

        // Construct a set from the transformed range and assign it to the map
        def_map_phi.emplace(v_id, std::move(phi_bb_set));
    }

    for (const auto &[v_id, _] : def_map) {
        // v_id < 0 is global. it's not supposed to be defined nor accessed
        assert(v_id >= 0);
        const std::set<BasicBlockPtr> &v_def_basic_blocks_without_phi = def_map_without_phi.at(v_id);
        const std::set<BasicBlockPtr> &v_def_basic_blocks_phi_only = def_map_phi.at(v_id);

        std::function<void(const BasicBlockPtr &, const BasicBlockPtr &, bool, std::set<ControlFlowVertex> &, std::vector<ControlFlowVertex> &)>
            populate_phi_in_block = [&](const BasicBlockPtr &visit_block,
                                        const BasicBlockPtr &v_defined_basic_block,
                                        bool is_phi_map,
                                        std::set<ControlFlowVertex> &block_visited,
                                        std::vector<ControlFlowVertex> &visit_history) {
                ControlFlowVertex visit_vertex = basic_block_to_vertex.at(visit_block);
                ControlFlowVertex v_defined_vertex = basic_block_to_vertex.at(v_defined_basic_block);

                if (block_visited.contains(visit_vertex)) {
                    return;
                }
                block_visited.insert(visit_vertex);

                // if there is another def of v_id on the way, return.
                // phi branch should be added by that def's traversal
                if (visit_vertex != v_defined_vertex && v_def_basic_blocks_without_phi.contains(cfg[visit_vertex])) {
                    return;
                }
                const BasicBlockPtr basic_block = cfg[visit_vertex];

                for (TACPtr &instr : basic_block->get_instructions()) {
                    if (instr->get_op() == IROperation::PHI) {
                        // The def map include phi may include this basic block, since this block defines a phi.
                        // That's our def map is constructed after the phi node insertion.
                        // This will pull all newly insered phi nodes too.
                        // We should skip it if def_block is current block, and the defined var is this var
                        if (v_defined_basic_block == basic_block && instr->get_tgt().get_register_id() == v_id) {
                            spdlog::trace(
                                "[SSA Phi Population] Skipped phi '{}' in '{}', def blk '{}'",
                                std::string(instr->get_tgt()),
                                basic_block->get_label(),
                                v_defined_basic_block->get_label());
                        } else {
                            const Operand &tgt = instr->get_tgt();
                            assert(tgt.get_type() == Operand::OperandType::VariableId);
                            assert(tgt.get_register_id() >= 0);

                            // check if the target is the var id we're working on
                            // if yes, we've met a phi where def block's defined var is used
                            if (tgt.get_register_id() == v_id) {
                                const BasicBlockPtr &predecessor = cfg[visit_history.back()];
                                instr->set_phi_incoming(predecessor, v_id, v_defined_basic_block);
                                // we already found the phi for this def. now exit
                                // there can only be one phi for one def
                                // we haven't renamed yet. the incoming var id is all the same
                                spdlog::trace(
                                    "[SSA Phi Population] Add phi '{}' in '{}' with pred blk '{}', def blk '{}'",
                                    std::string(instr->get_tgt()),
                                    basic_block->get_label(),
                                    predecessor->get_label(),
                                    v_defined_basic_block->get_label());
                                return;
                            }
                        }
                    }
                }

                for (ControlFlowEdge edge : boost::make_iterator_range(boost::out_edges(visit_vertex, cfg))) {
                    ControlFlowVertex child = boost::target(edge, cfg);
                    visit_history.push_back(visit_vertex);
                    populate_phi_in_block(cfg[child], v_defined_basic_block, is_phi_map, block_visited, visit_history);
                    visit_history.pop_back();
                }
            };

        // For all ordinary defs
        for (const BasicBlockPtr &def_block : v_def_basic_blocks_without_phi) {
            std::set<ControlFlowVertex> block_visited;
            std::vector<ControlFlowVertex> visit_history;
            visit_history.push_back(basic_block_to_vertex.at(def_block));
            // start traverse from def_block
            populate_phi_in_block(def_block, def_block, false, block_visited, visit_history);
            visit_history.pop_back();
            assert(visit_history.empty());
        }

        // For all phi defs
        for (const BasicBlockPtr &def_block : v_def_basic_blocks_phi_only) {
            std::set<ControlFlowVertex> block_visited;
            std::vector<ControlFlowVertex> visit_history;
            visit_history.push_back(basic_block_to_vertex.at(def_block));
            // start traverse from def_block
            populate_phi_in_block(def_block, def_block, true, block_visited, visit_history);
            visit_history.pop_back();
            assert(visit_history.empty());
        }
    }
}

void BuildSSAPass::rename_registers(const SubroutinePtr &subroutine, const std::map<ControlFlowVertex, ControlFlowVertex> &imm_dom_by_tree_map)
{
    const ControlFlowGraph &cfg = *subroutine->get_cfg();
    const ControlFlowVertex &start_block = subroutine->get_start_block();

    const unsigned int max_used = subroutine->get_max_reg_id();
    unsigned int current_number = max_used + 1;

    // The key is the original variable id. stack is the renamed id
    std::map<unsigned int, std::stack<unsigned int>> name_stacks;
    // map<new id, old id>. This maps the new id to original id, so it can be used to lookup name_stacks, basic_block_renamed_id, etc.
    std::map<unsigned int, unsigned int> original_id_map;
    for (unsigned i = 0; i <= max_used; ++i) {
        name_stacks[i].push(i);
        original_id_map[i] = i;
    }

    // map<old id, map<bb, new id in bb>. This maps the original variable id to the Basic Block <=> New Variable Id pairs
    std::map<unsigned int, std::map<BasicBlockPtr, unsigned int>> basic_block_renamed_id;

    // map<node, node imm doms>. Node <=> Immediate Dominanated Nodes
    std::map<ControlFlowVertex, std::set<ControlFlowVertex>> imm_doms;
    for (const auto &[vert, dom_by_vert] : imm_dom_by_tree_map) {
        imm_doms[dom_by_vert].insert(vert);
    }

    std::function<void(ControlFlowVertex)> rename_block = [&](const ControlFlowVertex &block_vertex) {
        // map<reg id, push count>
        std::map<unsigned int, unsigned int> push_count;
        const BasicBlockPtr current_basic_block = cfg[block_vertex];
        std::list<TACPtr> &instructions = current_basic_block->get_instructions();
        spdlog::trace("[SSA Rename] Traversing BB '{}'", current_basic_block->get_label());

        // For each instruction, rename the phi node's target first
        for (TACPtr &instruction : instructions) {
            if (instruction->get_op() != IROperation::PHI) {
                continue;
            }

            Operand tgt = instruction->get_tgt();
            assert(tgt.get_type() == Operand::OperandType::VariableId);
            assert(original_id_map.contains(tgt.get_register_id()));
            unsigned original_id = original_id_map[tgt.get_register_id()];
            assert(name_stacks.contains(original_id));

            unsigned int new_phi_id = current_number++;
            name_stacks[original_id].push(new_phi_id);
            push_count[original_id]++;
            original_id_map[new_phi_id] = original_id;
            basic_block_renamed_id[original_id][current_basic_block] = new_phi_id;

            TACPtr new_phi = ThreeAddressCode::create_phi(new_phi_id, instruction->get_ast_node());
            new_phi->set_phi_incomings(instruction->get_phi_incomings());
            instruction = new_phi;
            spdlog::trace("[SSA Rename] Renaming Phi node target '%{}' to '%{}', in BB '{}'", tgt.get_register_id(), new_phi_id, current_basic_block->get_label());
        }

        // Then rename other instructions
        for (TACPtr &instr : instructions) {
            if (instr->get_op() == IROperation::PHI) {
                continue;
            }

            Operand tgt = instr->get_tgt();
            Operand src1 = instr->get_src1();
            Operand src2 = instr->get_src2();
            bool mutated = false;

            // For source operands, we simply map the id to original one,
            // then we look up the stack's top to find it's current id
            if (src1.get_type() == Operand::OperandType::VariableId && src1.get_register_id() >= 0) {
                assert(original_id_map.contains(src1.get_register_id()));
                unsigned original_id = original_id_map[src1.get_register_id()];
                assert(name_stacks.contains(original_id));
                src1 = Operand(name_stacks[original_id].top());
                mutated = true;
            }

            if (src2.get_type() == Operand::OperandType::VariableId && src2.get_register_id() >= 0) {
                assert(original_id_map.contains(src2.get_register_id()));
                unsigned original_id = original_id_map[src2.get_register_id()];
                assert(name_stacks.contains(original_id));
                src2 = Operand(name_stacks[original_id].top());
                mutated = true;
            }

            // For target operand, we map the id to original one, then assign it a new id.
            if (tgt.get_type() == Operand::OperandType::VariableId && tgt.get_register_id() >= 0) {
                unsigned cur_tgt_id = tgt.get_register_id();
                assert(original_id_map.contains(cur_tgt_id));
                unsigned original_tgt_id = original_id_map[cur_tgt_id]; // get the old name

                unsigned int new_tgt_id = current_number++;
                tgt = Operand(new_tgt_id);
                assert(name_stacks.contains(original_tgt_id));
                name_stacks[original_tgt_id].push(new_tgt_id);
                push_count[original_tgt_id]++;
                original_id_map[new_tgt_id] = original_tgt_id;
                // the map always has the latest definition of var
                basic_block_renamed_id[original_tgt_id][current_basic_block] = new_tgt_id;

                mutated = true;
            }

            if (mutated) {
                TACPtr new_instr = ThreeAddressCode::create(instr->get_op(), tgt, src1, src2, instr->get_ast_node());
                spdlog::trace("[SSA Rename] Renaming ordinary node '{}' to '{}'", instr->to_string(true), new_instr->to_string(true));
                instr = new_instr;
            }
        }

        // Traverse the dominator tree
        for (const ControlFlowVertex &imm_dominated : imm_doms[block_vertex]) {
            rename_block(imm_dominated);
        }

        // Now the dominated nodes have been renamed.
        // Clean up the stack
        for (auto &[var_id, count] : push_count) {
            assert(name_stacks.contains(var_id));
            for (unsigned int i = 0; i < count; ++i) {
                name_stacks[var_id].pop();
            }
            count = 0;
            assert(!name_stacks[var_id].empty());
        }
    };

    rename_block(start_block);

    // Now all nodes' operands have been renamed. We now rename phi's incoming branch
    for (const BasicBlockPtr &current_basic_block : subroutine->get_basic_blocks()) {
        for (TACPtr &instruction : current_basic_block->get_instructions()) {
            if (instruction->get_op() != IROperation::PHI) {
                continue;
            }
            // phi is already renamed here. let's take care of incoming branches
            Operand tgt = instruction->get_tgt();
            assert(tgt.get_type() == Operand::OperandType::VariableId);

            // For each incoming branch, we look up the original id of the branch's incoming var
            // Then we look at the map to find it's renamed id in the block which is defined
            auto &phi_incomings = instruction->get_phi_incomings();
            for (auto incoming_it = phi_incomings.begin(); incoming_it != phi_incomings.end();) {
                const auto &[incoming_predecessor_block, phi_branch_meta] = *incoming_it;
                auto &[incoming_var_id, incoming_def_block] = phi_branch_meta;
                assert(original_id_map.contains(incoming_var_id));
                unsigned original_var_id = original_id_map[incoming_var_id];
                assert(basic_block_renamed_id.contains(original_var_id));

                // look up the incoming def block against original variable id, so we can get the renamed variable id in that def block
                if (basic_block_renamed_id[original_var_id].contains(incoming_def_block)) {
                    unsigned renamed_var_id_in_bb = basic_block_renamed_id[original_var_id][incoming_def_block];
                    instruction->set_phi_incoming(incoming_predecessor_block, renamed_var_id_in_bb, incoming_def_block);
                    ++incoming_it;
                } else {
                    // We should be able to find the renamed var id according to original var id and def block
                    // When performing a rename against target (both phi and ordinary), we've already recorded that
                    // What happened here?
                    // The phi branch is redundant because all non-redundant incomings are already renamed.
                    // If the map doesn't have it, it's probably rougue phi introduced in that phi def extraction.
                    spdlog::error(
                        "[SSA Rename] Renamed id for '%{}' (original) was not found. Traversing BB '{}', incoming def block '{}'. This is likely a bug, consider report it.\n"
                        "New idea: probably a redundant phi which is already cleaned up. Trying to remove this branch...",
                        original_var_id, current_basic_block->get_label(), incoming_def_block->get_label());
                    incoming_it = phi_incomings.erase(incoming_it);
                    // throw;
                    // FIXME:
                }
            }
        }
    }
}

CLOSE_IRGEN_NAMESPACE
// end
