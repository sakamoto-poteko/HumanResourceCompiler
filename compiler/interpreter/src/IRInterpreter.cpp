#include "IRInterpreter.h"
#include "HRMByte.h"
#include "IROps.h"
#include "IRProgramStructure.h"
#include "InterpreterExceptions.h"
#include "Operand.h"
#include "ThreeAddressCode.h"
#include "interpreter_global.h"
#include <cassert>
#include <string>

OPEN_INTERPRETER_NAMESPACE

int IRInterpreter::exec()
{
    for (const irgen::SubroutinePtr &subroutine : _program->get_subroutines()) {
        _subroutines[subroutine->get_func_name()] = subroutine;
    }

    return 0;
}

void IRInterpreter::exec_subroutine(const irgen::SubroutinePtr &subroutine, HRMByte parameter)
{
    _calling_stack.push_back({
        .subroutine_name = subroutine->get_func_name(),
        .variables = {},
        .variable_assignment_history = {},
    });

    CallFrame &call_frame = _calling_stack.back();

    const irgen::ControlFlowGraph &cfg = *subroutine->get_cfg();
    irgen::BasicBlockPtr current_block = cfg[subroutine->get_start_block()];

    std::map<std::string, irgen::BasicBlockPtr> basic_blocks;

    for (const irgen::BasicBlockPtr &basic_block : subroutine->get_basic_blocks()) {
        basic_blocks[basic_block->get_label()] = basic_block;
    }

    while (current_block) {
        for (const irgen::TACPtr &instruction : current_block->get_instructions()) {
            HRMByte op_result;
            const irgen::IROperation op = instruction->get_op();
            const irgen::Operand &tgt = instruction->get_tgt();
            const irgen::Operand &src1 = instruction->get_src1();
            const irgen::Operand &src2 = instruction->get_src2();
            bool should_set_tgt_var = false;
            bool should_branch = false;
            bool end_of_block = false;

            switch (op) {
            case irgen::IROperation::STORE:
            case irgen::IROperation::LOADI:
            case irgen::IROperation::MOV:
            case irgen::IROperation::LOAD:
                move_data(op, tgt, src1);
                // move_data sets the tgt. no need to set in this func
                break;

            case irgen::IROperation::NOT:
                op_result = HRMByte(get_variable(src1).operator bool() ? 0 : 1);
                should_set_tgt_var = true;
                break;

            case irgen::IROperation::NEG:
                op_result = HRMByte(-get_variable(src1));
                should_set_tgt_var = true;
                break;

            // binary
            case irgen::IROperation::ADD:
            case irgen::IROperation::SUB:
            case irgen::IROperation::MUL:
            case irgen::IROperation::DIV:
            case irgen::IROperation::MOD:
            case irgen::IROperation::AND:
            case irgen::IROperation::OR:
            case irgen::IROperation::EQ:
            case irgen::IROperation::NE:
            case irgen::IROperation::LT:
            case irgen::IROperation::LE:
            case irgen::IROperation::GT:
            case irgen::IROperation::GE:
                op_result = evaluate_binary_op_instructions(op, src1, src2);
                should_set_tgt_var = true;
                break;

            case irgen::IROperation::JE:
                op_result = evaluate_binary_op_instructions(irgen::IROperation::EQ, src1, src2);
                should_branch = op_result.operator bool();
                break;

            case irgen::IROperation::JNE:
                op_result = evaluate_binary_op_instructions(irgen::IROperation::NE, src1, src2);
                should_branch = op_result.operator bool();
                break;

            case irgen::IROperation::JGT:
                op_result = evaluate_binary_op_instructions(irgen::IROperation::GT, src1, src2);
                should_branch = op_result.operator bool();
                break;

            case irgen::IROperation::JLT:
                op_result = evaluate_binary_op_instructions(irgen::IROperation::LT, src1, src2);
                should_branch = op_result.operator bool();
                break;

            case irgen::IROperation::JGE:
                op_result = evaluate_binary_op_instructions(irgen::IROperation::GE, src1, src2);
                should_branch = op_result.operator bool();
                break;

            case irgen::IROperation::JLE:
                op_result = evaluate_binary_op_instructions(irgen::IROperation::LE, src1, src2);
                should_branch = op_result.operator bool();
                break;

            case irgen::IROperation::JZ:
                op_result = get_variable(src1);
                should_branch = !op_result.operator bool();
                break;

            case irgen::IROperation::JNZ:
                op_result = get_variable(src1);
                should_branch = op_result.operator bool();
                break;

            case irgen::IROperation::JMP:
                should_branch = true;
                break;

            case irgen::IROperation::CALL:
                assert(src1.get_type() == irgen::Operand::OperandType::Label);
                assert(_subroutines.contains(src1.get_label()));

                if (src2.get_type() == irgen::Operand::OperandType::VariableId) {
                    exec_subroutine(_subroutines.at(src1.get_label()), get_variable(src2));
                } else {
                    exec_subroutine(_subroutines.at(src1.get_label()));
                }
                op_result = _return_value;
                should_set_tgt_var = tgt.get_type() == irgen::Operand::OperandType::VariableId;
                break;

            case irgen::IROperation::ENTER:
                op_result = parameter;
                should_set_tgt_var = true;
                break;

            case irgen::IROperation::RET:
                _return_value = src1;
                end_of_block = true;
                break;

            case irgen::IROperation::HALT:
                throw InterpreterException(InterpreterException::ErrorType::HaltRequested, "Halt is requested");
                break;

            case irgen::IROperation::INPUT:
                if (!_io_manager.pop_input(op_result)) {
                    throw InterpreterException(InterpreterException::ErrorType::EndOfInput, "End of input reached");
                } else {
                    should_set_tgt_var = true;
                }
                break;

            case irgen::IROperation::OUTPUT:
                _io_manager.push_output(get_variable(src1));
                break;

            case irgen::IROperation::PHI:
                // instruction->get_phi_incomings().find();
                // FIXME: the phi node definition is incorrect
                should_set_tgt_var = true;
                break;

            case irgen::IROperation::NOP:
                // do nothing
                break;
            }

            if (end_of_block) {
                current_block = nullptr;
                break;
            }

            if (should_set_tgt_var) {
                set_variable(tgt, op_result);
            }

            if (should_branch) {
                assert(tgt.get_type() == irgen::Operand::OperandType::Label);
                assert(basic_blocks.contains(tgt.get_label()));
                current_block = basic_blocks.at(tgt.get_label());
                assert(current_block);
                break;
            }
        }
    }

    _calling_stack.pop_back();
}

CLOSE_INTERPRETER_NAMESPACE
// end
