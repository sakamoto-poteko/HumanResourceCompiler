#include "EscapeGraphviz.h"

std::string escape_graphviz(const std::string &text)
{
    std::string escaped;

    for (char it : text) {
        switch (it) {
        case '\\':
            escaped.append("\\\\");
            break;
        case '\n':
            escaped.append("\\n");
            break;
        case '"':
            escaped.append("\\\"");
            break;
        case '{':
            escaped.append("\\{");
            break;
        case '}':
            escaped.append("\\}");
            break;
        case '<':
            escaped.append("\\<");
            break;
        case '>':
            escaped.append("\\>");
            break;
        case '&':
            escaped.append("&amp;");
            break;
        default:
            escaped.push_back(it); // No escape needed
            break;
        }
    }

    return escaped;
}

std::string escape_graphviz_html(const std::string &text)
{
    std::string escaped;
    for (char ch : text) {
        switch (ch) {
        case '&':
            escaped += "&amp;";
            break;
        case '<':
            escaped += "&lt;";
            break;
        case '>':
            escaped += "&gt;";
            break;
        case '"':
            escaped += "&quot;";
            break;
        case '\'':
            escaped += "&#39;";
            break;
        case '%':
            escaped += "&#37;";
            break;
        case '#':
            escaped += "&#35;";
            break;
        default:
            escaped += ch;
            break;
        }
    }
    return escaped;
}
