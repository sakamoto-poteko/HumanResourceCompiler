#include <stdexcept>
#include <string>

#include <boost/format.hpp>
#include <spdlog/spdlog.h>

#include "TerminalColor.h"
#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_arithmetic(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2, std::shared_ptr<parser::ASTNode> ast)
{
    if (op != HighLevelIROps::ADD && op != HighLevelIROps::SUB && op != HighLevelIROps::MUL && op != HighLevelIROps::DIV && op != HighLevelIROps::MOD) {
        throw std::invalid_argument("Invalid arithmetic operation");
    }
    if (src1.get_type() != Operand::OperandType::VariableId || src2.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("Arithmetic operation requires all operands to be variables");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, src2, ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_arithmetic(HighLevelIROps op, const Operand &tgt, const Operand &src1, std::shared_ptr<parser::ASTNode> ast)
{
    if (op == HighLevelIROps::NEG) {
        if (src1.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("NEG operation requires src1 and tgt to be variables");
        }
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, Operand(), ast));
    } else {
        throw std::invalid_argument("Invalid logical operation");
    }
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_comparison(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2, std::shared_ptr<parser::ASTNode> ast)
{
    if (op != HighLevelIROps::EQ && op != HighLevelIROps::NE && op != HighLevelIROps::LT && op != HighLevelIROps::LE && op != HighLevelIROps::GT && op != HighLevelIROps::GE) {
        throw std::invalid_argument("Invalid comparison operation");
    }
    if (src1.get_type() != Operand::OperandType::VariableId || src2.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("Comparison operation requires all operands to be variables");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, src2, ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_logical(HighLevelIROps op, const Operand &tgt, const Operand &src1, std::shared_ptr<parser::ASTNode> ast)
{
    if (op == HighLevelIROps::NOT) { // Unary operations (NOT, NEG)
        if (src1.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("NOT operation requires src1 and tgt to be variables");
        }
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, Operand(), ast));
    } else {
        throw std::invalid_argument("Invalid logical operation");
    }
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_logical(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2, std::shared_ptr<parser::ASTNode> ast)
{
    if (op == HighLevelIROps::AND || op == HighLevelIROps::OR) { // Binary logical operations (AND, OR)
        if (src1.get_type() != Operand::OperandType::VariableId || src2.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("Binary logical operations require all operands to be variables");
        }
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, src2, ast));
    } else {
        throw std::invalid_argument("Invalid logical operation");
    }
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_branching(HighLevelIROps op, const Operand &tgt, const Operand &src1, const Operand &src2, std::shared_ptr<parser::ASTNode> ast)
{
    if (op != HighLevelIROps::JE && op != HighLevelIROps::JNE && op != HighLevelIROps::JGT && op != HighLevelIROps::JLT && op != HighLevelIROps::JGE && op != HighLevelIROps::JLE && op != HighLevelIROps::JMP) {
        throw std::invalid_argument("Invalid branching operation");
    }
    if (src1.get_type() != Operand::OperandType::VariableId || src2.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::Label) {
        throw std::runtime_error("Branching operations require src1, src2 to be variables and tgt to be a label");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, src2, ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_branching(HighLevelIROps op, const Operand &tgt, const Operand &src1, std::shared_ptr<parser::ASTNode> ast)
{
    if (op != HighLevelIROps::JNZ && op != HighLevelIROps::JZ) {
        throw std::invalid_argument("Invalid branching operation");
    }
    if (src1.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::Label) {
        throw std::runtime_error("JZ/JNZ operations require src1 to be variables and tgt to be a label");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, Operand(), ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_branching(const Operand &tgt, std::shared_ptr<parser::ASTNode> ast)
{
    if (tgt.get_type() != Operand::OperandType::Label) {
        throw std::runtime_error("JMP operation requires tgt to be a label");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(HighLevelIROps::JMP, tgt, Operand(), Operand(), ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_data_movement(HighLevelIROps op, const Operand &tgt, const Operand &src1, std::shared_ptr<parser::ASTNode> ast)
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
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, Operand(), ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_load_immediate(const Operand &tgt, int imm, std::shared_ptr<parser::ASTNode> ast)
{
    if (tgt.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("LOADI operation requires tgt to be a variable");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(HighLevelIROps::LOADI, tgt, Operand(imm, true), Operand(), ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_special(HighLevelIROps op, std::shared_ptr<parser::ASTNode> ast)
{
    if (op == HighLevelIROps::NOP || op == HighLevelIROps::HALT) {
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, Operand(), Operand(), Operand(), ast));
    } else {
        throw std::invalid_argument("Invalid special operation");
    }
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_io(HighLevelIROps op, const Operand &val, std::shared_ptr<parser::ASTNode> ast)
{
    if (op == HighLevelIROps::INPUT) {
        if (val.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("IO operations require a variable");
        }
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, val, Operand(), Operand(), ast));
    } else if (op == HighLevelIROps::OUTPUT) {
        if (val.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("IO operations require a variable");
        }
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, Operand(), val, Operand(), ast));
    } else {
        throw std::invalid_argument("Invalid IO operation");
    }
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_call(const Operand &label, const Operand &param, const Operand &ret, std::shared_ptr<parser::ASTNode> ast)
{
    if (label.get_type() != Operand::OperandType::Label || param.get_type() != Operand::OperandType::VariableId || ret.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("CALL operation requires label to be a label, param and ret to be variables");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(HighLevelIROps::CALL, ret, label, param, ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_call(const Operand &label, const Operand &ret, std::shared_ptr<parser::ASTNode> ast)
{
    if (label.get_type() != Operand::OperandType::Label || ret.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("CALL operation requires label to be a label, ret to be variables");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(HighLevelIROps::CALL, ret, label, Operand(), ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_return(const Operand &ret, std::shared_ptr<parser::ASTNode> ast)
{
    if (ret.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("RET operation requires ret to be a variable");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(HighLevelIROps::RET, Operand(), ret, Operand(), ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_enter(const Operand &tgt, std::shared_ptr<parser::ASTNode> ast)
{
    if (tgt.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("ENTER operation requires tgt to be a variable");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(HighLevelIROps::ENTER, tgt, Operand(), Operand(), ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_return(std::shared_ptr<parser::ASTNode> ast)
{
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(HighLevelIROps::RET, Operand(), Operand(), Operand(), ast));
}

std::string ThreeAddressCode::to_string() const
{
    auto instr = hir_to_string(_op);
    instr.resize(7, ' ');

    std::ostringstream oss;
    bool first = true;
    oss << __tc.C_DARK_CYAN << instr << __tc.C_RESET;

    if (_tgt) {
        if (!first) {
            oss << ", ";
        }
        first = false;
        oss << std::string(_tgt);
    }
    if (_src1) {
        if (!first) {
            oss << ", ";
        }
        first = false;
        oss << std::string(_src1);
    }
    if (_src2) {
        if (!first) {
            oss << ", ";
        }
        first = false;
        oss << std::string(_src2);
    }
    return oss.str();
}

// end

CLOSE_IRGEN_NAMESPACE
