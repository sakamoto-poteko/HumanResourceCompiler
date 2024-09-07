#ifndef HRLLEXER_H
#define HRLLEXER_H

#include "ErrorManager.h"
#include "lexer_global.h"

#include <vector>

OPEN_LEXER_NAMESPACE

class HRLLexer {
public:
    HRLLexer();
    ~HRLLexer();

    bool lex(FILE *in, const std::string &filepath, std::vector<TokenPtr> &result);

private:
    int lexer_initialize(FILE *in);
    int lexer_finalize();

    TokenPtr tokenize();
    void print_tokenization_error(const std::string &filepath, int lineno, int colno, std::size_t width, const StringPtr &text);

    void get_file_lines(FILE *in, std::vector<std::string> &rows);

    ErrorManager &_errmgr;
};

CLOSE_LEXER_NAMESPACE

#endif