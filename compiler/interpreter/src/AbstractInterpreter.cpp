#include "AbstractInterpreter.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

AbstractInterpreter::AbstractInterpreter(InterpreterIOManager &ioman, InterpreterMemoryManager &memman)
    : _io_manager(ioman)
    , _memory_manager(memman)
{
}

InterpreterIOManager &AbstractInterpreter::get_io_manager()
{
    return _io_manager;
}

InterpreterMemoryManager &AbstractInterpreter::get_memory_manager()
{
    return _memory_manager;
}

CLOSE_INTERPRETER_NAMESPACE
// end
