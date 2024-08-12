#include "HRLLexer.h"

extern FILE* yyin;
int yylex();
static std::string __lex_filename;

OPEN_LEXER_NAMESPACE

HRLLexer::HRLLexer()
{

}

HRLLexer::~HRLLexer()
{

}

int HRLLexer::lex(FILE *in, std::vector<GCToken> &result)
{
    lexer_finalize(); // clean up if there's previous lexing context

    std::vector<GCToken> r;

    // begin tokenization
    TokenId currentTokenId = END;

    do {
        GCToken token = tokenize();
        r.push_back(token);
    } while (currentTokenId > 0);

    if (currentTokenId == END) {
        
    }

    if (currentTokenId == ERROR) {

    }

    // this is not supposed to happen. tokenization ended but not with either END or ERROR.
    // this must be a bug
    return 0;
}

int HRLLexer::lexer_init(FILE *in)
{
    yyin = in;
    return 0;
}

int HRLLexer::lexer_finalize()
{
    yyin = nullptr;
    return 0;
}

CLOSE_LEXER_NAMESPACE