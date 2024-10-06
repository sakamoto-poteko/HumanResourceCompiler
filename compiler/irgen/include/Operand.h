#ifndef OPERAND_H
#define OPERAND_H

#include <string>
#include <variant>

#include "HRBox.h"
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

    explicit Operand()
        : _type(OperandType::Null)
    {
    }

    explicit Operand(int value)
        : _type(OperandType::VariableId)
        , _value(value)
    {
    }

    explicit Operand(const HRBox &value)
        : _type(OperandType::ImmediateValue)
        , _value(value)
    {
    }

    explicit Operand(const std::string &label)
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

    bool is_local_register() const { return _type == OperandType::VariableId && std::get<int>(_value) >= 0; }

    HRBox get_constant() const { return std::get<HRBox>(_value); }

    std::string get_label() const { return std::get<std::string>(_value); }

    void rename_register(unsigned int id);

private:
    OperandType _type;

    // imm value: the HRBox value
    // reg: the int reg id. negative represents global
    // label: string
    std::variant<int, std::string, HRBox> _value;
};

CLOSE_IRGEN_NAMESPACE

#endif