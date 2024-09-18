#ifndef IOMANAGER_H
#define IOMANAGER_H

#include "hrint_global.h"
#include <functional>
#include <queue>

OPEN_HRINT_NAMESPACE

class IOManager {
public:
    IOManager();
    ~IOManager();

    void push_input(int value);
    bool pop_input(int &value);
    void push_output(int value);
    bool pop_output(int &value);

    void set_on_input_popped(std::function<void(int)> hook);
    void set_on_output_pushed(std::function<void(int)> hook);

private:
    std::queue<int> _input;
    std::queue<int> _output;
    std::function<void(int)> _on_input_popped;
    std::function<void(int)> _on_output_pushed;
};

CLOSE_HRINT_NAMESPACE

#endif