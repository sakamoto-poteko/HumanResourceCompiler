#include <cassert>
#include <iterator>
#include <string>

#include "HRMByte.h"
#include "IRInterpreter.h"
#include "IROps.h"
#include "IRProgramStructure.h"
#include "InterpreterExceptions.h"
#include "Operand.h"
#include "ThreeAddressCode.h"
#include "interpreter_global.h"
#include "semanalyzer_global.h"

OPEN_INTERPRETER_NAMESPACE

int IRInterpreter::exec()
{
    irgen::ProgramMetadata &metadata = _program->get_metadata();
    _memory_manager.set_floor_max(metadata.get_floor_max());
    for (auto &[floor_id, floor_init_value] : _program->get_metadata().get_floor_inits()) {
        _memory_manager.set_floor(floor_id, HRMByte(floor_init_value));
    }

    for (const irgen::SubroutinePtr &subroutine : _program->get_subroutines()) {
        _subroutines[subroutine->get_func_name()] = subroutine;
    }

    spdlog::debug("[IRIntrExec] Starting IR interpretation...");
    irgen::SubroutinePtr entry_point = _subroutines.at(semanalyzer::GLOBAL_SCOPE_ID);
    exec_subroutine(entry_point);
    spdlog::debug("[IRIntrExec] IR reached the end");

    return 0;
}

void IRInterpreter::exec_subroutine(const irgen::SubroutinePtr &subroutine, HRMByte parameter)
{
    spdlog::debug("[IRIntrExecSubroutine] Entered subroutine '{}'", subroutine->get_func_name());

    _calling_stack.push_back({
        .subroutine_name = subroutine->get_func_name(),
        .variables = {},
        .basic_block_visited = {},
        .current_basic_block = nullptr,
    });

    CallFrame &call_frame = _calling_stack.back();
    irgen::BasicBlockPtr &current_block = call_frame.current_basic_block;

    const irgen::BBGraph &cfg = *subroutine->get_cfg();
    current_block = cfg[subroutine->get_cfg_entry()];
    irgen::BasicBlockPtr predecessor_block = nullptr;

    std::map<std::string, irgen::BasicBlockPtr> basic_blocks;
    // next bb in linear order
    std::map<irgen::BasicBlockPtr, irgen::BasicBlockPtr> next_basic_block;

    const auto &subroutine_bb = subroutine->get_basic_blocks();
    for (auto bb_it = subroutine->get_basic_blocks().begin(); bb_it != subroutine_bb.end(); ++bb_it) {
        const irgen::BasicBlockPtr &basic_block = *bb_it;
        basic_blocks[basic_block->get_label()] = basic_block;

        auto next = std::next(bb_it);
        if (next != subroutine_bb.end()) {
            next_basic_block[basic_block] = *next;
        } else {
            next_basic_block[basic_block] = nullptr;
        }
    }

    auto set_next_block_to_exec = [&predecessor_block, &current_block, &call_frame](const irgen::BasicBlockPtr &basic_block) {
        predecessor_block = current_block;
        call_frame.basic_block_visited.insert(current_block->get_label());
        current_block = basic_block;
    };

    while (current_block) {
        spdlog::debug("[IRIntrExecBB] Entered basic block '{}'", current_block->get_label());
        bool non_linear_control_flow = false;

        for (const irgen::TACPtr &instruction : current_block->get_instructions()) {
            spdlog::debug("[IRIntrExecInstr] Executing {}", instruction->to_string(true));

            HRMByte op_result;
            const irgen::IROperation op = instruction->get_op();
            const irgen::Operand &tgt = instruction->get_tgt();
            const irgen::Operand &src1 = instruction->get_src1();
            const irgen::Operand &src2 = instruction->get_src2();
            bool should_set_tgt_var = false;
            bool should_branch = false;
            bool is_return = false;

            switch (op) {
            case irgen::IROperation::STORE:
            case irgen::IROperation::LOADI:
            case irgen::IROperation::MOV:
            case irgen::IROperation::LOAD:
                move_data(op, tgt, src1, src2);
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
                // return may be empty or has value
                if (src1.get_type() == irgen::Operand::OperandType::VariableId) {
                    _return_value = get_variable(src1);
                }
                is_return = true;
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
                spdlog::debug("[IRIntrExecInstr] Phi: predecessor block is '{}'", predecessor_block->get_label());
                assert(instruction->get_phi_incomings().contains(predecessor_block));
                op_result = get_variable(irgen::Operand(std::get<0>(instruction->get_phi_incomings().at(predecessor_block))));
                should_set_tgt_var = true;
                break;

            case irgen::IROperation::NOP:
                // do nothing
                break;
            }

            if (is_return) {
                set_next_block_to_exec(nullptr);
                non_linear_control_flow = true;

                break;
            }

            if (should_set_tgt_var) {
                set_variable(tgt, op_result);
            }

            if (should_branch) {
                assert(tgt.get_type() == irgen::Operand::OperandType::Label);
                assert(basic_blocks.contains(tgt.get_label()));
                set_next_block_to_exec(basic_blocks.at(tgt.get_label()));
                non_linear_control_flow = true;
                break;
            }
        } // end of for loop: every instruction

        // all instructions are visited
        // if it's jmp, it's not natural end
        if (current_block && !non_linear_control_flow) {
            set_next_block_to_exec(next_basic_block.at(current_block));
        }
    }

    _calling_stack.pop_back();
}

