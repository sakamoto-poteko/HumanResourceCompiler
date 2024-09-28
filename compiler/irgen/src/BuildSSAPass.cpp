#include <boost/graph/dominator_tree.hpp>
#include <spdlog/spdlog.h>

#include "BuildSSAPass.h"
#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

BuildSSAPass::~BuildSSAPass()
{
}

std::vector<std::vector<ControlFlowVertex>> hrl::irgen::BuildSSAPass::build_dominance_frontiers(const ControlFlowGraphPtr &graph, const ControlFlowVertex &start_block)
{
    ControlFlowGraph &cfg = *graph;
    auto index_map = get(boost::vertex_index, cfg);
    std::vector<ControlFlowVertex> dom_tree(cfg.num_vertices(), cfg.null_vertex());

    auto [vert_begin_it, vert_end_it] = boost::vertices(cfg);

    int idx_i = 0;
    for (auto it = vert_begin_it; it != vert_end_it; ++it, ++idx_i) {
        boost::put(index_map, *it, idx_i);
    }
    auto dom_tree_map = make_iterator_property_map(dom_tree.begin(), index_map);

    boost::lengauer_tarjan_dominator_tree(cfg, boost::vertex(0, cfg), dom_tree_map);

    for (std::size_t i = 0; i < dom_tree.size(); ++i) {
        const auto &node_lbl = cfg[boost::vertex(i, cfg)]->get_label();
        if (dom_tree[i] == cfg.null_vertex()) {
            spdlog::debug("BB '{}' is not imm dominated", node_lbl);
        } else {
            spdlog::debug("BB '{}' is imm dominated by '{}'", node_lbl, cfg[dom_tree[i]]->get_label());
        }
    }

    // Compute the dominance frontier
    std::vector<std::vector<ControlFlowVertex>> dom_frontier(cfg.num_vertices());

    // auto [vert_begin, vert_end] = boost::vertices(cfg);
    // for (auto vert_it = vert_begin; vert_it != vert_end; ++vert_it) {
    //     auto [in_edge_begin, in_edge_end] = boost::in_edges(*vert_it, cfg);
    //     for (auto in_edge_it = in_edge_begin; in_edge_it != in_edge_end; ++in_edge_it) {
    //         ControlFlowVertex pred = boost::source(*in_edge_it, cfg);
    //         ControlFlowVertex runner = pred;
    //         while (runner != dom_tree[b]) {
    //             dom_frontier[runner].push_back(b);
    //             runner = dom_tree[runner];
    //         }
    //     }
    // }

    // // Print the dominance frontier for each node
    // for (std::size_t i = 0; i < dom_frontier.size(); ++i) {
    //     std::cout << "Dominance frontier of node " << i << ": ";
    //     for (Vertex v : dom_frontier[i]) {
    //         std::cout << v << " ";
    //     }
    //     std::cout << std::endl;
    // }
    return dom_frontier;
}

int BuildSSAPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    build_dominance_frontiers(subroutine->get_cfg(), subroutine->get_start_block());
    return 0;
}

CLOSE_IRGEN_NAMESPACE

// end
