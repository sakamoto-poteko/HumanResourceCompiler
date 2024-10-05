
#include <sstream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/graph/directed_graph.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/property_map/function_property_map.hpp>
#include <boost/range/adaptors.hpp>
#include <spdlog/spdlog.h>

#include "EscapeGraphviz.h"
#include "GraphvizGenerator.h"
#include "IRProgramStructure.h"
#include "ThreeAddressCode.h"
#include "hrl_global.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

std::string GraphvizGenerator::generate_graphviz_bb_graph(const BBGraph &cfg, const BBGraphVertex start_block, const std::string &func_name)
{
    boost::dynamic_properties dp;

    dp.property("label", boost::make_function_property_map<BBGraphVertex>([&cfg](const BBGraphVertex &v) {
        const BasicBlockPtr &node = cfg[v];

        auto instrs = boost::join(
            node->get_instructions() | boost::adaptors::transformed([](const TACPtr &instr) {
                return (boost::format("<TR><TD ALIGN='LEFT'>%1%</TD></TR>")
                    % escape_graphviz_html(boost::algorithm::trim_copy(instr->to_string())))
                    .str();
            }),
            "");

        boost::format xlabel;
        xlabel = boost::format(R"(<<TABLE BORDER='0' CELLBORDER='0'><TR><TD ALIGN='LEFT'><FONT COLOR='#8B4513'>%1%:</FONT></TD></TR>%2%</TABLE>>)")
            % escape_graphviz_html(node->get_label())
            % instrs;
        return xlabel.str();
    }));

    int lbl_id = 0;
    std::map<BBGraphVertex, std::string> lbl_map;
    dp.property("node_id", boost::make_function_property_map<BBGraphVertex>([&lbl_id, &lbl_map, &func_name](const BBGraphVertex &v) {
        auto lblit = lbl_map.find(v);
        if (lblit == lbl_map.end()) {
            auto id = func_name + std::to_string(lbl_id++);
            lbl_map[v] = id;
            return id;
        } else {
            return lblit->second;
        }
    }));

    dp.property("shape", boost::make_function_property_map<BBGraphVertex>([](const BBGraphVertex &v) {
        UNUSED(v);
        return "rect";
    }));

    dp.property("fillcolor", boost::make_function_property_map<BBGraphVertex>([&start_block](const BBGraphVertex &v) {
        UNUSED(v);
        return v == start_block ? "lightyellow" : "white";
    }));

    dp.property("style", boost::make_function_property_map<BBGraphVertex>([](const BBGraphVertex &v) {
        UNUSED(v);
        return "filled";
    }));

    dp.property("fontname", boost::make_function_property_map<BBGraphVertex>([](const BBGraphVertex &v) {
        UNUSED(v);
        return "Courier";
    }));

    std::stringstream dotfile;
    boost::write_graphviz_dp(dotfile, cfg, dp);
    std::string str = dotfile.str();
    boost::replace_all(str, "\"<<", "<<");
    boost::replace_all(str, ">>\"", ">>");
    return str;
}

CLOSE_IRGEN_NAMESPACE
// end
