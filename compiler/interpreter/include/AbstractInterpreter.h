#ifndef ABSTRACTINTERPRETER_H
#define ABSTRACTINTERPRETER_H

#include "IntAccumulator.h"
#include "IntIOManager.h"
#include "IntMemoryManager.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

class AbstractInterpreter {
public:
    virtual ~AbstractInterpreter() = default;
    virtual int exec() = 0;

protected:
    AbstractInterpreter(IOManager &ioman, MemoryManager &memman);
    IOManager &get_io_manager();
    MemoryManager &get_memory_manager();

    IOManager &_io_manager;
    MemoryManager &_memory_manager;
};

CLOSE_INTERPRETER_NAMESPACE

#endif