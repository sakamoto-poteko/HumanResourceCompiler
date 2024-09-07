#ifndef ERRORMANAGER_H
#define ERRORMANAGER_H

#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include <boost/format.hpp>

#include <spdlog/spdlog.h>

#include "TerminalColor.h"
#include "ErrorMessage.h"


// ErrorManager class
class ErrorManager {
public:
    ErrorManager(const ErrorManager &) = delete;
    ErrorManager &operator=(const ErrorManager &) = delete;

    static ErrorManager &instance();

    using ErrorFilter = std::function<bool(CompilerMessage &)>;

    /**
     * @brief Add the file content to the ErrorManager, so the error message is able to print the inline text
     *
     * @param filename
     * @param lines
     */
    void add_file(const std::string &filename, const std::vector<std::string> &lines);

    void add_error_filter(ErrorFilter error_filter);

    void add_common_filters();

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

    std::vector<CompilerMessage> _errors; // Store error messages
    std::vector<CompilerMessage> _warnings; // Store warning messages
    std::vector<CompilerMessage> _notes; // Store note messages
    std::size_t _message_order_counter = 0; // Track message order
    std::map<std::string, std::vector<std::string>> _lines; // Store filename -> lines
    std::vector<ErrorFilter> _error_filters;

    bool apply_filters(CompilerMessage &msg) const;
};

#endif