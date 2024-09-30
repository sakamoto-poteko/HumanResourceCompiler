#include <cassert>
#include <list>
#include <map>
#include <stack>
#include <string>
#include <tuple>
#include <utility>

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
        if (dominance_frontiers.at(b).find(b) != dominance_frontiers.at(b).end()) {
            spdlog::error("Property Violation: Node {} has itself in its DF.", cfg[b]->get_label());
            valid = false;
        }
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
            "[DOM] '{}': {}",
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
                    if (idom_w_it != immediate_dom_by_tree_map.end() && idom_w_it->second != b) {
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
    remove_single_branch_phi(subroutine->get_basic_blocks());
    rename_registers(subroutine, immediate_dom_tree_map);
    renumber_registers(subroutine);
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

void BuildSSAPass::remove_single_branch_phi(const std::list<BasicBlockPtr> &basic_blocks)
{
    for (const BasicBlockPtr &basic_block : basic_blocks) {
        std::list<TACPtr> &instructions = basic_block->get_instructions();
        instructions.remove_if([](const TACPtr &instr) {
            if (instr->get_op() == IROperation::PHI) {
                std::size_t incoming_count = instr->get_phi_incomings().size();
                assert(incoming_count != 0); // report zero branch
                return incoming_count == 1; // remove single branch
            }
            return false;
        });
    }
}

void BuildSSAPass::rename_registers(const SubroutinePtr &subroutine, const std::map<ControlFlowVertex, ControlFlowVertex> &imm_dom_by_tree_map)
{
    const ControlFlowGraph &cfg = *subroutine->get_cfg();
    const ControlFlowVertex &start_block = subroutine->get_start_block();

    const unsigned int max_used = subroutine->get_max_reg_id();
    unsigned int current_number = max_used + 1;

    std::map<unsigned int, std::stack<unsigned int>> name_stacks;
    // map<new id, old id>
    std::map<unsigned int, unsigned int> original_id_map;
    for (unsigned i = 0; i <= max_used; ++i) {
        name_stacks[i].push(i);
        original_id_map[i] = i;
    }

    // map<old id, map<bb, new id in bb>
    std::map<unsigned int, std::map<BasicBlockPtr, unsigned int>> basic_block_renamed_id;

    // map<node, node imm doms>
    std::map<ControlFlowVertex, std::set<ControlFlowVertex>> imm_doms;
    for (const auto &[vert, dom_by_vert] : imm_dom_by_tree_map) {
        imm_doms[dom_by_vert].insert(vert);
    }

    std::function<void(ControlFlowVertex)> rename_block = [&](const ControlFlowVertex &block_vertex) {
        // map<reg id, push count>
        std::map<unsigned int, unsigned int> push_count;
        const BasicBlockPtr current_basic_block = cfg[block_vertex];
        std::list<TACPtr> &instructions = current_basic_block->get_instructions();

        for (TACPtr &instr : instructions) {
            Operand tgt = instr->get_tgt();
            Operand src1 = instr->get_src1();
            Operand src2 = instr->get_src2();
            bool mutated = false;

            // if assert fails, consider skip phi nodes here
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
                if (instr->get_op() == IROperation::PHI) {
                    new_instr->set_phi_incomings(instr->get_phi_incomings());
                }

                instr = new_instr;
            }
        }

        for (auto out_edge : boost::make_iterator_range(boost::out_edges(block_vertex, cfg))) {
            ControlFlowVertex successor_vertex = boost::target(out_edge, cfg);
            const BasicBlockPtr &successor_block = cfg[successor_vertex];

            for (TACPtr &instruction : successor_block->get_instructions()) {
                if (instruction->get_op() == IROperation::PHI) {
                    Operand tgt = instruction->get_tgt();
                    assert(tgt.get_type() == Operand::OperandType::VariableId);
                    assert(original_id_map.contains(tgt.get_register_id()));
                    unsigned original_id = original_id_map[tgt.get_register_id()];
                    assert(name_stacks.contains(original_id));

                    TACPtr new_phi = ThreeAddressCode::create_phi(name_stacks[original_id].top(), instruction->get_ast_node());
                    for (const auto &[incoming_bb, incoming_var_id] : instruction->get_phi_incomings()) {
                        assert(original_id_map.contains(incoming_var_id));
                        unsigned original_var_id = original_id_map[incoming_var_id];
                        assert(basic_block_renamed_id.contains(original_var_id));
                        if (basic_block_renamed_id[original_var_id].contains(incoming_bb)) {
                            unsigned renamed_var_id_in_bb = basic_block_renamed_id[original_var_id][incoming_bb];
                            new_phi->set_phi_incoming(incoming_bb, renamed_var_id_in_bb);
                        } else {
                            new_phi->set_phi_incoming(incoming_bb, incoming_var_id);
                        }
                    }
                    instruction = new_phi;
                }
            }
        }

        for (const ControlFlowVertex &imm_dominated : imm_doms[block_vertex]) {
            rename_block(imm_dominated);
        }

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
}

void BuildSSAPass::renumber_registers(const SubroutinePtr &subroutine)
{
    const std::list<BasicBlockPtr> &basic_blocks = subroutine->get_basic_blocks();
    unsigned int current_number = 0;
    std::map<unsigned int, unsigned int> old_to_new_id_map;

    auto get_new_reg_id = [&current_number, &old_to_new_id_map](unsigned int old_id) {
        auto it = old_to_new_id_map.find(old_id);
        if (it == old_to_new_id_map.end()) {
            unsigned int new_id = current_number++;
            old_to_new_id_map[old_id] = new_id;
            return new_id;
        } else {
            return it->second;
        }
    };

    for (const BasicBlockPtr &basic_block : basic_blocks) {
        for (TACPtr &instr : basic_block->get_instructions()) {
            Operand tgt = instr->get_tgt();
            Operand src1 = instr->get_src1();
            Operand src2 = instr->get_src2();
            bool mutated = false;

            // if assert fails, consider skip phi nodes here
            if (src1.get_type() == Operand::OperandType::VariableId && src1.get_register_id() >= 0) {
                src1 = Operand(get_new_reg_id(src1.get_register_id()));
                mutated = true;
            }

            if (src2.get_type() == Operand::OperandType::VariableId && src2.get_register_id() >= 0) {
                src2 = Operand(get_new_reg_id(src2.get_register_id()));
                mutated = true;
            }

            if (tgt.get_type() == Operand::OperandType::VariableId && tgt.get_register_id() >= 0) {
                tgt = Operand(get_new_reg_id(tgt.get_register_id()));
                mutated = true;
            }

            if (mutated) {
                TACPtr new_instr = ThreeAddressCode::create(instr->get_op(), tgt, src1, src2, instr->get_ast_node());
                if (instr->get_op() == IROperation::PHI) {
                    for (const auto &[income_basic_block, var_id] : instr->get_phi_incomings()) {
                        new_instr->set_phi_incoming(income_basic_block, get_new_reg_id(var_id));
                    }
                }

                instr = new_instr;
            }
        }
    }
}

CLOSE_IRGEN_NAMESPACE
// end
