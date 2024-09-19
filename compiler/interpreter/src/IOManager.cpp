#include "IntIOManager.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

IOManager::IOManager()
{
}

IOManager::~IOManager()
{
}

void IOManager::push_input(int value)
{
    _input.push(value);
}

bool IOManager::pop_input(int &value)
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

void IOManager::push_output(int value)
{
    _output.push(value);
    if (_on_output_pushed) {
        _on_output_pushed(value);
    }
}

bool IOManager::pop_output(int &value)
{
    if (_output.empty()) {
        return false;
    } else {
        value = _output.front();
        _output.pop();
        return true;
    }
}

void IOManager::set_on_input_popped(std::function<void(int)> hook)
{
    _on_input_popped = hook;
}

void IOManager::set_on_output_pushed(std::function<void(int)> hook)
{
    _on_output_pushed = hook;
}

CLOSE_INTERPRETER_NAMESPACE

// end
