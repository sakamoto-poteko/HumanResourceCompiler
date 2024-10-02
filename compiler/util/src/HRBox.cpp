#include <string>

#include "HRBox.h"

HRBox::operator std::string() const
{
    if (_is_char) {
        return "'" + std::to_string(static_cast<char>(_value)) + "'";
    } else {
        return std::to_string(static_cast<int>(_value));
    }
}
