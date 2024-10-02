#include <string>

#include <boost/format.hpp>
#include <spdlog/spdlog.h>

#include "HRMByte.h"
#include "IntMemoryManager.h"
#include "InterpreterExceptions.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

MemoryManager::MemoryManager()
{
}

MemoryManager::~MemoryManager()
{
}

bool MemoryManager::get_variable(const hrl::semanalyzer::SymbolPtr &symbol, HRMByte &value)
{
    auto found = _variables.find(symbol);
    if (found == _variables.end()) {
        spdlog::trace("Variable '{}' does not exist", symbol->name);
        return false;
    } else {
        value = found->second;
        spdlog::trace("Accessed variable '{}': {}", symbol->name, value);
        return true;
    }
}

void MemoryManager::set_variable(const hrl::semanalyzer::SymbolPtr &symbol, HRMByte value)
{
    spdlog::trace("Set variable '{}' = {}", symbol->name, value);
    _variables[symbol] = value;
}

void MemoryManager::set_floor(int id, HRMByte value)
{
    if (id < 0 || id > _floormax) {
        auto errstr = boost::format("floor id %1% is out of range [0, %2%]") % id % _floormax;
        throw InterpreterException(InterpreterException::ErrorType::FloorOutOfRange, errstr.str());
    }
    spdlog::trace("Set floor[{}] = {}", id, value);
    _floor[id] = value;
}

bool MemoryManager::get_floor(int id, HRMByte &value)
{
    if (id < 0 || id > _floormax) {
        auto errstr = boost::format("floor id %1% is out of range [0, %2%]") % id % _floormax;
        throw InterpreterException(InterpreterException::ErrorType::FloorOutOfRange, errstr.str());
    }

    auto found = _floor.find(id);
    if (found == _floor.end()) {
        spdlog::trace("floor[{}] does not exist", id);
        return false;
    } else {
        value = found->second;
        spdlog::trace("Accessed floor[{}]: {}", id, value);
        return true;
    }
}

void MemoryManager::set_floor_max(int floormax)
{
    if (floormax < 0) {
        throw InterpreterException(InterpreterException::ErrorType::FloorOutOfRange, "floor max cannot be negative");
    }

    _floormax = floormax;
    auto it = _floor.begin();
    while (it != _floor.end()) {
        if (it->first > floormax) {
            it = _floor.erase(it);
        } else {
            ++it;
        }
    }
}

CLOSE_INTERPRETER_NAMESPACE
// end
