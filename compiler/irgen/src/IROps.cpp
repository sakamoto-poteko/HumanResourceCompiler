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

bool is_control_flow_op(HighLevelIROps op)
{
    switch (op) {
    case HighLevelIROps::MOV:
    case HighLevelIROps::LOAD:
    case HighLevelIROps::STORE:
    case HighLevelIROps::LOADI:
    case HighLevelIROps::ADD:
    case HighLevelIROps::SUB:
    case HighLevelIROps::MUL:
    case HighLevelIROps::DIV:
    case HighLevelIROps::MOD:
    case HighLevelIROps::NEG:
    case HighLevelIROps::AND:
    case HighLevelIROps::OR:
    case HighLevelIROps::NOT:
    case HighLevelIROps::EQ:
    case HighLevelIROps::NE:
    case HighLevelIROps::LT:
    case HighLevelIROps::LE:
    case HighLevelIROps::GT:
    case HighLevelIROps::GE:
    case HighLevelIROps::INPUT:
    case HighLevelIROps::OUTPUT:
    case HighLevelIROps::NOP:
        return false;

    case HighLevelIROps::JE:
    case HighLevelIROps::JNE:
    case HighLevelIROps::JGT:
    case HighLevelIROps::JLT:
    case HighLevelIROps::JGE:
    case HighLevelIROps::JLE:
    case HighLevelIROps::JZ:
    case HighLevelIROps::JNZ:
    case HighLevelIROps::JMP:
    case HighLevelIROps::CALL:
    case HighLevelIROps::ENTER:
    case HighLevelIROps::RET:
    case HighLevelIROps::HALT:
        return true;
    }
}

CLOSE_IRGEN_NAMESPACE
// end
