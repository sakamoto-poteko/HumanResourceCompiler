#include <spdlog/spdlog.h>

#include "IntAccumulator.h"
#include "IntIOManager.h"
#include "IntMemoryManager.h"
#include "InterpreterExceptions.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

Accumulator::~Accumulator()
{
}

void Accumulator::inbox()
{
    int value;
    bool ok = _ioman.pop_input(value);
    if (!ok) {
        throw InterpreterException(InterpreterException::ErrorType::EndOfInput, "End of input reached");
    }
    set_register(value);
}

void Accumulator::outbox()
{
    if (_register.has_value()) {
        _ioman.push_output(_register.value());
        _register.reset();
    } else {
        throw InterpreterException(InterpreterException::ErrorType::RegisterIsEmpty, "Register is empty");
    }
}

void Accumulator::__arithmetic_operation(ArithmeticOperator op, int target)
{
    switch (op) {
    case ArithmeticOperator::Add:
        target += get_register();
        break;
    case ArithmeticOperator::Sub:
        target = get_register() - target;
        break;
    default:
        spdlog::critical("Unknown arithmetic operator {}. {}", static_cast<int>(op), __PRETTY_FUNCTION__);
        throw;
    }
    set_register(target);
}

void Accumulator::arithmetic_operation(ArithmeticOperator op, const hrl::semanalyzer::SymbolPtr &symbol)
{
    int val;
    _memory.get_variable(symbol, val);
    __arithmetic_operation(op, val);
}

void Accumulator::arithmetic_operation(ArithmeticOperator op, unsigned int floor_id)
{
    int val;
    _memory.get_floor(floor_id, val);
    __arithmetic_operation(op, val);
}

void Accumulator::bumpdn(unsigned int floor_id)
{
    int val;
    bool ok = _memory.get_floor(floor_id, val);
    if (!ok) {
        throw InterpreterException(InterpreterException::ErrorType::FloorIsEmpty, "Floor is null");
    }
    --val;
    ensure_range(val);
    _memory.set_floor(floor_id, val);
    set_register(val);
}

void Accumulator::bumpdn(const hrl::semanalyzer::SymbolPtr &symbol)
{
    int val;
    bool ok = _memory.get_variable(symbol, val);
    if (!ok) {
        throw InterpreterException(InterpreterException::ErrorType::FloorIsEmpty, "Variable is null");
    }
    --val;
    ensure_range(val);
    _memory.set_variable(symbol, val);
    set_register(val);
}

void Accumulator::bumpup(unsigned int floor_id)
{
    int val;
    bool ok = _memory.get_floor(floor_id, val);
    if (!ok) {
        throw InterpreterException(InterpreterException::ErrorType::FloorIsEmpty, "Floor is null");
    }
    ++val;
    ensure_range(val);
    _memory.set_floor(floor_id, val);
    set_register(val);
}

void Accumulator::bumpup(const hrl::semanalyzer::SymbolPtr &symbol)
{
    int val;
    bool ok = _memory.get_variable(symbol, val);
    if (!ok) {
        throw InterpreterException(InterpreterException::ErrorType::FloorIsEmpty, "Variable is null");
    }
    ++val;
    ensure_range(val);
    _memory.set_variable(symbol, val);
    set_register(val);
}

void Accumulator::copy_to(const hrl::semanalyzer::SymbolPtr &symbol)
{
    _memory.set_variable(symbol, get_register());
}

void Accumulator::copy_to_floor(unsigned int floor_id)
{
    _memory.set_floor(floor_id, get_register());
}

void Accumulator::copy_from(const hrl::semanalyzer::SymbolPtr &symbol)
{
    int value;
    bool ok = _memory.get_variable(symbol, value);
    if (!ok) {
        throw InterpreterException(InterpreterException::ErrorType::FloorIsEmpty, "Variable is null");
    }
    set_register(value);
}

void Accumulator::copy_from_floor(unsigned int floor_id)
{
    int value;
    bool ok = _memory.get_floor(floor_id, value);
    if (!ok) {
        throw InterpreterException(InterpreterException::ErrorType::FloorIsEmpty, "Floor is null");
    }
    set_register(value);
}

void Accumulator::__set_register(int value)
{
    ensure_range(value);
    _register = value;
}

int Accumulator::get_register()
{
    return __get_register();
}

bool Accumulator::is_zero()
{
    return __get_register() == 0;
}

bool Accumulator::is_not_zero()
{
    return __get_register() != 0;
}

bool Accumulator::is_negative()
{
    return __get_register() < 0;
}

bool Accumulator::is_true()
{
    return __get_register() == 0 ? false : true;
}

void Accumulator::ensure_range(int value)
{
    if (value < -999 || value > 999) {
        throw InterpreterException(InterpreterException::ErrorType::ValueOutOfRange, "value is out of range");
    }
}

int Accumulator::__get_register()
{
    if (_register.has_value()) {
        return _register.value();
    } else {
        throw InterpreterException(InterpreterException::ErrorType::RegisterIsEmpty, "Register is empty");
    }
}

CLOSE_INTERPRETER_NAMESPACE

// end
