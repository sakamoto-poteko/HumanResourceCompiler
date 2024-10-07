#ifndef INTERPRETER_MEMORYMANAGER_H
#define INTERPRETER_MEMORYMANAGER_H

#include <map>

#include "HRMByte.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

class InterpreterMemoryManager {
public:
    InterpreterMemoryManager();
    ~InterpreterMemoryManager();

    void set_floor(int id, HRMByte value);
    bool get_floor(int id, HRMByte &value);
    void set_floor_max(int floormax);

private:
    std::map<int, HRMByte> _floor;
    int _floormax = 63;
};

CLOSE_INTERPRETER_NAMESPACE

#endif