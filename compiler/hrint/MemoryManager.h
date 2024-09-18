#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "Symbol.h"
#include "hrint_global.h"

OPEN_HRINT_NAMESPACE

class MemoryManager {
public:
    MemoryManager();
    ~MemoryManager();

    void set_variable(const hrl::semanalyzer::SymbolPtr &symbol, int value);
    bool get_variable(const hrl::semanalyzer::SymbolPtr &symbol, int &value);
    void set_floor(int id, int value);
    bool get_floor(int id, int &value);

private:
    std::map<int, int> _floor;
    std::map<hrl::semanalyzer::SymbolPtr, int> _variables;
};

CLOSE_HRINT_NAMESPACE

#endif