HRMByte IRInterpreter::get_variable(const irgen::Operand &variable)
{
    if (variable.get_type() != irgen::Operand::OperandType::VariableId) {
        spdlog::error("Tring to get variable for operand {} but it's not a variable id. This is likely a bug, consider report it. {}", std::string(variable), __PRETTY_FUNCTION__);
        throw;
    }

    int reg_id = variable.get_register_id();
    const auto &var_map = reg_id < 0 ? _global_variables : _calling_stack.back().variables;

    auto var_it = var_map.find(reg_id);
    if (var_it == var_map.end()) {
        spdlog::error("The variable {} is accessed before assigned. This is likely a bug, consider report it. {}", std::string(variable), __PRETTY_FUNCTION__);
        throw;
    } else {
        spdlog::trace("Get variable {}: {}", std::string(variable), var_it->second);
        return var_it->second;
    }
}

void IRInterpreter::set_variable(const irgen::Operand &variable, const HRMByte &value)
{
    if (variable.get_type() != irgen::Operand::OperandType::VariableId) {
        spdlog::error("Tring to set variable for operand {} but it's not a variable id. This is likely a bug, consider report it. {}", std::string(variable), __PRETTY_FUNCTION__);
        throw;
    }
    CallFrame &call_frame = _calling_stack.back();

    int reg_id = variable.get_register_id();
    auto &var_map = reg_id < 0 ? _global_variables : call_frame.variables;

    auto var_it = var_map.find(reg_id);
    // only check if this block haven't be visited.
    if (_enforce_ssa && var_it != var_map.end() && !call_frame.basic_block_visited.contains(call_frame.current_basic_block->get_label())) {
        spdlog::error("Tring to set variable {} which is already set, which violates SSA rule. This is likely a bug, consider report it. {}", std::string(variable), __PRETTY_FUNCTION__);
        throw;
    } else {
        spdlog::trace("Set variable {} = {}", std::string(variable), value);
        var_map[reg_id] = value;
    }
}

HRMByte IRInterpreter::evaluate_binary_op_instructions(irgen::IROperation op, const irgen::Operand &src1, const irgen::Operand &src2)
{
    HRMByte o1 = get_variable(src1);
    HRMByte o2 = get_variable(src2);
    HRMByte result;

    switch (op) {
    case irgen::IROperation::ADD:
        return o1 + o2;
    case irgen::IROperation::SUB:
        return o1 - o2;
    case irgen::IROperation::MUL:
        return o1 * o2;
    case irgen::IROperation::DIV:
        return o1 / o2;
    case irgen::IROperation::MOD:
        return o1 % o2;
    case irgen::IROperation::AND:
        return HRMByte(static_cast<bool>(o1) && static_cast<bool>(o2) ? 1 : 0);
    case irgen::IROperation::OR:
        return HRMByte(static_cast<bool>(o1) || static_cast<bool>(o2) ? 1 : 0);
    case irgen::IROperation::EQ:
        return HRMByte(o1 == o2 ? 1 : 0);
    case irgen::IROperation::NE:
        return HRMByte(o1 != o2 ? 1 : 0);
    case irgen::IROperation::LT:
        return HRMByte(static_cast<int>(o1) < static_cast<int>(o2) ? 1 : 0);
    case irgen::IROperation::LE:
        return HRMByte(static_cast<int>(o1) <= static_cast<int>(o2) ? 1 : 0);
    case irgen::IROperation::GT:
        return HRMByte(static_cast<int>(o1) > static_cast<int>(o2) ? 1 : 0);
    case irgen::IROperation::GE:
        return HRMByte(static_cast<int>(o1) >= static_cast<int>(o2) ? 1 : 0);
    default:
        break;
    }
    spdlog::error("'{}' is not a binary op but called to evalute as binary expression. {}", irgen::irop_to_string(op), __PRETTY_FUNCTION__);
    throw;
}

