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
    void set_floor_max(int floormax);

private:
    std::map<int, HRMByte> _floor;
    std::map<hrl::semanalyzer::SymbolPtr, HRMByte> _variables;
    int _floormax = 63;
};

CLOSE_INTERPRETER_NAMESPACE

#endif