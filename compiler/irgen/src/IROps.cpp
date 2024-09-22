#include <spdlog/spdlog.h>

#include "IROps.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

std::string hir_to_string(HighLevelIROps op)
{
    switch (op) {
    // arithmetic operations
    case HighLevelIROps::ADD:
        return "add";
    case HighLevelIROps::SUB:
        return "sub";
    case HighLevelIROps::MUL:
        return "mul";
    case HighLevelIROps::DIV:
        return "div";
    case HighLevelIROps::MOD:
        return "mod";
    case HighLevelIROps::NEG:
        return "neg";

    // data movement operations
    case HighLevelIROps::MOV:
        return "mov";
    case HighLevelIROps::LOAD:
        return "load";
    case HighLevelIROps::STORE:
        return "store";
    case HighLevelIROps::LOADI:
        return "loadi";

    // logical operations
    case HighLevelIROps::AND:
        return "and";
    case HighLevelIROps::OR:
        return "or";
    case HighLevelIROps::NOT:
        return "not";

    // comparison
    case HighLevelIROps::EQ:
        return "eq";
    case HighLevelIROps::NE:
        return "ne";
    case HighLevelIROps::LT:
        return "lt";
    case HighLevelIROps::LE:
        return "le";
    case HighLevelIROps::GT:
        return "gt";
    case HighLevelIROps::GE:
        return "ge";

    // control flow operations
    case HighLevelIROps::JE:
        return "je";
    case HighLevelIROps::JNE:
        return "jne";
    case HighLevelIROps::JGT:
        return "jgt";
    case HighLevelIROps::JLT:
        return "jlt";
    case HighLevelIROps::JGE:
        return "jge";
    case HighLevelIROps::JLE:
        return "jle";
    case HighLevelIROps::JZ:
        return "jz";
    case HighLevelIROps::JNZ:
        return "jnz";
    case HighLevelIROps::JMP:
        return "jmp";
    case HighLevelIROps::CALL:
        return "call";
    case HighLevelIROps::ENTER:
        return "enter";
    case HighLevelIROps::RET:
        return "ret";

    // special operations
    case HighLevelIROps::INPUT:
        return "input";
    case HighLevelIROps::OUTPUT:
        return "output";
    case HighLevelIROps::NOP:
        return "nop";
    case HighLevelIROps::HALT:
        return "halt";

    default:
        spdlog::critical("Unknown HighLevelIROps {}. {}", static_cast<int>(op), __PRETTY_FUNCTION__);
        throw;
    }
}

CLOSE_IRGEN_NAMESPACE
// end
