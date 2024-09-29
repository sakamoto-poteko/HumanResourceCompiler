#ifndef OPERAND_H
#define OPERAND_H

#include <string>
#include <variant>

#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class Operand {
public:
    enum class OperandType {
        Null = 0,
        VariableId,
        ImmediateValue,
        Label,
    };

    Operand()
        : _type(OperandType::Null)
    {
    }

    Operand(int value, bool is_immediate = false)
        : _type(is_immediate ? OperandType::ImmediateValue : OperandType::VariableId)
        , _value(value)
    {
    }

    Operand(const std::string &label)
        : _type(OperandType::Label)
        , _value(label)
    {
    }

    operator bool() const
    {
        return _type != OperandType::Null;
    }

    operator std::string() const;

    OperandType get_type() const { return _type; }

    int get_register_id() const { return std::get<int>(_value); }

    int get_constant() const { return std::get<int>(_value); }

    std::string get_label() const { return std::get<std::string>(_value); }

    void rename_register(unsigned int id);

private:
    OperandType _type;

    // imm value: the int value
    // reg: the int reg id. negative represents global
    // label: string
    std::variant<int, std::string> _value;
};

CLOSE_IRGEN_NAMESPACE

#endif