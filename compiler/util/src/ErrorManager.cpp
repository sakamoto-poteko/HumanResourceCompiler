#include <sstream>
#include <string>

#include "ErrorFilters.h"
#include "ErrorManager.h"
#include "TerminalColor.h"

void ErrorManager::report(int error_id, ErrorSeverity severity, const ErrorLocation &location, const std::string &message, const std::string &suggestion)
{
    CompilerMessage msg(error_id, severity, location, message, suggestion);
    msg.order = _message_order_counter++;

    // Store in appropriate container based on severity
    switch (severity) {
    case ErrorSeverity::Error:
        _errors.push_back(msg);
        break;
    case ErrorSeverity::Warning:
        _warnings.push_back(msg);
        break;
    case ErrorSeverity::Note:
        _notes.push_back(msg);
        break;
    }
}

bool apply_filters(CompilerMessage &msg);

void ErrorManager::print_all() const
{
    // Temporary vector to hold all messages
    std::vector<CompilerMessage> all_messages;

    for (auto msg : _errors) {
        if (apply_filters(msg)) {
            all_messages.push_back(msg);
        }
    }

    for (auto msg : _warnings) {
        if (apply_filters(msg)) {
            all_messages.push_back(msg);
        }
    }

    for (auto msg : _notes) {
        if (apply_filters(msg)) {
            all_messages.push_back(msg);
        }
    }

    // Combine all messages from the different vectors
    // all_messages.insert(all_messages.end(), _errors.begin(), _errors.end());
    // all_messages.insert(all_messages.end(), _warnings.begin(), _warnings.end());
    // all_messages.insert(all_messages.end(), _notes.begin(), _notes.end());

    // Sort messages based on their order
    std::sort(all_messages.begin(), all_messages.end(), [](const CompilerMessage &a, const CompilerMessage &b) {
        return a.order < b.order;
    });

    // Print messages
    for (const auto &msg : all_messages) {
        switch (msg.severity) {
        case ErrorSeverity::Error:
            spdlog::error(msg_to_string(msg));
            break;
        case ErrorSeverity::Warning:
            spdlog::warn(msg_to_string(msg));
            break;
        case ErrorSeverity::Note:
            spdlog::info(msg_to_string(msg));
            break;
        }
    }
}

std::string ErrorManager::msg_to_string(CompilerMessage msg) const
{
    std::stringstream ss;

    // Prefix based on severity
    // spdlog handles that.

    switch (msg.severity) {
    case ErrorSeverity::Error:
        // ss << "error: ";
        ss << "[E";
        break;
    case ErrorSeverity::Warning:
        // ss << "warning: ";
        ss << "[W";
        break;
    case ErrorSeverity::Note:
        // ss << "note: ";
        ss << "[N";
        break;
    }

    // Show error ID, location, and message
    ss << msg.error_id << "] " << msg.location.to_string() << ": " << __tc.COLOR_HIGHLIGHT << msg.message << __tc.COLOR_RESET;

    if (_lines.contains(msg.location.file_name)) {
        ss << "\n"
           << __tc.COLOR_LIGHT_GREEN;
        ss << _lines.at(msg.location.file_name).at(msg.location.line - 1); // line starts from 1
        ss << "\n";
        for (int i = 1; i < msg.location.column; ++i) {
            ss << ' ';
        }
        ss << __tc.COLOR_LIGHT_RED;
        if (msg.location.width != 0) {
            for (std::size_t i = 0; i < msg.location.width; ++i) {
                ss << '^';
            }
        } else {
            ss << '^';
        }
        ss << __tc.COLOR_RESET;
    }

    // Add a suggestion, if available
    if (!msg.suggestion.empty()) {
        ss << "\nSuggestion: " << msg.suggestion;
    }

    return ss.str();
}

ErrorManager &ErrorManager::instance()
{
    static ErrorManager err_mgr;
    return err_mgr;
}

void ErrorManager::add_file(const std::string &filename, const std::vector<std::string> &lines)
{
    _lines[filename] = lines;
}

bool ErrorManager::has_errors() const
{
    return !_errors.empty();
}

void ErrorManager::clear()
{
    _errors.clear();
    _warnings.clear();
    _notes.clear();
    _message_order_counter = 0;
}

void ErrorManager::add_error_filter(ErrorFilter error_filter)
{
    _error_filters.push_back(error_filter);
}

bool ErrorManager::apply_filters(CompilerMessage &msg) const
{
    for (const auto &filter : _error_filters) {
        bool ok = filter(msg);
        if (!ok) {
            return false;
        }
    }
    return true;
}

void ErrorManager::add_common_filters()
{
    add_error_filter(error_filter_transform_E2001_with_suggestion);
}

void ErrorManager::report(CompilerMessage message)
{
    message.order = _message_order_counter++;

    // Store in appropriate container based on severity
    switch (message.severity) {
    case ErrorSeverity::Error:
        _errors.push_back(std::move(message));
        break;
    case ErrorSeverity::Warning:
        _errors.push_back(std::move(message));
        break;
    case ErrorSeverity::Note:
        _errors.push_back(std::move(message));
        break;
    }
}
