#include <spdlog/spdlog.h>

#include "InterpreterAccumulator.h"
#include "InterpreterExceptions.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

void InterpreterAccumulator::__set_register(HRMByte value)
{
    _register = value;
}

HRMByte InterpreterAccumulator::get_register()
{
    return __get_register();
}

bool InterpreterAccumulator::is_zero()
{
    return __get_register().operator int() == 0;
}

bool InterpreterAccumulator::is_not_zero()
{
    return __get_register().operator int() != 0;
}

bool InterpreterAccumulator::is_negative()
{
    return __get_register().operator int() < 0;
}

bool InterpreterAccumulator::is_true()
{
    return __get_register().operator bool();
}

HRMByte InterpreterAccumulator::__get_register()
{
    if (_register.has_value()) {
        return _register.value();
    } else {
        throw InterpreterException(InterpreterException::ErrorType::RegisterIsEmpty, "Register is empty");
    }
}

void InterpreterAccumulator::reset_register()
{
    _register.reset();
}

CLOSE_INTERPRETER_NAMESPACE
// end
