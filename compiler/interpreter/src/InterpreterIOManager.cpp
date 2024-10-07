#include "InterpreterIOManager.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

InterpreterIOManager::InterpreterIOManager()
{
}

InterpreterIOManager::~InterpreterIOManager()
{
}

void InterpreterIOManager::push_input(HRMByte value)
{
    _input.push(value);
}

bool InterpreterIOManager::pop_input(HRMByte &value)
{
    if (_input.empty()) {
        return false;
    } else {
        value = _input.front();
        _input.pop();
        if (_on_input_popped) {
            _on_input_popped(value);
        }
        return true;
    }
}

void InterpreterIOManager::push_output(HRMByte value)
{
    _output.push(value);
    if (_on_output_pushed) {
        _on_output_pushed(value);
    }
}

bool InterpreterIOManager::pop_output(HRMByte &value)
{
    if (_output.empty()) {
        return false;
    } else {
        value = _output.front();
        _output.pop();
        return true;
    }
}

void InterpreterIOManager::set_on_input_popped(std::function<void(HRMByte)> hook)
{
    _on_input_popped = hook;
}

void InterpreterIOManager::set_on_output_pushed(std::function<void(HRMByte)> hook)
{
    _on_output_pushed = hook;
}

CLOSE_INTERPRETER_NAMESPACE

// end
