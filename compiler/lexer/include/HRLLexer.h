#ifndef HRLLEXER_H
#define HRLLEXER_H

#include "lexer_global.h"
#include "HRLToken.h"

#include <vector>

OPEN_LEXER_NAMESPACE

class HRLLexer
{
public:
    HRLLexer();
    ~HRLLexer();

    int lex(FILE *in, const std::string &filepath, std::vector<GCToken> &result);

private:
    int lexer_initialize(FILE *in);
    int lexer_finalize();

    GCToken tokenize();
    void print_tokenization_error(const std::string &filepath, int lineno, int colno, int width, const GCString &text, const std::vector<std::string> &lines);

    void get_file_lines(FILE *in, std::vector<std::string> &rows);
};

CLOSE_LEXER_NAMESPACE

#endif