#include <stdexcept>

#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

void ThreeAddressCode::ensure_operand()
{
    switch (_op) {
    case HighLevelIROps::ADD:
    case HighLevelIROps::SUB:
    case HighLevelIROps::MUL:
    case HighLevelIROps::DIV:
    case HighLevelIROps::MOD:
    case HighLevelIROps::AND:
    case HighLevelIROps::OR:
        if (_src1.get_type() != Operand::OperandType::VariableId || _src2.get_type() != Operand::OperandType::VariableId || _tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("Binary arithmetic/logical IR requires src1, src2, tgt to be non-null variable");
        }
        break;

    case HighLevelIROps::NEG:
    case HighLevelIROps::NOT:
        if (_src1.get_type() != Operand::OperandType::VariableId || _tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("Unary arithmetic/logical IR requires src1, tgt to be non-null variable");
        }
        break;

    case HighLevelIROps::JE:
    case HighLevelIROps::JNE:
    case HighLevelIROps::JGT:
    case HighLevelIROps::JLT:
    case HighLevelIROps::JGE:
    case HighLevelIROps::JLE:
        if (_src1.get_type() != Operand::OperandType::VariableId || _src2.get_type() != Operand::OperandType::VariableId || _tgt.get_type() != Operand::OperandType::Label) {
            throw std::runtime_error("Conditional branching IR requires src1, src2 to be non-null variable, and tgt to be label");
        }
        break;

    case HighLevelIROps::INPUT:
    case HighLevelIROps::OUTPUT:
        if (_tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("IO IR requires tgt to be non-null variable");
        }
        break;

    case HighLevelIROps::MOV:
        if (_src1.get_type() != Operand::OperandType::VariableId || _tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("MOV IR requires src1 and tgt to be non-null variable");
        }
        break;

    case HighLevelIROps::LOAD:
        if (!(_src1.get_type() == Operand::OperandType::ImmediateValue || _src1.get_type() == Operand::OperandType::VariableId) || _tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("LOAD IR requires src1 to be either constant or variable, and tgt to be variable");
        }
        break;

    case HighLevelIROps::STORE:
        if (!(_tgt.get_type() == Operand::OperandType::ImmediateValue || _tgt.get_type() == Operand::OperandType::VariableId) || _src1.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("STORE IR requires tgt to be either constant or variable, and src1 to be variable");
        }
        break;

    case HighLevelIROps::LOADI:
        if (_src1.get_type() != Operand::OperandType::ImmediateValue || _tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("LOADI IR requires src1 to be constant, and tgt to be variable");
        }
        break;

    case HighLevelIROps::JMP:
        if (_tgt.get_type() != Operand::OperandType::Label) {
            throw std::runtime_error("JMP IR requires tgt to be label");
        }
        break;

    case HighLevelIROps::CALL:
        if (_src1.get_type() != Operand::OperandType::Label) {
            throw std::runtime_error("CALL IR requires src1 to be label");
        }
        break;

    case HighLevelIROps::RET:
        // If needed, ensure that src1 is handled properly if it's optional.
        break;

    case HighLevelIROps::NOP:
    case HighLevelIROps::HALT:
        break;

    default:
        throw std::runtime_error("Unhandled IR operation");
    }
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::createArithmetic(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2)
{
    if (op != HighLevelIROps::ADD && op != HighLevelIROps::SUB && op != HighLevelIROps::MUL && op != HighLevelIROps::DIV && op != HighLevelIROps::MOD) {
        throw std::invalid_argument("Invalid arithmetic operation");
    }
    if (src1.get_type() != Operand::OperandType::VariableId || src2.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("Arithmetic operation requires all operands to be variables");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, src2));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::createArithmetic(HighLevelIROps op, const Operand &tgt, const Operand &src1)
{
    if (op == HighLevelIROps::NEG) {
        if (src1.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("NEG operation requires src1 and tgt to be variables");
        }
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1));
    } else {
        throw std::invalid_argument("Invalid logical operation");
    }
}

