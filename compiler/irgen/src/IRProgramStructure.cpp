#include <sstream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/property_map/function_property_map.hpp>
#include <boost/range/adaptors.hpp>

#include "EscapeGraphviz.h"
#include "IRProgramStructure.h"
#include "TerminalColor.h"
#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

void ProgramMetadata::set_label_alias(const std::string &src, const std::string &tgt)
{
    _label_aliases[src] = tgt;
}

bool ProgramMetadata::get_label_alias(const std::string &src, std::string &tgt)
{
    auto it = _label_aliases.find(src);
    if (it != _label_aliases.end()) {
        tgt = it->second;
        return true;
    } else {
        return false;
    }
}

std::string Subroutine::generate_graphviz_cfg()
{
    boost::dynamic_properties dp;

    dp.property("label", boost::make_function_property_map<ControlFlowVertex>([this](const ControlFlowVertex &v) {
        const BasicBlockPtr &node = _cfg[v];

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
    std::map<ControlFlowVertex, std::string> lbl_map;
    dp.property("node_id", boost::make_function_property_map<ControlFlowVertex>([this, &lbl_id, &lbl_map](const ControlFlowVertex &v) {
        auto lblit = lbl_map.find(v);
        if (lblit == lbl_map.end()) {
            auto id = get_func_name() + std::to_string(lbl_id++);
            lbl_map[v] = id;
            return id;
        } else {
            return lblit->second;
        }
    }));

    dp.property("shape", boost::make_function_property_map<ControlFlowVertex>([this](const ControlFlowVertex &v) {
        return _cfg[v]->get_label() == _cfg[_start_block]->get_label() ? "diamond" : "rect";
    }));

    dp.property("fontname", boost::make_function_property_map<ControlFlowVertex>([this](const ControlFlowVertex &v) {
        return "Courier";
    }));

    std::stringstream dotfile;
    boost::write_graphviz_dp(dotfile, _cfg, dp);
    std::string str = dotfile.str();
    boost::replace_all(str, "\"<<", "<<");
    boost::replace_all(str, ">>\"", ">>");
    return str;
}

std::string Program::generaet_graphviz()
{
    std::vector<std::string> subroutine_cfgs;
    for (const auto &subroutine : _subroutines) {
        auto graphviz_str = subroutine->generate_graphviz_cfg();
        boost::replace_head(
            graphviz_str,
            sizeof("digraph G{"),
            "subgraph " + subroutine->get_func_name() + "{\nlabel=\"" + subroutine->get_func_name() + "\";");
        subroutine_cfgs.push_back(graphviz_str);
    }

    return "digraph G {\ncluster=true;\n" + boost::join(subroutine_cfgs, "\n") + "\n}";
}

std::string Program::to_string(bool color)
{
    std::ostringstream os;

    os << (color ? __tc.C_DARK_YELLOW : "") << "@floor_max" << (color ? __tc.C_RESET : "") << " = " << _metadata.get_floor_max() << std::endl;
    for (const auto [id, value] : _metadata.get_floor_inits()) {
        os << (color ? __tc.C_DARK_YELLOW : "") << "@floor[" << id << "]" << (color ? __tc.C_RESET : "") << " = " << value << std::endl;
    }
    os << std::endl
       << std::endl;

    for (const SubroutinePtr &subroutine : _subroutines) {
        os << (color ? __tc.C_DARK_PINK : "")
           << "def " << subroutine->get_func_name() << (subroutine->has_param() ? "(param)" : "()")
           << " -> " << (subroutine->has_return() ? "value" : "void") << ":"
           << (color ? __tc.C_RESET : "") << std::endl;

        for (const BasicBlockPtr &basic_block : subroutine->get_basic_blocks()) {
            os << (color ? __tc.C_DARK_BLUE : "") << basic_block->get_label() << ":" << (color ? __tc.C_RESET : "") << std::endl;

            for (const TACPtr &instr : basic_block->get_instructions()) {
                os << "    " << instr->to_string(color) << std::endl;
            }

            os << std::endl;
        }
        os << std::endl;
    }

    return os.str();
}

CLOSE_IRGEN_NAMESPACE

// end
