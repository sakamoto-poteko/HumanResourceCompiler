#include <cassert>
#include <functional>
#include <list>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/graph/dominator_tree.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptors.hpp>
#include <spdlog/spdlog.h>

#include "BuildSSAPass.h"
#include "GraphvizGenerator.h"
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
        auto _ = strict_dom_tree_children[vertex]; // make sure every vertex is in the map

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
    const ControlFlowVertex entry_vertex = subroutine->get_start_block();

    // build the map where variables are defined in basic blocks
    std::map<unsigned int, std::set<BasicBlockPtr>> def_map = subroutine->get_def_variables();

    auto [immediate_dom_tree_map, strict_dom_tree_children] = build_dominance_tree(cfg, subroutine->get_start_block());
    auto dom_frontiers_vert = build_dominance_frontiers(cfg, entry_vertex, immediate_dom_tree_map, strict_dom_tree_children);
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
    rename_and_populate_phi(def_map, strict_dom_tree_children, cfg, entry_vertex);

    // write dom tree
    ControlFlowGraph dom_tree_bgl;
    std::map<ControlFlowVertex, ControlFlowVertex> cfg_vertex_to_dom_tree_bgl;
    for (const auto &[node, children] : strict_dom_tree_children) {
        ControlFlowVertex vert = dom_tree_bgl.add_vertex(cfg[node]);
        cfg_vertex_to_dom_tree_bgl[node] = vert;
    }

    for (const auto &[node, children] : strict_dom_tree_children) {
        ControlFlowVertex vert = cfg_vertex_to_dom_tree_bgl.at(node);
        for (ControlFlowVertex child : children) {
            dom_tree_bgl.add_edge(vert, cfg_vertex_to_dom_tree_bgl[child]);
        }
    }

    _dominance_trees.insert_or_assign(subroutine, std::move(dom_tree_bgl));
    return 0;
}

