#ifndef ERRORMESSAGE_H
#define ERRORMESSAGE_H

#include "hrl_global.h"
#include <string>

#include <boost/format.hpp>

#include <spdlog/spdlog.h>

// Severity levels
enum class ErrorSeverity {
    Error,
    Warning,
    Note
};

// Struct to represent the location of an error
struct ErrorLocation {
    std::string file_name;
    int line; // could be -1
    int column; // could be -1
    std::size_t width;

    ErrorLocation(const std::string filename, int line, int column, std::size_t width)
        : file_name(filename)
        , line(line)
        , column(column)
        , width(width)
    {
    }

    ErrorLocation(StringPtr filename, int line, int column, std::size_t width)
        : file_name(*filename)
        , line(line)
        , column(column)
        , width(width)
    {
    }

    std::string to_string() const
    {
        return (boost::format("%1%:%2%:%3%") % file_name % line % column).str();
    }
};

// Struct to represent a compiler message
struct CompilerMessage {
    int error_id; // Unique error ID
    ErrorSeverity severity; // Severity level
    ErrorLocation location; // Location information
    std::string message; // Main error message
    std::string suggestion; // Optional suggestion
    std::size_t order; // To track message order
    bool is_cont; // Extra message following previous one?

    CompilerMessage(int error_id, ErrorSeverity severity, ErrorLocation location, std::string message, std::string suggestion = "")
        : error_id(error_id)
        , severity(severity)
        , location(std::move(location))
        , message(std::move(message))
        , suggestion(std::move(suggestion))
        , is_cont(false)
    {
    }

    CompilerMessage(ErrorSeverity severity, ErrorLocation location, std::string message)
        : error_id(0)
        , severity(severity)
        , location(std::move(location))
        , message(std::move(message))
        , suggestion()
        , is_cont(true)
    {
    }
};

#endif