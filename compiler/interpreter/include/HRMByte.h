#ifndef HRMBYTE_H
#define HRMBYTE_H

#include <spdlog/fmt/ostr.h>

#include "InterpreterExceptions.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

class HRMByte {
public:
    HRMByte()
        : _value(0)
        , _is_char(false)
    {
    }

    explicit HRMByte(int value)
    {
        set_value(value);
    }

    explicit HRMByte(char value)
    {
        set_value(value);
    }

    HRMByte(const HRMByte &copy)
        : _value(copy._value)
        , _is_char(copy._is_char)
    {
    }

    ~HRMByte() = default;

    HRMByte &operator=(const HRMByte &copy)
    {
        _value = copy._value;
        _is_char = copy._is_char;
        return *this;
    }

    HRMByte &operator=(int copy)
    {
        set_value(copy);
        return *this;
    }

    HRMByte &operator=(char copy)
    {
        set_value(copy);
        return *this;
    }

    bool operator==(const HRMByte &other) const
    {
        return other._is_char == _is_char && other._value == _value;
    }

    HRMByte operator+(const HRMByte &rhs) const
    {
        ensure_same_type(rhs);
        return HRMByte(this->_value + rhs._value);
    }

    HRMByte operator-(const HRMByte &rhs) const
    {
        ensure_same_type(rhs);
        return HRMByte(this->_value - rhs._value);
    }

    HRMByte operator*(const HRMByte &rhs) const
    {
        ensure_same_type(rhs);
        return HRMByte(this->_value * rhs._value);
    }

    HRMByte operator/(const HRMByte &rhs) const
    {
        if (rhs._value == 0) {
            throw InterpreterException(InterpreterException::ErrorType::DivByZero, "Division by zero");
        }
        ensure_same_type(rhs);
        return HRMByte(this->_value / rhs._value);
    }

    HRMByte operator%(const HRMByte &rhs) const
    {
        if (rhs._value == 0) {
            throw InterpreterException(InterpreterException::ErrorType::ModByZero, "Mod by zero");
        }
        ensure_same_type(rhs);
        return HRMByte(this->_value % rhs._value);
    }

    HRMByte operator-() const
    {
        HRMByte r = *this;
        r._value = -r._value;
        return r;
    }

    HRMByte &operator++()
    {
        set_value(_value + 1);
        return *this;
    }

    HRMByte &operator--()
    {
        set_value(_value - 1);
        return *this;
    }

    operator int() { return _value; }

    operator char() { return static_cast<char>(_value); }

    operator bool() { return _value == 0 ? false : true; }

    void set_value(int value)
    {
        if (value < -999 || value > 999) {
            throw InterpreterException(InterpreterException::ErrorType::ValueOutOfRange, "value is out of range");
        }
        _is_char = false;
        _value = value;
    }

    void set_value(char value)
    {
        _is_char = true;
        _value = value;
    }

    bool is_char() const { return _is_char; }

    bool is_int() const { return !_is_char; }

private:
    int _value;
    bool _is_char;

    void ensure_same_type(const HRMByte &other) const
    {
        if (_is_char != other._is_char) {
            throw InterpreterException(InterpreterException::ErrorType::TypeMismatch, "Operation performed on int and char");
        }
    }
};

CLOSE_INTERPRETER_NAMESPACE

#ifndef SPDLOG_USE_STD_FORMAT
template <>
struct fmt::formatter<hrl::interpreter::HRMByte> : fmt::formatter<std::string> {
    auto format(hrl::interpreter::HRMByte hrmbyte, format_context &ctx) const -> decltype(ctx.out())
    {
        if (hrmbyte.is_char()) {
            return fmt::format_to(ctx.out(), "{}", static_cast<char>(hrmbyte));
        } else {
            return fmt::format_to(ctx.out(), "'{}'", static_cast<int>(hrmbyte));
        }
    }
};
#else
#error "std::format for HRMByte is not yet supported"
#endif

#endif