void BuildSSAPass::insert_phi_functions(
    const std::map<unsigned int, std::set<BasicBlockPtr>> &def_map,
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
                - If `v` does not already have a phi function in `Y`, and `v` is live in `Y`:
                    - Insert a phi function for `v` in `Y`.
                    - Add `Y` to `Phi(v)`.
                    - If `v` is not already in `Def(v)` for `Y`, add `Y` to `Work(v)`.

        3. Repeat for all variables.
    */

    // Repeat for all variables
    for (const auto &[v_id, def] : def_map) {
        // v_id < 0 is global. it's not supposed to be defined nor accessed

        // Let `Def(v)` be the set of basic blocks where `v` is defined.
        // def is from for loop
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
                if (!phi.contains(y_basic_block) && y_basic_block->get_in_variables().contains(v_id)) {
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

void BuildSSAPass::rename_and_populate_phi(
    const std::map<unsigned int, std::set<BasicBlockPtr>> &def_map,
    const std::map<ControlFlowVertex, std::set<ControlFlowVertex>> &strict_dom_tree_children,
    const ControlFlowGraph &cfg,
    ControlFlowVertex entry)
{
    // the renaming stacks
    std::map<unsigned int, std::vector<std::tuple<unsigned int, BasicBlockPtr>>> stacks;
    // maps the newly assigned id to original id
    std::map<unsigned int, unsigned int> new_id_to_original_id;

    unsigned int max_original_id = 0;
    // def_map contains all non-renamed var
    for (const auto &[variable_id, _] : def_map) {
        stacks[variable_id] = {};
        new_id_to_original_id[variable_id] = variable_id;
        if (variable_id > max_original_id) {
            max_original_id = variable_id;
        }
    }
    unsigned int current_assignable_var_id = max_original_id + 1;

    std::function<void(ControlFlowVertex)> rename_basic_block = [&](const ControlFlowVertex &visiting_block_vertex) {
        const BasicBlockPtr &visiting_basic_block = cfg[visiting_block_vertex];
        std::map<unsigned int, unsigned int> push_count;

        /*
        for each phi in block.phiNodes:
            v = phi.variable
            varVersionCounters[v] += 1
            newVersion = varVersionCounters[v]
            phi.ssaName = v + str(newVersion)
            stacks[v].push(newVersion)
        */
        for (TACPtr &phi_instr : visiting_basic_block->get_instructions()) {
            if (phi_instr->get_op() != IROperation::PHI) {
                continue;
            }

            const Operand &tgt = phi_instr->get_tgt();
            // it's not possible that phi's target is global
            const unsigned int original_tgt_id = tgt.get_register_id();
            assert(original_tgt_id >= 0);
            const unsigned int new_tgt_id = current_assignable_var_id++;
            stacks.at(original_tgt_id).push_back({ new_tgt_id, visiting_basic_block });
            new_id_to_original_id[new_tgt_id] = original_tgt_id;
            push_count[original_tgt_id]++;

            // set the phi node's target to new id
            TACPtr new_phi = ThreeAddressCode::create_phi(new_tgt_id, phi_instr->get_ast_node());
            new_phi->set_phi_incomings(phi_instr->get_phi_incomings());
            spdlog::trace(
                "[SSA Rename] In BB '{}': Renaming phi instruction for %{} to {}",
                visiting_basic_block->get_label(),
                original_tgt_id,
                new_phi->to_string(true));
            phi_instr = new_phi;
        }

        /*
        for each instruction in block.instructions:
            # Rename uses
            for each operand in instruction.operands:
                if isVariable(operand):
                    currentVersion = stacks[operand].top()
                    instruction.replaceUse(operand, operand + str(currentVersion))

            # Rename definitions
            if instruction.definesVariable(v):
                varVersionCounters[v] += 1
                newVersion = varVersionCounters[v]
                instruction.ssaName = v + str(newVersion)
                stacks[v].push(newVersion)
        */
        for (TACPtr &instruction : visiting_basic_block->get_instructions()) {
            if (instruction->get_op() == IROperation::PHI) {
                continue;
            }

            Operand src1 = instruction->get_src1();
            Operand src2 = instruction->get_src2();
            Operand tgt = instruction->get_tgt();

            bool mutated = false;

            if (src1.get_type() == Operand::OperandType::VariableId && src1.get_register_id() >= 0) {
                const unsigned original_id = src1.get_register_id();
                auto &[new_id, _] = stacks.at(original_id).back();
                src1 = Operand(new_id);
                mutated = true;
            }

            if (src2.get_type() == Operand::OperandType::VariableId && src2.get_register_id() >= 0) {
                unsigned original_id = src2.get_register_id();
                auto &[new_id, _] = stacks.at(original_id).back();
                src2 = Operand(new_id);
                mutated = true;
            }

            if (tgt.get_type() == Operand::OperandType::VariableId && tgt.get_register_id() >= 0) {
                unsigned original_tgt_id = tgt.get_register_id();
                unsigned int new_tgt_id = current_assignable_var_id++;

                // create the new operand with new id, and set the name stack, new id => old id map, and count the push
                tgt = Operand(new_tgt_id);
                stacks.at(original_tgt_id).push_back({ new_tgt_id, visiting_basic_block });
                new_id_to_original_id[new_tgt_id] = original_tgt_id;
                push_count[original_tgt_id]++;

                mutated = true;
            }

            if (mutated) {
                TACPtr new_instr = ThreeAddressCode::create(instruction->get_op(), tgt, src1, src2, instruction->get_ast_node());
                spdlog::trace(
                    "[SSA Rename] In BB '{}': Renaming ordinary instruction '{}' to '{}'",
                    visiting_basic_block->get_label(),
                    instruction->to_string(true),
                    new_instr->to_string(true));
                instruction = new_instr;
            }
        }

        /*
        # Rename incoming arguments for phi nodes in successor blocks
        for each successor in block.successors:
            for each phi in successor.phiNodes:
                v = phi.variable
                if stacks[v] is not empty:
                    incomingVersion = stacks[v].top()
                else:
                    incomingVersion = "undef_" + v  # Handle undefined as needed
                phi.addIncoming(v + str(incomingVersion), block)
        */
        for (ControlFlowEdge successor_edge : boost::make_iterator_range(boost::out_edges(visiting_block_vertex, cfg))) {
            ControlFlowVertex successor_vertex = boost::target(successor_edge, cfg);
            const BasicBlockPtr &successor_block = cfg[successor_vertex];
            for (const TACPtr &phi_instr_in_succ_blk : successor_block->get_instructions()) {
                if (phi_instr_in_succ_blk->get_op() != IROperation::PHI) {
                    continue;
                }

                const Operand &tgt = phi_instr_in_succ_blk->get_tgt();
                assert(tgt.get_register_id() >= 0);
                const unsigned int tgt_new_id = tgt.get_register_id();
                const unsigned int tgt_original_id = new_id_to_original_id.at(tgt_new_id);

                // the stack could be empty
                auto stack = stacks.at(tgt_original_id);

                if (stack.empty()) {
                    spdlog::trace(
                        "[SSA Rename] In BB '{}', succ of '{}': Phi's incoming {}'s ({}) is not yet defined",
                        successor_block->get_label(),
                        visiting_basic_block->get_label(),
                        tgt_new_id,
                        tgt_original_id);
                    spdlog::trace("[SSA Rename] Current phi: {}", phi_instr_in_succ_blk->to_string(true));
                } else {
                    auto &[new_id, new_id_def_block] = stacks.at(tgt_original_id).back();
                    phi_instr_in_succ_blk->set_phi_incoming(visiting_basic_block, new_id, new_id_def_block);

                    spdlog::trace(
                        "[SSA Rename] In BB '{}', succ of '{}': Setting phi %{}'s (original %{}) incoming: %{}, defined in '{}'",
                        successor_block->get_label(),
                        visiting_basic_block->get_label(),
                        tgt_new_id,
                        tgt_original_id,
                        new_id,
                        new_id_def_block->get_label());
                    spdlog::trace("[SSA Rename] Populated phi: {}", phi_instr_in_succ_blk->to_string(true));
                }
            }
        }

        /*
        # Recursively rename children in the dominance tree
        for each child in dominanceTree.children(block):
            renameBlock(child)
        */
        // it's possible this node has no dominatee. if it does not exist, we skip
        auto dom_children_set_it = strict_dom_tree_children.find(visiting_block_vertex);
        if (dom_children_set_it != strict_dom_tree_children.end()) {
            const std::set<ControlFlowVertex> &dom_children = strict_dom_tree_children.at(visiting_block_vertex);
            for (ControlFlowVertex dom_child : dom_children) {
                rename_basic_block(dom_child);
            }
        }
        /*
        # After renaming, pop the versions for variables defined in this block
        for each instruction in reverse(block.instructions):
            if instruction.definesVariable(v):
                stacks[v].pop()
        for each phi in reverse(block.phiNodes):
            v = phi.variable
            stacks[v].pop()
        */
        for (const auto &[var_id, count] : push_count) {
            auto &vec = stacks.at(var_id);
            std::size_t new_size = vec.size() - count;
            assert(new_size >= 0);
            vec.resize(new_size);
        }
    };

    rename_basic_block(entry);
}

std::string BuildSSAPass::get_additional_metadata_text(unsigned int task_index, const std::string &path)
{
    UNUSED(task_index);
    UNUSED(path);
    return generate_dominance_tree_graphviz();
}

std::string BuildSSAPass::generate_dominance_tree_graphviz()
{
    std::vector<std::string> subroutine_cfgs;
    for (const auto &[subroutine, tree] : _dominance_trees) {
        auto graphviz_str = GraphvizGenerator::generate_graphviz_cfg_for_subroutine(tree, tree.null_vertex(), subroutine->get_func_name());
        auto fmt = boost::format("subgraph %1% {\nlabel=\"%1%\";")
            % subroutine->get_func_name();
        boost::replace_head(
            graphviz_str,
            sizeof("digraph G{"),

            "subgraph " + subroutine->get_func_name() + "{\nlabel=\"" + subroutine->get_func_name() + "\";");
        subroutine_cfgs.push_back(graphviz_str);
    }

    return "digraph G {\ncluster=true;\n" + boost::join(subroutine_cfgs, "\n") + "\n}";
}

CLOSE_IRGEN_NAMESPACE
// end
