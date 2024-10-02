#ifndef HRBOX_H
#define HRBOX_H

#include <string>

class HRBox {
public:
    // Explicit constructor from int
    explicit HRBox(int value)
        : _value(value)
        , _is_char(false)
    {
    }

    // Explicit constructor from char
    explicit HRBox(char value)
        : _value(static_cast<int>(value))
        , _is_char(true)
    {
    }

    // Copy constructor
    HRBox(const HRBox &other)
        : _value(other._value)
        , _is_char(other._is_char)
    {
    }

    // Move constructor
    HRBox(HRBox &&other) noexcept
        : _value(other._value)
        , _is_char(other._is_char)
    {
        other._value = 0;
        other._is_char = false;
    }

    // Copy assignment operator
    HRBox &operator=(const HRBox &other)
    {
        if (this != &other) {
            _value = other._value;
            _is_char = other._is_char;
        }
        return *this;
    }

    // Move assignment operator
    HRBox &operator=(HRBox &&other) noexcept
    {
        if (this != &other) {
            _value = other._value;
            _is_char = other._is_char;
            // Optionally reset the moved-from object
            other._value = 0;
            other._is_char = false;
        }
        return *this;
    }

    // Destructor
    ~HRBox() = default;

    // Member functions
    bool is_int() const { return !_is_char; }

    bool is_char() const { return _is_char; }

    // Conversion operators
    explicit operator int() const { return _value; }

    explicit operator char() const { return static_cast<char>(_value); }

    operator std::string() const;

private:
    int _value;
    bool _is_char;
};

#endif