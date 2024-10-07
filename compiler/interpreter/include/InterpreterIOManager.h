#ifndef INTERPRETER_IOMANAGER_H
#define INTERPRETER_IOMANAGER_H

#include <functional>
#include <queue>

#include "HRMByte.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

class InterpreterIOManager {
public:
    InterpreterIOManager();
    ~InterpreterIOManager();

    void push_input(HRMByte value);
    bool pop_input(HRMByte &value);
    void push_output(HRMByte value);
    bool pop_output(HRMByte &value);

    void set_on_input_popped(std::function<void(HRMByte)> hook);
    void set_on_output_pushed(std::function<void(HRMByte)> hook);

private:
    std::queue<HRMByte> _input;
    std::queue<HRMByte> _output;
    std::function<void(HRMByte)> _on_input_popped;
    std::function<void(HRMByte)> _on_output_pushed;
};

CLOSE_INTERPRETER_NAMESPACE

#endif