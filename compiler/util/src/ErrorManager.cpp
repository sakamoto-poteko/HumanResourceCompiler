#include "ErrorManager.h"

void ErrorManager::report(int error_id, ErrorSeverity severity, const ErrorLocation &location, const std::string &message, const std::string &suggestion)
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

void ErrorManager::print_all() const
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
    ss << msg.error_id << "] " << msg.location.to_string() << ": " << __tc.COLOR_HIGHLIGHT << msg.message << __tc.COLOR_RESET << "\n";

    if (msg.location.width != 0 && _lines.contains(msg.location.file_name)) {
        ss << __tc.COLOR_LIGHT_GREEN;
        ss << _lines.at(msg.location.file_name).at(msg.location.line - 1); // line starts from 1
        std::stringstream ss;
        for (int i = 1; i < msg.location.column; ++i) {
            ss << ' ';
        }
        for (int i = 0; i < msg.location.width; ++i) {
            ss << '^';
        }
        ss << __tc.COLOR_RESET;
    }

    // Add a suggestion, if available
    if (!msg.suggestion.empty()) {
        ss << "    suggestion: " << msg.suggestion << "\n";
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
    return !errors.empty();
}

void ErrorManager::clear()
{
    errors.clear();
    warnings.clear();
    notes.clear();
    message_order_counter = 0;
}
