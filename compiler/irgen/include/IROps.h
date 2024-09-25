#ifndef IROPS_H
#define IROPS_H

#include <cstdint>
#include <string>

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
bool is_control_transfer_operation(HighLevelIROps op);

CLOSE_IRGEN_NAMESPACE
#endif