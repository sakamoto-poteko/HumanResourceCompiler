#include <string>

#include <boost/format.hpp>
#include <spdlog/spdlog.h>

#include "HRMByte.h"
#include "InterpreterMemoryManager.h"
#include "InterpreterExceptions.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

InterpreterMemoryManager::InterpreterMemoryManager()
{
}

InterpreterMemoryManager::~InterpreterMemoryManager()
{
}

void InterpreterMemoryManager::set_floor(int id, HRMByte value)
{
    if (id < 0 || id > _floormax) {
        auto errstr = boost::format("floor id %1% is out of range [0, %2%]") % id % _floormax;
        throw InterpreterException(InterpreterException::ErrorType::FloorOutOfRange, errstr.str());
    }
    spdlog::trace("Set floor[{}] = {}", id, value);
    _floor[id] = value;
}

bool InterpreterMemoryManager::get_floor(int id, HRMByte &value)
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

void InterpreterMemoryManager::set_floor_max(int floormax)
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
