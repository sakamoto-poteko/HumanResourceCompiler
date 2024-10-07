#ifndef ABSTRACTINTERPRETER_H
#define ABSTRACTINTERPRETER_H

#include "InterpreterAccumulator.h"
#include "InterpreterIOManager.h"
#include "InterpreterMemoryManager.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

class AbstractInterpreter {
public:
    virtual ~AbstractInterpreter() = default;
    virtual int exec() = 0;

protected:
    AbstractInterpreter(InterpreterIOManager &ioman, InterpreterMemoryManager &memman);
    InterpreterIOManager &get_io_manager();
    InterpreterMemoryManager &get_memory_manager();

    InterpreterIOManager &_io_manager;
    InterpreterMemoryManager &_memory_manager;
};

CLOSE_INTERPRETER_NAMESPACE

#endif