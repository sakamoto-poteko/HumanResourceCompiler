#ifndef INT_MEMORYMANAGER_H
#define INT_MEMORYMANAGER_H

#include "HRMByte.h"
#include "Symbol.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

class MemoryManager {
public:
    MemoryManager();
    ~MemoryManager();

    void set_variable(const hrl::semanalyzer::SymbolPtr &symbol, HRMByte value);
    bool get_variable(const hrl::semanalyzer::SymbolPtr &symbol, HRMByte &value);
    void set_floor(int id, HRMByte value);
    bool get_floor(int id, HRMByte &value);

private:
    std::map<int, HRMByte> _floor;
    std::map<hrl::semanalyzer::SymbolPtr, HRMByte> _variables;
};

CLOSE_INTERPRETER_NAMESPACE

#endif