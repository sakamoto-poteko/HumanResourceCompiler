#ifndef ERRORMANAGER_H
#define ERRORMANAGER_H

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include <boost/format.hpp>

#include <spdlog/spdlog.h>

#include "TerminalColor.h"

// Severity levels
enum class ErrorSeverity {
    Error,
    Warning,
    Note
};

// Struct to represent the location of an error
struct ErrorLocation {
    std::string file_name;
    int line;
    int column;
    int width;

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

// ErrorManager class
class ErrorManager {
public:
    ErrorManager(const ErrorManager &) = delete;
    ErrorManager &operator=(const ErrorManager &) = delete;

    static ErrorManager &instance();

    /**
     * @brief Add the file content to the ErrorManager, so the error message is able to print the inline text
     *
     * @param filename
     * @param lines
     */
    void add_file(const std::string &filename, const std::vector<std::string> &lines);

    /**
     * @brief Report (store) an error, warning, or note to the ErrorManager
     *
     * @param error_id The error id. Refer to errors.xlsx
     * @param severity
     * @param location
     * @param message
     * @param suggestion
     */
    void report(int error_id, ErrorSeverity severity, const ErrorLocation &location, const std::string &message, const std::string &suggestion = "");

    // Print all messages in the order they were reported
    void print_all() const;

    // Check if there are any errors (not just warnings or notes)
    bool has_errors() const;

    // Clear all stored messages
    void clear();

    std::string msg_to_string(CompilerMessage msg) const;

private:
    ErrorManager() = default;
    ~ErrorManager() = default;

    std::vector<CompilerMessage> errors; // Store error messages
    std::vector<CompilerMessage> warnings; // Store warning messages
    std::vector<CompilerMessage> notes; // Store note messages
    std::size_t message_order_counter = 0; // Track message order
    std::map<std::string, std::vector<std::string>> _lines; // Store filename -> lines
};

template <typename Useless = int>
void example_check_variable(const std::string &var_name, ErrorManager &error_manager)
{
    if (var_name.empty()) {
        error_manager.report(1001, ErrorSeverity::Error, { "test.cpp", 10, 5 }, "Variable name cannot be empty", "Did you forget to name the variable?");
    } else if (var_name == "int") {
        error_manager.report(1002, ErrorSeverity::Error, { "test.cpp", 10, 5 }, "Variable name cannot be a keyword", "Choose a valid variable name.");
    } else if (var_name.length() == 1) {
        error_manager.report(2001, ErrorSeverity::Warning, { "test.cpp", 12, 3 }, "Variable name is too short", "Consider using a more descriptive name.");
    }
}

template <typename Useless = int>
int example_main()
{
    ErrorManager &error_manager = ErrorManager::instance();

    // Simulate some semantic checks
    example_check_variable("", error_manager);
    example_check_variable("int", error_manager);
    example_check_variable("x", error_manager);

    // Print all reported errors and warnings
    error_manager.print_all();

    if (error_manager.has_errors()) {
        std::cout << "Compilation failed due to errors.\n";
    }

    return 0;
}

#endif