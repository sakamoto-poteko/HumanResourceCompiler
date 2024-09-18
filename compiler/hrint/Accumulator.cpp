#include "Accumulator.h"
#include "IOManager.h"
#include "InterpreterExceptions.h"
#include "MemoryManager.h"
#include "hrint_global.h"

OPEN_HRINT_NAMESPACE

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

void Accumulator::bumpdn(int floor_id)
{
    int val;
    bool ok = _memory.get_floor(floor_id, val);
    if (!ok) {
        throw InterpreterException(InterpreterException::ErrorType::FloorIsEmpty, "Floor is null");
    }
    --val;
    _memory.set_floor(floor_id, val);
    _register = val;
}

void Accumulator::bumpdn(const hrl::semanalyzer::SymbolPtr &symbol)
{
    int val;
    bool ok = _memory.get_variable(symbol, val);
    if (!ok) {
        throw InterpreterException(InterpreterException::ErrorType::FloorIsEmpty, "Variable is null");
    }
    --val;
    _memory.set_variable(symbol, val);
    _register = val;
}

void Accumulator::bumpup(int floor_id)
{
    int val;
    bool ok = _memory.get_floor(floor_id, val);
    if (!ok) {
        throw InterpreterException(InterpreterException::ErrorType::FloorIsEmpty, "Floor is null");
    }
    ++val;
    _memory.set_floor(floor_id, val);
    _register = val;
}

void Accumulator::bumpup(const hrl::semanalyzer::SymbolPtr &symbol)
{
    int val;
    bool ok = _memory.get_variable(symbol, val);
    if (!ok) {
        throw InterpreterException(InterpreterException::ErrorType::FloorIsEmpty, "Variable is null");
    }
    ++val;
    _memory.set_variable(symbol, val);
    _register = val;
}

void Accumulator::copy_to(const hrl::semanalyzer::SymbolPtr &symbol)
{
    _memory.set_variable(symbol, get_register());
}

void Accumulator::copy_to_floor(int floor_id)
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

void Accumulator::copy_from_floor(int floor_id)
{
    int value;
    bool ok = _memory.get_floor(floor_id, value);
    if (!ok) {
        throw InterpreterException(InterpreterException::ErrorType::FloorIsEmpty, "Floor is null");
    }
    set_register(value);
}

void hrl::hrint::Accumulator::__set_register(int value)
{
    _register = value;
}

int hrl::hrint::Accumulator::get_register()
{
    if (_register.has_value()) {
        return _register.value();
    } else {
        throw InterpreterException(InterpreterException::ErrorType::RegisterIsEmpty, "Register is empty");
    }
}

bool hrl::hrint::Accumulator::is_true()
{
    if (_register.has_value()) {
        return _register.value() == 0 ? false : true;
    } else {
        throw InterpreterException(InterpreterException::ErrorType::RegisterIsEmpty, "Register is empty");
    }
}

CLOSE_HRINT_NAMESPACE

// end
