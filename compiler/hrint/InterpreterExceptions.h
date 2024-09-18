#ifndef INTERPRETEREXCEPTIONS_H
#define INTERPRETEREXCEPTIONS_H

#include <exception>
#include <string>

#include "hrint_global.h"

OPEN_HRINT_NAMESPACE

class InterpreterException : public std::exception {
public:
    enum class ErrorType {
        ValueIsZero,
        RegisterIsEmpty,
        EndOfInput,
        FloorIsEmpty,
    };

    InterpreterException(ErrorType err, const std::string &message = "")
        : _err(err)
        , _msg("Interpreter Exception: " + message)
    {
    }

    const char *what() const noexcept override
    {
        return _msg.c_str();
    }

    ErrorType get_error_type() { return _err; }

private:
    ErrorType _err;
    std::string _msg;
};

CLOSE_HRINT_NAMESPACE

#endif