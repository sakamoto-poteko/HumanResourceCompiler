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

    int lex(FILE *in, std::vector<GCToken> &result);

private:
    int lexer_init(FILE *in);
    int lexer_finalize();

    GCToken tokenize();
    void print_tokenization_error();
};

CLOSE_LEXER_NAMESPACE

#endif