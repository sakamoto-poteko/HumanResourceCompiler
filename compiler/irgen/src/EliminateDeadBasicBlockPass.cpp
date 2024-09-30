#include <set>

#include <boost/graph/depth_first_search.hpp>

#include "EliminateDeadBasicBlockPass.h"
#include "IRProgramStructure.h"
#include "hrl_global.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

EliminateDeadBasicBlockPass::~EliminateDeadBasicBlockPass()
{
}

int EliminateDeadBasicBlockPass::run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program)
{
    UNUSED(metadata);
    UNUSED(program);

    ControlFlowGraph &cfg = *subroutine->get_cfg();

    std::vector<boost::default_color_type> color_map(cfg.num_vertices());
    auto vertex_index_map = get(boost::vertex_index, cfg);
    boost::iterator_property_map<std::vector<boost::default_color_type>::iterator, decltype(vertex_index_map)> color_pmap(color_map.begin(), vertex_index_map);

    boost::depth_first_visit(cfg, subroutine->get_start_block(), boost::default_dfs_visitor(), color_pmap);

    std::set<ControlFlowVertex> unvisited;
    for (ControlFlowVertex vertex : boost::make_iterator_range(boost::vertices(cfg))) {
        // White: Vertex has not been discovered yet.
        if (color_pmap[vertex] == boost::color_traits<boost::default_color_type>::white()) {
            unvisited.insert(vertex);
        }
    }

    auto &basic_blocks = subroutine->get_basic_blocks();
    for (ControlFlowVertex vertex : unvisited) {
        basic_blocks.remove(cfg[vertex]);
        cfg.remove_vertex(vertex);
    }
    cfg.renumber_vertex_indices();

    return 0;
}

CLOSE_IRGEN_NAMESPACE
// end
