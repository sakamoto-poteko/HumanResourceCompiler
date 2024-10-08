#include <string>

#include <spdlog/spdlog.h>

#include "HRBox.h"
#include "Operand.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

void Operand::rename_register(unsigned int id)
{
    if (_type != OperandType::VariableId) {
        spdlog::critical("operand is not variable id, but '{}' {}", operator std::string(), __PRETTY_FUNCTION__);
        throw;
    }

    if (std::get<int>(_value) < 0) {
        spdlog::critical("operand is a global which cannot be renamed: '{}'. {}", operator std::string(), __PRETTY_FUNCTION__);
        throw;
    }

    std::get<int>(_value) = id;
}

std::string operand_name_get_named_variable(int id)
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

std::string operand_name_get_ssa(int id)
{
    bool global = false;
    if (id < 0) {
        id = -id;
        global = true;
    }

    return (global ? "glb%" : "%") + std::to_string(id);
}

std::string operand_name_get_virtual_reg(int id)
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
            return operand_name_get_named_variable(std::get<int>(_value));
        } else if constexpr (OPERAND_FORMAT == OperandFormat::SSA) {
            return operand_name_get_ssa(std::get<int>(_value));
        } else if constexpr (OPERAND_FORMAT == OperandFormat::VirtualRegisters) {
            return operand_name_get_virtual_reg(std::get<int>(_value));
        } else {
            spdlog::critical("unimplemented Operand to string. {}", __PRETTY_FUNCTION__);
            throw;
        }
    case OperandType::ImmediateValue:
        if constexpr (OPERAND_FORMAT == OperandFormat::NamedVariables) {
            return std::string(std::get<HRBox>(_value));
        } else if constexpr (OPERAND_FORMAT == OperandFormat::SSA || OPERAND_FORMAT == OperandFormat::VirtualRegisters) {
            return "#" + std::string(std::get<HRBox>(_value));
        } else {
            spdlog::critical("unimplemented Operand to string. {}", __PRETTY_FUNCTION__);
            throw;
        }
    case OperandType::Label:
        return std::get<std::string>(_value);
    default:
        spdlog::critical("unknown operand type: {}. {}", static_cast<int>(_type), __PRETTY_FUNCTION__);
        throw;
    }
}

CLOSE_IRGEN_NAMESPACE
// end
