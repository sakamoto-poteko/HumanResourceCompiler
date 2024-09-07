#include "ErrorFilters.h"

bool error_filter_transform_E2001_with_suggestion(CompilerMessage &msg)
{
    if (msg.error_id == 2001
        && msg.severity == ErrorSeverity::Error
        && msg.suggestion.empty()
        && msg.message.find("Expect ';'") != std::string::npos) {
        msg.suggestion = "A semicolon (';') is required to end the statement";
    }
    return true;
}
