#include <spdlog/spdlog.h>

#include "IROps.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

std::string IROperationMetadata::to_string(IROperation op)
{
    switch (op) {
    // arithmetic operations
    case IROperation::ADD:
        return "add";
    case IROperation::SUB:
        return "sub";
    case IROperation::MUL:
        return "mul";
    case IROperation::DIV:
        return "div";
    case IROperation::MOD:
        return "mod";
    case IROperation::NEG:
        return "neg";

    // data movement operations
    case IROperation::MOV:
        return "mov";
    case IROperation::LOAD:
        return "ld";
    case IROperation::STORE:
        return "st";
    case IROperation::LOADI:
        return "ldi";

    // logical operations
    case IROperation::AND:
        return "and";
    case IROperation::OR:
        return "or";
    case IROperation::NOT:
        return "not";

    // comparison
    case IROperation::EQ:
        return "eq";
    case IROperation::NE:
        return "ne";
    case IROperation::LT:
        return "lt";
    case IROperation::LE:
        return "le";
    case IROperation::GT:
        return "gt";
    case IROperation::GE:
        return "ge";

    // control flow operations
    case IROperation::JE:
        return "je";
    case IROperation::JNE:
        return "jne";
    case IROperation::JGT:
        return "jgt";
    case IROperation::JLT:
        return "jlt";
    case IROperation::JGE:
        return "jge";
    case IROperation::JLE:
        return "jle";
    case IROperation::JZ:
        return "jz";
    case IROperation::JNZ:
        return "jnz";
    case IROperation::JMP:
        return "jmp";
    case IROperation::CALL:
        return "c";
    case IROperation::ENTER:
        return "ent";
    case IROperation::RET:
        return "ret";

    // special operations
    case IROperation::INPUT:
        return "in";
    case IROperation::OUTPUT:
        return "out";
    case IROperation::NOP:
        return "nop";
    case IROperation::HALT:
        return "hlt";

    case IROperation::PHI:
        return "phi";
    }

    spdlog::critical("Unknown IR Op {}. {}", static_cast<int>(op), __PRETTY_FUNCTION__);
    throw;
}

bool IROperationMetadata::is_branch(IROperation op)
{
    switch (op) {
    case IROperation::MOV:
    case IROperation::LOAD:
    case IROperation::STORE:
    case IROperation::LOADI:
    case IROperation::ADD:
    case IROperation::SUB:
    case IROperation::MUL:
    case IROperation::DIV:
    case IROperation::MOD:
    case IROperation::NEG:
    case IROperation::AND:
    case IROperation::OR:
    case IROperation::NOT:
    case IROperation::EQ:
    case IROperation::NE:
    case IROperation::LT:
    case IROperation::LE:
    case IROperation::GT:
    case IROperation::GE:
    case IROperation::INPUT:
    case IROperation::OUTPUT:
    case IROperation::NOP:
    case IROperation::CALL:
    case IROperation::ENTER:
    case IROperation::RET:
    case IROperation::HALT:
    case IROperation::PHI:
        return false;

    case IROperation::JE:
    case IROperation::JNE:
    case IROperation::JGT:
    case IROperation::JLT:
    case IROperation::JGE:
    case IROperation::JLE:
    case IROperation::JZ:
    case IROperation::JNZ:
    case IROperation::JMP:
        return true;
    }

    spdlog::critical("Unknown IR Op {}. {}", static_cast<int>(op), __PRETTY_FUNCTION__);
    throw;
}

bool IROperationMetadata::is_control_transfer(IROperation op)
{
    switch (op) {
    case IROperation::MOV:
    case IROperation::LOAD:
    case IROperation::STORE:
    case IROperation::LOADI:
    case IROperation::ADD:
    case IROperation::SUB:
    case IROperation::MUL:
    case IROperation::DIV:
    case IROperation::MOD:
    case IROperation::NEG:
    case IROperation::AND:
    case IROperation::OR:
    case IROperation::NOT:
    case IROperation::EQ:
    case IROperation::NE:
    case IROperation::LT:
    case IROperation::LE:
    case IROperation::GT:
    case IROperation::GE:
    case IROperation::INPUT:
    case IROperation::OUTPUT:
    case IROperation::NOP:
    case IROperation::PHI:
    // call is not for now
    case IROperation::CALL:
        return false;

    case IROperation::JE:
    case IROperation::JNE:
    case IROperation::JGT:
    case IROperation::JLT:
    case IROperation::JGE:
    case IROperation::JLE:
    case IROperation::JZ:
    case IROperation::JNZ:
    case IROperation::JMP:
    case IROperation::ENTER:
    case IROperation::RET:
    case IROperation::HALT:
        return true;
    }

    spdlog::critical("Unknown IR Op {}. {}", static_cast<int>(op), __PRETTY_FUNCTION__);
    throw;
}

bool IROperationMetadata::is_comparison(IROperation op)
{
    return op >= IROperation::EQ && op <= IROperation::GE;
}

bool IROperationMetadata::has_side_effect(IROperation op)
{
    switch (op) {
    case IROperation::LOAD:
    case IROperation::STORE:
    case IROperation::RET:
    case IROperation::INPUT:
    case IROperation::OUTPUT:
    case IROperation::HALT:
    case IROperation::JE:
    case IROperation::JNE:
    case IROperation::JGT:
    case IROperation::JLT:
    case IROperation::JGE:
    case IROperation::JLE:
    case IROperation::JZ:
    case IROperation::JNZ:
    case IROperation::JMP:
    case IROperation::CALL:
        return true;

    case IROperation::MOV:
    case IROperation::LOADI:
    case IROperation::ADD:
    case IROperation::SUB:
    case IROperation::MUL:
    case IROperation::DIV:
    case IROperation::MOD:
    case IROperation::NEG:
    case IROperation::AND:
    case IROperation::OR:
    case IROperation::NOT:
    case IROperation::EQ:
    case IROperation::NE:
    case IROperation::LT:
    case IROperation::LE:
    case IROperation::GT:
    case IROperation::GE:
    case IROperation::ENTER:
    case IROperation::NOP:
    case IROperation::PHI:
        return false;
    }
}

CLOSE_IRGEN_NAMESPACE
// end
