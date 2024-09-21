#ifndef THREEADDRESSCODE_H
#define THREEADDRESSCODE_H

#include "irgen_global.h"
#include <memory>
#include <string>
#include <variant>

OPEN_IRGEN_NAMESPACE

enum class HighLevelIROps {
    // Arithmetic Operations
    ADD, // add a, b, c
    SUB, // sub a, b, c
    MUL, // mul a, b, c
    DIV, // div a, b, c
    MOD, // mod a, b, c
    NEG, // neg a, b

    // Data Movement Operations
    MOV, // mov a, b
    LOAD, // load a, [100] or load a, (r1)
    STORE, // store a, [100] or store a, (r1)
    LOADI, // loadi a, 10

    // Logical Operations
    AND, // and a, b, c
    OR, // or a, b, c
    NOT, // not a, b

    // Control Flow Operations
    JE, // je a, b, label
    JNE, // jne a, b, label
    JGT, // jgt a, b, label
    JLT, // jlt a, b, label
    JGE, // jge a, b, label
    JLE, // jle a, b, label
    JMP, // jmp label
    CALL, // call label, b, a
    RET, // ret

    // Special Operations
    INPUT, // input a
    OUTPUT, // output a
    NOP, // nop
    HALT // halt

};

class Operand {
public:
    enum class OperandType {
        Register,
        Constant,
        Label,
        Null,
    };

    Operand()
        : _type(OperandType::Null)
    {
    }

    Operand(int value, bool is_immediate = false)
        : _type(is_immediate ? OperandType::Constant : OperandType::Register)
        , _value(value)
    {
    }

    Operand(const std::string &label)
        : _type(OperandType::Label)
        , _value(label)
    {
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

    static std::shared_ptr<ThreeAddressCode> createArithmetic(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2);
    static std::shared_ptr<ThreeAddressCode> createLogical(HighLevelIROps op, const Operand &tgt, const Operand &src1);
    static std::shared_ptr<ThreeAddressCode> createLogical(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2);
    static std::shared_ptr<ThreeAddressCode> createBranching(HighLevelIROps op, const Operand &src1, const Operand &src2, const Operand &tgt);
    static std::shared_ptr<ThreeAddressCode> createDataMovement(HighLevelIROps op, const Operand &tgt, const Operand &src1);
    static std::shared_ptr<ThreeAddressCode> createLoadImmediate(const Operand &tgt, const Operand &src1);
    static std::shared_ptr<ThreeAddressCode> createSpecial(HighLevelIROps op);
    static std::shared_ptr<ThreeAddressCode> createIO(HighLevelIROps op, const Operand &reg);
    static std::shared_ptr<ThreeAddressCode> createCall(const Operand &label, const Operand &param, const Operand &ret);
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

    template <typename T, typename... Args>
    friend std::shared_ptr<T> std::make_shared(Args &&...args);
    
    HighLevelIROps _op;
    Operand _src1;
    Operand _src2;
    Operand _tgt;

    void ensure_operand();
};

using TACPtr = std::shared_ptr<ThreeAddressCode>;

CLOSE_IRGEN_NAMESPACE

#endif