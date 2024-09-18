#include "MemoryManager.h"
#include "hrint_global.h"

OPEN_HRINT_NAMESPACE

MemoryManager::MemoryManager()
{
}

MemoryManager::~MemoryManager()
{
}

bool hrl::hrint::MemoryManager::get_variable(const hrl::semanalyzer::SymbolPtr &symbol, int &value)
{
    auto found = _variables.find(symbol);
    if (found == _variables.end()) {
        return false;
    } else {
        value = found->second;
        return true;
    }
}

void hrl::hrint::MemoryManager::set_variable(const hrl::semanalyzer::SymbolPtr &symbol, int value)
{
    _variables[symbol] = value;
}

void hrl::hrint::MemoryManager::set_floor(int id, int value)
{
    _floor[id] = value;
}

bool hrl::hrint::MemoryManager::get_floor(int id, int &value)
{
    auto found = _floor.find(id);
    if (found == _floor.end()) {
        return false;
    } else {
        value = found->second;
        return true;
    }
}

CLOSE_HRINT_NAMESPACE
// end
