#ifndef THREEADDRESSCODE_H
#define THREEADDRESSCODE_H

#include <cstdint>
#include <memory>
#include <string>
#include <variant>

#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

enum class HighLevelIROps : std::uint8_t {
    // Data Movement Operations (0x00 - 0x03)
    MOV = 0x00, // mov a, b
    LOAD = 0x01, // load a, [100] or load a, (r1)
    STORE = 0x02, // store a, [100] or store a, (r1)
    LOADI = 0x03, // loadi a, 10

    // Arithmetic Operations (0x10 - 0x15)
    ADD = 0x10, // add a, b, c
    SUB = 0x11, // sub a, b, c
    MUL = 0x12, // mul a, b, c
    DIV = 0x13, // div a, b, c
    MOD = 0x14, // mod a, b, c
    NEG = 0x15, // neg a, b

    // Logical Operations (0x20 - 0x22)
    AND = 0x20, // and a, b, c
    OR = 0x21, // or a, b, c
    NOT = 0x22, // not a, b

    // Comparison (0x30 - 0x35)
    EQ = 0x30, // eq a, b
    NE = 0x31, // ne a, b
    LT = 0x32, // lt a, b
    LE = 0x33, // le a, b
    GT = 0x34, // gt a, b
    GE = 0x35, // ge a, b

    // Control Flow Operations (0x40 - 0x4C)
    JE = 0x40, // je a, b, label
    JNE = 0x41, // jne a, b, label
    JGT = 0x42, // jgt a, b, label
    JLT = 0x43, // jlt a, b, label
    JGE = 0x44, // jge a, b, label
    JLE = 0x45, // jle a, b, label
    JZ = 0x46, // jz a, label
    JNZ = 0x47, // jnz a, label
    JMP = 0x48, // jmp label

    // Call and Return (0x50 - 0x52)
    CALL = 0x50, // call label, b, a
    ENTER = 0x51, // on function called
    RET = 0x52, // ret

    // IO (0x60 - 0x61)
    INPUT = 0x60, // input a
    OUTPUT = 0x61, // output a

    // Special Operations (0x70 - 0x71)
    NOP = 0x70, // nop
    HALT = 0x71, // halt
};

std::string hir_to_string(HighLevelIROps op);

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

    operator std::string() const
    {
        switch (_type) {
        case OperandType::Null:
            return "(null)";
        case OperandType::VariableId:
            return "v" + std::to_string(std::get<int>(_value));
        case OperandType::ImmediateValue:
            return std::to_string(std::get<int>(_value));
        case OperandType::Label:
            return std::get<std::string>(_value);
        }
    }

    OperandType get_type() const { return _type; }

    int get_register_id() const { return std::get<int>(_value); }

    int get_constant() const { return std::get<int>(_value); }

    std::string get_label() const { return std::get<std::string>(_value); }

private:
    OperandType _type;

    std::variant<int, std::string> _value;
};

class ThreeAddressCode : public std::enable_shared_from_this<ThreeAddressCode> {
public:
    ~ThreeAddressCode() = default;

    HighLevelIROps get_op() const { return _op; }

    const Operand &get_src1() const { return _src1; }

    const Operand &get_src2() const { return _src2; }

    const Operand &get_tgt() const { return _tgt; }

    std::string to_string() const;

    static std::shared_ptr<ThreeAddressCode> createArithmetic(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2);
    static std::shared_ptr<ThreeAddressCode> createArithmetic(HighLevelIROps op, const Operand &tgt, const Operand &src1);
    static std::shared_ptr<ThreeAddressCode> createComparison(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2);
    static std::shared_ptr<ThreeAddressCode> createLogical(HighLevelIROps op, const Operand &tgt, const Operand &src1);
    static std::shared_ptr<ThreeAddressCode> createLogical(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2);
    static std::shared_ptr<ThreeAddressCode> createBranching(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2);
    static std::shared_ptr<ThreeAddressCode> createBranching(HighLevelIROps op, const Operand &tgt, const Operand &src1);
    static std::shared_ptr<ThreeAddressCode> createBranching(const Operand &tgt);
    static std::shared_ptr<ThreeAddressCode> createDataMovement(HighLevelIROps op, const Operand &tgt, const Operand &src1);
    static std::shared_ptr<ThreeAddressCode> createLoadImmediate(const Operand &tgt, int src1);
    static std::shared_ptr<ThreeAddressCode> createSpecial(HighLevelIROps op);
    static std::shared_ptr<ThreeAddressCode> createIO(HighLevelIROps op, const Operand &val);
    static std::shared_ptr<ThreeAddressCode> createCall(const Operand &label, const Operand &param, const Operand &ret);
    static std::shared_ptr<ThreeAddressCode> createCall(const Operand &label, const Operand &ret);
    static std::shared_ptr<ThreeAddressCode> createEnter(const Operand &tgt);
    static std::shared_ptr<ThreeAddressCode> createReturn();
    static std::shared_ptr<ThreeAddressCode> createReturn(const Operand &ret);

private:
    ThreeAddressCode(HighLevelIROps op, const Operand &tgt = Operand(), const Operand &src1 = Operand(), const Operand &src2 = Operand())
        : _op(op)
        , _src1(src1)
        , _src2(src2)
        , _tgt(tgt)
    {
    }

    HighLevelIROps _op;
    Operand _src1;
    Operand _src2;
    Operand _tgt;
};

using TACPtr = std::shared_ptr<ThreeAddressCode>;

CLOSE_IRGEN_NAMESPACE

#endif