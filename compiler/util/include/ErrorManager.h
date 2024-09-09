#ifndef ERRORMANAGER_H
#define ERRORMANAGER_H

#include <functional>
#include <map>
#include <string>
#include <vector>

#include <boost/format.hpp>

#include <spdlog/spdlog.h>

#include "ErrorMessage.h"

/**
 * @brief Manages error reporting and message filtering during the compilation process.
 *
 * The ErrorManager class is responsible for storing and managing error, warning, and note messages
 * generated during compilation. It supports adding filters to control which messages are reported,
 * adding file content for detailed inline error reporting, and handling message severity (errors, warnings, and notes).
 *
 * ErrorManager is a singleton class, ensuring that there is only one instance of error management throughout the compilation process.
 */
class ErrorManager {
public:
    ErrorManager(const ErrorManager &) = delete;
    ErrorManager &operator=(const ErrorManager &) = delete;

    static ErrorManager &instance();

    using CompilerMessageFilter = std::function<bool(CompilerMessage &)>;

    /**
     * @brief Add the file content to the ErrorManager, so the error message is able to print the inline text
     *
     * @param filename
     * @param lines
     */
    void add_file(const std::string &filename, const std::vector<std::string> &lines);

    /**
     * @brief Add a message filter to the ErrorManager. The filter will be executed when \c print_all() is called.
     * The raw data won't be transformed by the filter, but a copy in printing will.
     *
     * @param message_filter A bool func(CompilerMessage &). The filter should return true if the message is to be kept, false if deleted.
     */
    void add_message_filter(CompilerMessageFilter message_filter);

    /**
     * @brief Add common filters to the ErrorManager.
     *
     * Common filters include:
     *   - Filter that adds suggestion on Error 2001 when expect ; but got add others
     *
     */
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

    /**
     * @copybrief void report(int error_id, ErrorSeverity severity, const ErrorLocation &location, const std::string &message, const std::string &suggestion = "")
     *
     * @param message
     */
    void report(CompilerMessage message);

    /**
     * @brief Report extra information following the last error with severity \p severity
     *
     * @param severity
     * @param location
     * @param message
     */
    void report_continued(ErrorSeverity severity, const ErrorLocation &location, const std::string &message);

    /**
     * @brief Print all errors, warnings and notes
     *
     */
    void print_all() const;

    /**
     * @brief Check if there are any errors (not just warnings or notes)
     *
     * @return true
     * @return false
     */
    bool has_errors() const;

    // Clear all stored messages
    /**
     * @brief Clear all stored messages
     *
     */
    void clear();

private:
    ErrorManager() = default;
    ~ErrorManager() = default;

    std::vector<CompilerMessage> _errors; // Store error messages
    std::vector<CompilerMessage> _warnings; // Store warning messages
    std::vector<CompilerMessage> _notes; // Store note messages
    std::size_t _message_order_counter = 0; // Track message order
    std::map<std::string, std::vector<std::string>> _lines; // Store filename -> lines
    std::vector<CompilerMessageFilter> _error_filters;

    bool apply_filters(CompilerMessage &msg) const;
    std::string msg_to_string(CompilerMessage msg) const;
};

#endif