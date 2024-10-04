#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptors.hpp>
#include <spdlog/spdlog.h>

#include "HRBox.h"
#include "IROps.h"
#include "IRProgramStructure.h" // IWYU pragma: keep
#include "Operand.h"
#include "TerminalColor.h"
#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_arithmetic(IROperation op, const Operand &tgt, const Operand &src1, const Operand &src2, std::shared_ptr<parser::ASTNode> ast)
{
    if (op != IROperation::ADD && op != IROperation::SUB && op != IROperation::MUL && op != IROperation::DIV && op != IROperation::MOD) {
        throw std::invalid_argument("Invalid arithmetic operation");
    }
    if (src1.get_type() != Operand::OperandType::VariableId || src2.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("Arithmetic operation requires all operands to be variables");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, src2, ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_arithmetic(IROperation op, const Operand &tgt, const Operand &src1, std::shared_ptr<parser::ASTNode> ast)
{
    if (op == IROperation::NEG) {
        if (src1.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("NEG operation requires src1 and tgt to be variables");
        }
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, Operand(), ast));
    } else {
        throw std::invalid_argument("Invalid NEG operation");
    }
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_comparison(IROperation op, const Operand &tgt, const Operand &src1, const Operand &src2, std::shared_ptr<parser::ASTNode> ast)
{
    if (op != IROperation::EQ && op != IROperation::NE && op != IROperation::LT && op != IROperation::LE && op != IROperation::GT && op != IROperation::GE) {
        throw std::invalid_argument("Invalid comparison operation");
    }
    if (src1.get_type() != Operand::OperandType::VariableId || src2.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("Comparison operation requires all operands to be variables");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, src2, ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_logical(IROperation op, const Operand &tgt, const Operand &src1, std::shared_ptr<parser::ASTNode> ast)
{
    if (op == IROperation::NOT) { // Unary operations (NOT, NEG)
        if (src1.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("NOT operation requires src1 and tgt to be variables");
        }
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, Operand(), ast));
    } else {
        throw std::invalid_argument("Invalid NOT operation");
    }
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_logical(IROperation op, const Operand &tgt, const Operand &src1, const Operand &src2, std::shared_ptr<parser::ASTNode> ast)
{
    if (op == IROperation::AND || op == IROperation::OR) { // Binary logical operations (AND, OR)
        if (src1.get_type() != Operand::OperandType::VariableId || src2.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("Binary logical operations require all operands to be variables");
        }
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, src2, ast));
    } else {
        throw std::invalid_argument("Invalid AND/OR operation");
    }
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_branching(IROperation op, const Operand &tgt, const Operand &src1, const Operand &src2, std::shared_ptr<parser::ASTNode> ast)
{
    if (op != IROperation::JE && op != IROperation::JNE && op != IROperation::JGT && op != IROperation::JLT && op != IROperation::JGE && op != IROperation::JLE) {
        throw std::invalid_argument("Invalid branching operation");
    }
    if (src1.get_type() != Operand::OperandType::VariableId || src2.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::Label) {
        throw std::runtime_error("Branching operations require src1, src2 to be variables and tgt to be a label");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, src2, ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_branching(IROperation op, const Operand &tgt, const Operand &src1, std::shared_ptr<parser::ASTNode> ast)
{
    if (op != IROperation::JNZ && op != IROperation::JZ) {
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
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(IROperation::JMP, tgt, Operand(), Operand(), ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_data_movement(IROperation op, const Operand &tgt, const Operand &src1, const Operand &src2, std::shared_ptr<parser::ASTNode> ast)
{
    if (op == IROperation::MOV) { // MOV requires both src1 and tgt to be variables
        if (src1.get_type() != Operand::OperandType::VariableId || tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("MOV operation requires both src1 and tgt to be variables");
        }
    } else if (op == IROperation::LOAD) { // LOAD can load from a constant or variable to a variable
        if ((src1.get_type() != Operand::OperandType::ImmediateValue && src1.get_type() != Operand::OperandType::VariableId) || tgt.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("LOAD operation requires src1 (where to load) to be a constant or variable, and tgt (load to where) to be a variable");
        }
    } else if (op == IROperation::STORE) { // STORE can store from a variable to a constant or variable
        if ((src1.get_type() != Operand::OperandType::ImmediateValue && src1.get_type() != Operand::OperandType::VariableId) || src2.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("STORE operation requires src1 (where to store) to be a constant or variable and src2 (what to store) to be a variable ");
        }
    } else {
        throw std::invalid_argument("Invalid data movement operation");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, src2, ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_load_immediate(const Operand &tgt, HRBox imm, std::shared_ptr<parser::ASTNode> ast)
{
    if (tgt.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("LOADI operation requires tgt to be a variable");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(IROperation::LOADI, tgt, Operand(imm), Operand(), ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_special(IROperation op, std::shared_ptr<parser::ASTNode> ast)
{
    if (op == IROperation::NOP || op == IROperation::HALT) {
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, Operand(), Operand(), Operand(), ast));
    } else {
        throw std::invalid_argument("Invalid special operation");
    }
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_io(IROperation op, const Operand &val, std::shared_ptr<parser::ASTNode> ast)
{
    if (op == IROperation::INPUT) {
        if (val.get_type() != Operand::OperandType::VariableId) {
            throw std::runtime_error("IO operations require a variable");
        }
        return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, val, Operand(), Operand(), ast));
    } else if (op == IROperation::OUTPUT) {
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
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(IROperation::CALL, ret, label, param, ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_call(const Operand &label, const Operand &ret, std::shared_ptr<parser::ASTNode> ast)
{
    if (label.get_type() != Operand::OperandType::Label || ret.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("CALL operation requires label to be a label, ret to be variables");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(IROperation::CALL, ret, label, Operand(), ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_return(const Operand &ret, std::shared_ptr<parser::ASTNode> ast)
{
    if (ret.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("RET operation requires ret to be a variable");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(IROperation::RET, Operand(), ret, Operand(), ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_enter(const Operand &tgt, std::shared_ptr<parser::ASTNode> ast)
{
    if (tgt.get_type() != Operand::OperandType::VariableId) {
        throw std::runtime_error("ENTER operation requires tgt to be a variable");
    }
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(IROperation::ENTER, tgt, Operand(), Operand(), ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_return(std::shared_ptr<parser::ASTNode> ast)
{
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(IROperation::RET, Operand(), Operand(), Operand(), ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create(IROperation op, const Operand &tgt, const Operand &src1, const Operand &src2, std::shared_ptr<parser::ASTNode> ast)
{
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(op, tgt, src1, src2, ast));
}

std::shared_ptr<ThreeAddressCode> ThreeAddressCode::create_phi(int var_id, std::shared_ptr<parser::ASTNode> ast)
{
    return std::shared_ptr<ThreeAddressCode>(new ThreeAddressCode(IROperation::PHI, Operand(var_id), Operand(), Operand(), ast));
}

std::string ThreeAddressCode::to_string(bool with_color) const
{
    auto instr = irop_to_string(_op);
    instr.resize(4, ' ');

    std::ostringstream oss;

    auto tc = with_color ? __tc : __empty_tc;

    oss << tc.C_DARK_CYAN << instr << tc.C_RESET;

    if (_op != IROperation::PHI) {
        bool first = true;
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
    } else {
        auto incoming_strs = _phi_incoming | boost::adaptors::transformed([&tc](const auto &bb_varid_pair) {
            auto fmt = boost::format("[%1% %3%@%2%%4%]")
                % std::string(Operand(std::get<0>(bb_varid_pair.second)))
                % bb_varid_pair.first->get_label()
                % tc.C_DARK_BLUE
                % tc.C_RESET;
            return fmt.str();
        });
        oss << std::string(_tgt) << ", " << boost::join(incoming_strs, ", ");
    }
    return oss.str();
}

bool operator<(const InstructionListIter &it1, const InstructionListIter &it2)
{
    return &(*it1) < &(*it2); // Compare based on the memory address of the pointed-to objects
}

CLOSE_IRGEN_NAMESPACE
// end
