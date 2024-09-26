#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/range/adaptors.hpp>

#include "EscapeGraphviz.h"
#include "IRProgramStructure.h"
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
    std::stringstream dotfile;
    boost::write_graphviz(
        dotfile,
        _cfg,
        // vertex
        [this](std::ostream &out, ControlFlowVertex &v) {
            const BasicBlockPtr &node = _cfg[v];

            auto instrs = boost::join(
                node->get_instructions() | boost::adaptors::transformed([](const TACPtr &instr) {
                    return (boost::format("<TR><TD ALIGN=\"LEFT\">%1%</TD></TR>")
                        % escape_graphviz_html(boost::algorithm::trim_copy(instr->to_string())))
                        .str();
                }),
                "");

            boost::format xlabel;
            xlabel = boost::format(R"([shape=rect label=<<TABLE BORDER="0" CELLBORDER="0"><TR><TD ALIGN="LEFT"><FONT COLOR="#8B4513">%1%:</FONT></TD></TR>%2%</TABLE>> fontname=Courier])")
                % escape_graphviz_html(node->get_label())
                % instrs;
            out << xlabel;
        },
        // edge
        [](std::ostream &out, const ControlFlowEdge &e) {
            //
            UNUSED(out);
            UNUSED(e);
        },
        // graph
        [&](std::ostream &out) {
            out << boost::format("label=\"%1%\";\nnode[ordering=out];\n") % escape_graphviz(_func_name);
        });

    return dotfile.str();
}

CLOSE_IRGEN_NAMESPACE
// end