HRMByte IRInterpreter::evaluate_unary_op_instructions(irgen::IROperation op, const irgen::Operand &src1)
{
    HRMByte o1 = get_variable(src1);

    switch (op) {
    case irgen::IROperation::NEG:
        return -o1;
    case irgen::IROperation::NOT:
        return HRMByte(o1.operator bool() ? 0 : 1);
    default:
        break;
    }
    spdlog::error("'{}' is not a unary op but called to evalute as unary expression. {}", irgen::irop_to_string(op), __PRETTY_FUNCTION__);
    throw;
}

void IRInterpreter::move_data(irgen::IROperation op, const irgen::Operand &tgt, const irgen::Operand &src1, const irgen::Operand &src2)
{
    // bool ok = false;
    HRMByte value;
    int floor_id = 0;

    switch (op) {
    case irgen::IROperation::MOV:
        set_variable(tgt, get_variable(src1));
        return;
    case irgen::IROperation::LOAD:
        // either load from global or floor
        if (src1.get_type() == irgen::Operand::OperandType::ImmediateValue) {
            floor_id = src1.get_constant().operator int();
        } else {
            // global, or indirect addressing
            if (src1.get_type() == irgen::Operand::OperandType::VariableId && src1.get_register_id() < 0) {
                floor_id = src1.get_register_id();
            } else {
                floor_id = get_variable(src1).operator int();
            }
        }

        if (floor_id < 0) {
            auto glb_it = _global_variables.find(floor_id);
            if (glb_it == _global_variables.end()) {
                spdlog::error("Global variable %{} is used before assignment. {}", floor_id, __PRETTY_FUNCTION__);
            } else {
                value = glb_it->second;
            }
        } else {
            if (!_memory_manager.get_floor(floor_id, value)) {
                throw InterpreterException(InterpreterException::ErrorType::FloorIsEmpty, "Floor is null");
            }
        }
        set_variable(tgt, value);
        return;
    case irgen::IROperation::STORE:
        // NOTE: STORE op has src1 and src2 and no tgt. src1 is the target address, src2 is the value
        // either store to global or floor
        if (src2.get_type() == irgen::Operand::OperandType::ImmediateValue) {
            floor_id = src2.get_constant().operator int();
        } else {
            // global, or indirect addressing
            if (src1.get_type() == irgen::Operand::OperandType::VariableId && src1.get_register_id() < 0) {
                floor_id = src1.get_register_id();
            } else {
                // get_variable will check if it's a variable id and raise if not
                // no need to make another if here. ugly
                floor_id = get_variable(src1).operator int();
            }
        }

        if (floor_id < 0) {
            _global_variables[floor_id] = get_variable(src2);
        } else {
            _memory_manager.set_floor(floor_id, get_variable(src2));
        }
        return;
    case irgen::IROperation::LOADI:
        if (src1.get_type() != irgen::Operand::OperandType::ImmediateValue) {
            spdlog::error("Trying to LOADI of an operand which is not an immediate value: {}. {}", std::string(src1), __PRETTY_FUNCTION__);
            throw;
        }
        set_variable(tgt, HRMByte(src1.get_constant()));
        return;
    default:
        break;
    }
    spdlog::error("'{}' is not a unary op but called to evalute as unary expression. {}", irgen::irop_to_string(op), __PRETTY_FUNCTION__);
    throw;
}

CLOSE_INTERPRETER_NAMESPACE
// end
