#include <memory>
#include <string>
#include <vector>

#include <boost/format.hpp>

#include <spdlog/spdlog.h>

#include "ASTNode.h"
#include "HRLToken.h"
#include "RecursiveDescentParser.h"
#include "hrl_global.h"
#include "lexer_global.h"
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

void RecursiveDescentParser::push_error(const std::string &expect, const lexer::TokenPtr &got, int lineno, int colno)
{
    auto err_str = boost::format("Expect %1% but got '%2%'. [%3%:%4%]")
        % expect % got->token_text() % (lineno != -1 ? lineno : got->lineno()) % (colno != -1 ? colno : got->colno());

    _errors.push_back(err_str.str());
}

void RecursiveDescentParser::print_error()
{
    for (const auto &err : _errors) {
        spdlog::error(err);
    }

    _errors.clear();
}

void RecursiveDescentParser::push_error(const std::string &message, int lineno, int colno)
{
    auto err_str = boost::format("%1%%2%")
        % message
        % (lineno != -1 && colno != -1 ? boost::format(" [%1%:%2%]") % lineno % colno : boost::format());

    _errors.push_back(err_str.str());
}

void RecursiveDescentParser::pop_error()
{
    _errors.pop_back();
}

void RecursiveDescentParser::pop_error_till(std::list<std::string>::iterator till_exclusive)
{
    while (!_errors.empty() && std::prev(_errors.end()) != till_exclusive) {
        _errors.pop_back();
    }
}

CLOSE_PARSER_NAMESPACE
// end
