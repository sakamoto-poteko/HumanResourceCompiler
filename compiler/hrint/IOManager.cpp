#include "IOManager.h"
#include "hrint_global.h"

OPEN_HRINT_NAMESPACE

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
        return true;
    }
}

void IOManager::push_output(int value)
{
    _output.push(value);
}

bool IOManager::pop_output(int &value)
{
    if (_output.empty()) {
        return false;
    } else {
        value = _output.front();
        return true;
    }
}

CLOSE_HRINT_NAMESPACE

// end
