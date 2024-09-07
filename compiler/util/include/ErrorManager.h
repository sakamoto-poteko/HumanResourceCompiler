#ifndef ERRORMANAGER_H
#define ERRORMANAGER_H

#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

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
    int line;
    int column;

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

    // Function to display the error with formatting
    std::string to_string() const
    {
        std::stringstream ss;

        // Prefix based on severity
        switch (severity) {
        case ErrorSeverity::Error:
            ss << "error: ";
            break;
        case ErrorSeverity::Warning:
            ss << "warning: ";
            break;
        case ErrorSeverity::Note:
            ss << "note: ";
            break;
        }

        // Show error ID, location, and message
        ss << "[E" << error_id << "] " << location.to_string() << ": " << message << "\n";

        // Add a suggestion, if available
        if (!suggestion.empty()) {
            ss << "    suggestion: " << suggestion << "\n";
        }

        return ss.str();
    }
};

// ErrorManager class
class ErrorManager {
private:
    std::vector<CompilerMessage> errors; // Store error messages
    std::vector<CompilerMessage> warnings; // Store warning messages
    std::vector<CompilerMessage> notes; // Store note messages
    size_t message_order_counter = 0; // Track message order

public:
    // Report an error, warning, or note
    void report(int error_id, ErrorSeverity severity, const ErrorLocation &location, const std::string &message, const std::string &suggestion = "")
    {
        CompilerMessage msg = {
            error_id,
            severity,
            location,
            message,
            suggestion,
            message_order_counter++,
        };

        // Store in appropriate container based on severity
        switch (severity) {
        case ErrorSeverity::Error:
            errors.push_back(msg);
            break;
        case ErrorSeverity::Warning:
            warnings.push_back(msg);
            break;
        case ErrorSeverity::Note:
            notes.push_back(msg);
            break;
        }
    }

    // Print all messages in the order they were reported
    void print_all() const
    {
        // Temporary vector to hold all messages
        std::vector<CompilerMessage> all_messages;

        // Combine all messages from the different vectors
        all_messages.insert(all_messages.end(), errors.begin(), errors.end());
        all_messages.insert(all_messages.end(), warnings.begin(), warnings.end());
        all_messages.insert(all_messages.end(), notes.begin(), notes.end());

        // Sort messages based on their order
        std::sort(all_messages.begin(), all_messages.end(), [](const CompilerMessage &a, const CompilerMessage &b) {
            return a.order < b.order;
        });

        // Print messages
        for (const auto &msg : all_messages) {
            switch (msg.severity) {
            case ErrorSeverity::Error:
                spdlog::error(msg.to_string());
                break;
            case ErrorSeverity::Warning:
                spdlog::warn(msg.to_string());
                break;
            case ErrorSeverity::Note:
                spdlog::info(msg.to_string());
                break;
            }
        }
    }

    // Check if there are any errors (not just warnings or notes)
    bool has_errors() const
    {
        return !errors.empty();
    }

    // Clear all stored messages
    void clear()
    {
        errors.clear();
        warnings.clear();
        notes.clear();
        message_order_counter = 0;
    }
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
    ErrorManager error_manager;

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