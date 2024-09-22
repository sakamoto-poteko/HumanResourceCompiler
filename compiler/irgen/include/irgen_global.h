#ifndef IRGEN_GLOBAL_H
#define IRGEN_GLOBAL_H

#define OPEN_IRGEN_NAMESPACE \
    namespace hrl {          \
    namespace irgen {
#define CLOSE_IRGEN_NAMESPACE \
    }                         \
    }

OPEN_IRGEN_NAMESPACE

constexpr int DEFAULT_FLOOR_MAX = 64;

enum class OperandFormat {
    NamedVariables, // a b c
    VirtualRegisters, // v1 v2 v3
    SSA, // %1 %2 %3
};

constexpr OperandFormat OPERAND_FORMAT = OperandFormat::NamedVariables;
// constexpr OperandFormat OPERAND_FORMAT = OperandFormat::SSA;
// constexpr OperandFormat OPERAND_FORMAT = OperandFormat::VirtualRegisters;

CLOSE_IRGEN_NAMESPACE

#endif