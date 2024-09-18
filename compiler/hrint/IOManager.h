#ifndef IOMANAGER_H
#define IOMANAGER_H

#include "hrint_global.h"
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

private:
    std::queue<int> _input;
    std::queue<int> _output;
};

CLOSE_HRINT_NAMESPACE

#endif