std::shared_ptr<ThreeAddressCode> hrl::irgen::ThreeAddressCode::createComparison(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2)
{
    if (op != HighLevelIROps::EQ && op != HighLevelIROps::NE && op != HighLevelIROps::LT && op != HighLevelIROps::LE && op != HighLevelIROps::GT && op != HighLevelIROps::GE) {
        throw std::invalid_argument("Invalid comparison operation");
    }
    if (src1.get_type() != Operand::OperandType::VariableId || src2.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("Comparison operation requires all operands to be variables");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, src2));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::createLogical(HighLevelIROps op, const Operand &tgt, const Operand &src1)
{
    if (op == HighLevelIROps::NOT) { // Unary operations (NOT, NEG)
        if (src1.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("NOT operation requires src1 and tgt to be variables");
        }
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1));
    } else {
        throw std::invalid_argument("Invalid logical operation");
    }
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::createLogical(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2)
{
    if (op == HighLevelIROps::AND || op == HighLevelIROps::OR) { // Binary logical operations (AND, OR)
        if (src1.get_type() != Operand::OperandType::VariableId || src2.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("Binary logical operations require all operands to be variables");
        }
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, src2));
    } else {
        throw std::invalid_argument("Invalid logical operation");
    }
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::createBranching(HighLevelIROps op, const Operand &src1, const Operand &src2, const Operand &tgt)
{
    if (op != HighLevelIROps::JE && op != HighLevelIROps::JNE && op != HighLevelIROps::JGT && op != HighLevelIROps::JLT && op != HighLevelIROps::JGE && op != HighLevelIROps::JLE && op != HighLevelIROps::JMP) {
        throw std::invalid_argument("Invalid branching operation");
    }
    if (op == HighLevelIROps::JMP) {
        if (tgt.get_type() != Operand::OperandType::Label) {
            throw std::runtime_error("JMP operation requires tgt to be a label");
        }
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt));
    } else {
        if (src1.get_type() != Operand::OperandType::VariableId || src2.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::Label) {
            throw std::runtime_error("Branching operations require src1, src2 to be variables and tgt to be a label");
        }
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, src2));
    }
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::createDataMovement(HighLevelIROps op, const Operand &tgt, const Operand &src1)
{
    if (op == HighLevelIROps::MOV) { // MOV requires both src1 and tgt to be variables
        if (src1.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("MOV operation requires both src1 and tgt to be variables");
        }
    } else if (op == HighLevelIROps::LOAD) { // LOAD can load from a constant or variable to a variable
        if ((src1.get_type() != Operand::OperandType::ImmediateValue && src1.get_type() != Operand::OperandType::VariableId) || tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("LOAD operation requires src1 to be a constant or variable, and tgt to be a variable");
        }
    } else if (op == HighLevelIROps::STORE) { // STORE can store from a variable to a constant or variable
        if (src1.get_type() != Operand::OperandType::VariableId || (tgt.get_type() != Operand::OperandType::ImmediateValue && tgt.get_type() != Operand::OperandType::VariableId)) {
            throw std::runtime_error("STORE operation requires src1 to be a variable and tgt to be a constant or variable");
        }
    } else {
        throw std::invalid_argument("Invalid data movement operation");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::createLoadImmediate(const Operand &tgt, int src1)
{
    if (tgt.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("LOADI operation requires tgt to be a variable");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(HighLevelIROps::LOADI, tgt, Operand(src1, true)));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::createSpecial(HighLevelIROps op)
{
    if (op == HighLevelIROps::NOP || op == HighLevelIROps::HALT) {
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op));
    } else {
        throw std::invalid_argument("Invalid special operation");
    }
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::createIO(HighLevelIROps op, const Operand &val)
{
    if (op == HighLevelIROps::INPUT) {
        if (val.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("IO operations require a variable");
        }
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, val));
    } else if (op == HighLevelIROps::OUTPUT) {
        if (val.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("IO operations require a variable");
        }
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, Operand(), val));
    } else {
        throw std::invalid_argument("Invalid IO operation");
    }
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::createCall(const Operand &label, const Operand &param, const Operand &ret)
{
    if (label.get_type() != Operand::OperandType::Label || param.get_type() != Operand::OperandType::VariableId || ret.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("CALL operation requires label to be a label, param and ret to be variables");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(HighLevelIROps::CALL, ret, label, param));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::createCall(const Operand &label, const Operand &ret)
{
    if (label.get_type() != Operand::OperandType::Label || ret.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("CALL operation requires label to be a label, ret to be variables");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(HighLevelIROps::CALL, ret, label));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::createReturn(const Operand &ret)
{
    if (ret.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("RET operation requires ret to be a variable");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(HighLevelIROps::RET, Operand(), ret));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::createReturn()
{
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(HighLevelIROps::RET));
}

CLOSE_IRGEN_NAMESPACE
// end
