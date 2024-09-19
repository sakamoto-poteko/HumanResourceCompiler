#include "IntMemoryManager.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

MemoryManager::MemoryManager()
{
}

MemoryManager::~MemoryManager()
{
}

bool MemoryManager::get_variable(const hrl::semanalyzer::SymbolPtr &symbol, int &value)
{
    auto found = _variables.find(symbol);
    if (found == _variables.end()) {
        return false;
    } else {
        value = found->second;
        return true;
    }
}

void MemoryManager::set_variable(const hrl::semanalyzer::SymbolPtr &symbol, int value)
{
    _variables[symbol] = value;
}

void MemoryManager::set_floor(int id, int value)
{
    _floor[id] = value;
}

bool MemoryManager::get_floor(int id, int &value)
{
    auto found = _floor.find(id);
    if (found == _floor.end()) {
        return false;
    } else {
        value = found->second;
        return true;
    }
}

CLOSE_INTERPRETER_NAMESPACE
// end
