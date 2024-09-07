#ifndef ERRORMESSAGE_H
#define ERRORMESSAGE_H

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
};

#endif