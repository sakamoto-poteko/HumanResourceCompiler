#ifndef INTERPRETER_ACCUMULATOR_H
#define INTERPRETER_ACCUMULATOR_H

#include <optional>

#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include "HRMByte.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

class InterpreterIOManager;
class InterpreterMemoryManager;

class InterpreterAccumulator {
public:
    InterpreterAccumulator() = default;

    ~InterpreterAccumulator() = default;

    bool is_zero();
    bool is_not_zero();
    bool is_negative();
    bool is_true();

    HRMByte get_register();

    // this func is to enforce calling with int, char or HRMByte, instead of other implicitly convertible, such as bool
    template <typename T>
        requires(std::is_same_v<T, int> || std::is_same_v<T, char> || std::is_same_v<T, HRMByte>)
    void set_register(T value)
    {
        __set_register(value);
    }

    void reset_register();

private:
    std::optional<HRMByte> _register;

    void __set_register(HRMByte value);

    void __set_register(int value) { __set_register(HRMByte(value)); }

    void __set_register(char value) { __set_register(HRMByte(value)); }

    HRMByte __get_register();
};

CLOSE_INTERPRETER_NAMESPACE

#endif