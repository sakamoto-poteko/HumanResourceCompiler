#ifndef IRINTERPRETER_H
#define IRINTERPRETER_H

#include <list>
#include <string>
#include <vector>

#include "AbstractInterpreter.h"
#include "HRMByte.h"
#include "IROps.h"
#include "IRProgramStructure.h"
#include "Operand.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

class IRInterpreter : public AbstractInterpreter {
public:
    IRInterpreter(IOManager &ioman, MemoryManager &memman, const irgen::ProgramPtr &program)
        : AbstractInterpreter(ioman, memman)
    {
    }

    ~IRInterpreter() = default;

    int exec() override;

private:
    struct CallFrame {
        std::string subroutine_name;
        std::map<unsigned int, HRMByte> variables;
        std::map<unsigned int, std::string> variable_assignment_history;
    };

    HRMByte _return_value;

    irgen::ProgramPtr _program;
    std::map<unsigned int, HRMByte> _global_variables;
    std::map<std::string, irgen::SubroutinePtr> _subroutines;

    std::list<CallFrame> _calling_stack;

    void exec_subroutine(const irgen::SubroutinePtr &subroutine, HRMByte parameter = HRMByte());
    HRMByte get_variable(const irgen::Operand &variable);
    void set_variable(const irgen::Operand &variable, const HRMByte &value);

    HRMByte evaluate_binary_op_instructions(irgen::IROperation op, const irgen::Operand &src1, const irgen::Operand &src2);
    HRMByte evaluate_unary_op_instructions(irgen::IROperation op, const irgen::Operand &src1);
    HRMByte move_data(irgen::IROperation op, const irgen::Operand &tgt, const irgen::Operand &src1);
};

CLOSE_INTERPRETER_NAMESPACE

#endif