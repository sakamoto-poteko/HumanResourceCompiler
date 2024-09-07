#include <memory>
#include <string>
#include <vector>

#include <boost/format.hpp>

#include <spdlog/spdlog.h>

#include "ErrorManager.h"
#include "ParseTreeNode.h"
#include "RecursiveDescentParser.h"
#include "parser_global.h"

OPEN_PARSER_NAMESPACE

void RecursiveDescentParser::enter_parse_frame()
{
    _parse_frame_token_pointer.push(_token_pointer);
}

void RecursiveDescentParser::revert_parse_frame()
{
    _token_pointer = _parse_frame_token_pointer.top();
    _parse_frame_token_pointer.pop();
}

void RecursiveDescentParser::leave_parse_frame()
{
    _parse_frame_token_pointer.pop();
}

void RecursiveDescentParser::push_error(const std::string &expect, const lexer::TokenPtr &got, int lineno, int colno, int width)
{
    auto err_str = boost::format("Expect %1% but got '%2%'") % expect % *got->token_text();
    _errors.push_back(CompilerMessage {
        .message = err_str.str(),
        .error_id = 2001,
        .location = ErrorLocation {
            .file_name = _filename,
            .column = colno != -1 ? colno : got->colno(),
            .line = lineno != -1 ? lineno : got->lineno(),
            .width = width != -1 ? width : got->width(),
        },
        .severity = ErrorSeverity::Error,
    });
}

void RecursiveDescentParser::report_errors()
{
    auto &errmgr = ErrorManager::instance();
    for (const auto &err : _errors) {
        errmgr.report(err.error_id, err.severity, err.location, err.message, err.suggestion);
    }

    _errors.clear();
}

void RecursiveDescentParser::push_error(int errid, const std::string &message, int lineno, int colno, int width)
{
    _errors.push_back(CompilerMessage {
        .message = message,
        .error_id = errid,
        .location = ErrorLocation {
            .file_name = _filename,
            .column = colno,
            .line = lineno,
            .width = width,
        },
        .severity = ErrorSeverity::Error,
    });
}

void RecursiveDescentParser::pop_error()
{
    _errors.pop_back();
}

void RecursiveDescentParser::pop_error_till(std::list<CompilerMessage>::iterator till_exclusive)
{
    while (!_errors.empty() && std::prev(_errors.end()) != till_exclusive) {
        _errors.pop_back();
    }
}

const lexer::TokenPtr RecursiveDescentParser::NULL_TOKEN = lexer::TokenPtr();

const lexer::TokenPtr &RecursiveDescentParser::lookahead() const
{
    if (_token_pointer < _tokens.size()) {
        return _tokens.at(_token_pointer);
    } else {
        return NULL_TOKEN;
    }
}

void RecursiveDescentParser::consume()
{
    ++_token_pointer;
}

CLOSE_PARSER_NAMESPACE
// end
