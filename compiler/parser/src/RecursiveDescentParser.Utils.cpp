#include <string>
#include <vector>

#include <boost/format.hpp>

#include <spdlog/spdlog.h>

#include "ErrorManager.h"
#include "HRLToken.h" // IWYU pragma: keep
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

void RecursiveDescentParser::push_error(const std::string &expect, const lexer::TokenPtr &got, int lineno, int colno, std::size_t width)
{
    auto err_str = boost::format("Expect %1% but got '%2%'") % expect % *got->token_text();
    _errors.emplace_back(
        2001,
        ErrorSeverity::Error,
        ErrorLocation(_filename,
            lineno != -1 ? lineno : got->lineno(),
            colno != -1 ? colno : got->colno(),
            width == 0 ? got->width() : 0),
        err_str.str());
}

void RecursiveDescentParser::report_errors()
{
    auto &errmgr = ErrorManager::instance();
    for (const auto &err : _errors) {
        errmgr.report(err);
    }

    _errors.clear();
}

void RecursiveDescentParser::push_error(int errid, const std::string &message, int lineno, int colno, std::size_t width)
{
    _errors.emplace_back(errid, ErrorSeverity::Error, ErrorLocation(_filename, lineno, colno, width), message);
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
