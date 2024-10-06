#include <algorithm>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/property_map/function_property_map.hpp>
#include <boost/range/adaptors.hpp>
#include <spdlog/spdlog.h>

#include "GraphvizGenerator.h"
#include "IRProgramStructure.h"
#include "Operand.h"
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

std::string Program::generaet_graphviz()
{
    std::vector<std::string> subroutine_cfgs;
    for (const auto &subroutine : _subroutines) {
        auto graphviz_str = GraphvizGenerator::generate_graphviz_bb_graph(*subroutine->get_cfg(), subroutine->get_cfg_entry(), subroutine->get_func_name());
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

std::string Program::to_string(bool color)
{
    std::ostringstream os;

    const TerminalColor &tc = color ? __tc : __empty_tc;

    os << tc.C_DARK_YELLOW << "@floor_max" << tc.C_RESET << " = " << _metadata.get_floor_max() << std::endl;
    for (const auto &[id, value] : _metadata.get_floor_inits()) {
        os << tc.C_DARK_YELLOW << "@floor[" << id << "]" << tc.C_RESET << " = " << std::string(value) << std::endl;
    }
    os << std::endl
       << std::endl;

    for (const SubroutinePtr &subroutine : _subroutines) {
        os << tc.C_DARK_PINK
           << "def " << subroutine->get_func_name() << (subroutine->has_param() ? "(param)" : "()")
           << " -> " << (subroutine->has_return() ? "value" : "void") << ":"
           << tc.C_RESET << std::endl;
        os << tc.C_DARK_GREEN << "// max_reg = " << subroutine->get_max_reg_id() << tc.C_RESET << std::endl;

        for (const BasicBlockPtr &basic_block : subroutine->get_basic_blocks()) {
            os << tc.C_DARK_BLUE << basic_block->get_label() << ":" << tc.C_RESET << std::endl;

            for (const TACPtr &instr : basic_block->get_instructions()) {
                os << "    " << instr->to_string(color) << std::endl;
            }

            os << std::endl;
        }
        os << std::endl;
    }

    return os.str();
}

BasicBlock::~BasicBlock()
{
    spdlog::trace("BB '{}' is destructed", get_label());
}

unsigned int BasicBlock::get_max_reg_id() const
{
    int max = 0;
    for (const TACPtr &instruction : get_instructions()) {
        int tgt = 0, src1 = 0, src2 = 0;
        if (instruction->get_tgt().get_type() == Operand::OperandType::VariableId) {
            tgt = instruction->get_tgt().get_register_id();
        }

        if (instruction->get_src1().get_type() == Operand::OperandType::VariableId) {
            src1 = instruction->get_src1().get_register_id();
        }

        if (instruction->get_src2().get_type() == Operand::OperandType::VariableId) {
            src2 = instruction->get_src2().get_register_id();
        }

        max = std::max({ max, tgt, src1, src2 });
    }

    if (max < 0) {
        max = 0;
    }

    return max;
}

unsigned int Subroutine::get_max_reg_id() const
{
    unsigned int max = std::ranges::max(get_basic_blocks() | std::views::transform([](const BasicBlockPtr &basic_block) {
        return basic_block->get_max_reg_id();
    }));

    return max;
}

CLOSE_IRGEN_NAMESPACE
// end
