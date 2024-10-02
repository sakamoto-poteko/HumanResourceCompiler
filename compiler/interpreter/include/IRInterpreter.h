#ifndef IRINTERPRETER_H
#define IRINTERPRETER_H

#include <list>
#include <string>

#include "AbstractInterpreter.h"
#include "HRMByte.h"
#include "IROps.h"
#include "IRProgramStructure.h"
#include "InsertionOrderedSet.h"
#include "Operand.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

class IRInterpreter : public AbstractInterpreter {
public:
    IRInterpreter(IOManager &ioman, MemoryManager &memman, const irgen::ProgramPtr &program, bool enforce_ssa = true)
        : AbstractInterpreter(ioman, memman)
        , _program(program)
        , _enforce_ssa(enforce_ssa)
    {
    }

    ~IRInterpreter() = default;

    int exec() override;

private:
    struct CallFrame {
        std::string subroutine_name;
        std::map<int, HRMByte> variables;
        // this is used to verify SSA. the variable may be defined multiples the block is revisited
        InsertionOrderedSet<std::string> basic_block_visited;
        // the current basic block executing
        irgen::BasicBlockPtr current_basic_block;
    };

    irgen::ProgramPtr _program;
    std::map<int, HRMByte> _global_variables;
    std::map<std::string, irgen::SubroutinePtr> _subroutines;
    bool _enforce_ssa;

    std::list<CallFrame> _calling_stack;
    HRMByte _return_value;

    void exec_subroutine(const irgen::SubroutinePtr &subroutine, HRMByte parameter = HRMByte());
    HRMByte get_variable(const irgen::Operand &variable);
    void set_variable(const irgen::Operand &variable, const HRMByte &value);

    HRMByte evaluate_binary_op_instructions(irgen::IROperation op, const irgen::Operand &src1, const irgen::Operand &src2);
    HRMByte evaluate_unary_op_instructions(irgen::IROperation op, const irgen::Operand &src1);
    void move_data(irgen::IROperation op, const irgen::Operand &tgt, const irgen::Operand &src1, const irgen::Operand &src2);
};

CLOSE_INTERPRETER_NAMESPACE

#endif