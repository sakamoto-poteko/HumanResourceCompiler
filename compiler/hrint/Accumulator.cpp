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
    // FIXME: check return
    set_register(value);
}

void Accumulator::outbox()
{
    if (_register.has_value()) {
        _ioman.push_output(_register.value());
        _register.reset();
    } else {
        // FIXME: throw empty register error
        throw;
    }
}

void Accumulator::bumpdn(int floor_id)
{
    int val;
    bool ok = _memory.get_floor(floor_id, val);
    // FIXME: check
    --val;
    _memory.set_floor(floor_id, val);
}

void Accumulator::bumpdn(const hrl::semanalyzer::SymbolPtr &symbol)
{
    int val;
    bool ok = _memory.get_variable(symbol, val);
    // FIXME: check
    --val;
    _memory.set_variable(symbol, val);
}

void Accumulator::bumpup(int floor_id)
{
    int val;
    bool ok = _memory.get_floor(floor_id, val);
    // FIXME: check
    ++val;
    _memory.set_floor(floor_id, val);
}

void Accumulator::bumpup(const hrl::semanalyzer::SymbolPtr &symbol)
{
    int val;
    bool ok = _memory.get_variable(symbol, val);
    // FIXME: check
    ++val;
    _memory.set_variable(symbol, val);
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
    // FIXME: check
    set_register(value);
}

void Accumulator::copy_from_floor(int floor_id)
{
    int value;
    bool ok = _memory.get_floor(floor_id, value);
    // FIXME: check
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
        // FIXME:throw custom ex
        throw;
    }
}

bool hrl::hrint::Accumulator::is_true()
{
    if (_register.has_value()) {
        return _register.value() == 0 ? false : true;
    } else {
        throw InterpreterException(InterpreterException::ErrorType::RegisterIsEmpty, "register is empty");
    }
}

CLOSE_HRINT_NAMESPACE

// end
