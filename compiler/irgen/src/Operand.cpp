#include <string>

#include <spdlog/spdlog.h>

#include "Operand.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

static std::string get_named_variable(int id)
{
    bool global = false;
    if (id < 0) {
        id = -id;
        global = true;
    }

    if (id > 26 * 26) {
        spdlog::critical("id is out of range. not yet implemented. {}", __PRETTY_FUNCTION__);
        throw;
    }

    std::string result(4, '\0');
    if (id < 26) {
        result[0] = static_cast<char>('a' + id);
        if (global) {
            result[1] = '_';
        }
    } else {
        result[0] = static_cast<char>('a' + id / 26);
        result[1] = static_cast<char>('a' + id % 26);
        if (global) {
            result[2] = '_';
        }
    }
    return result;
}

static std::string get_ssa(int id)
{
    bool global = false;
    if (id < 0) {
        id = -id;
        global = true;
    }

    return (global ? "glb%" : "%") + std::to_string(id);
}

static std::string get_virtual_reg(int id)
{
    bool global = false;
    if (id < 0) {
        id = -id;
        global = true;
    }
    return (global ? "gv" : "v") + std::to_string(id);
}

Operand::operator std::string() const
{
    switch (_type) {
    case OperandType::Null:
        return "(null)";
    case OperandType::VariableId:
        if constexpr (OPERAND_FORMAT == OperandFormat::NamedVariables) {
            return get_named_variable(std::get<int>(_value));
        } else if constexpr (OPERAND_FORMAT == OperandFormat::SSA) {
            return get_ssa(std::get<int>(_value));
        } else if constexpr (OPERAND_FORMAT == OperandFormat::VirtualRegisters) {
            return get_virtual_reg(std::get<int>(_value));
        } else {
            spdlog::critical("unimplemented Operand to string. {}", __PRETTY_FUNCTION__);
            throw;
        }
    case OperandType::ImmediateValue:
        if constexpr (OPERAND_FORMAT == OperandFormat::NamedVariables) {
            return std::to_string(std::get<int>(_value));
        } else if constexpr (OPERAND_FORMAT == OperandFormat::SSA || OPERAND_FORMAT == OperandFormat::VirtualRegisters) {
            return "#" + std::to_string(std::get<int>(_value));
        } else {
            spdlog::critical("unimplemented Operand to string. {}", __PRETTY_FUNCTION__);
            throw;
        }
    case OperandType::Label:
        return std::get<std::string>(_value);
    }
}

CLOSE_IRGEN_NAMESPACE
// end
