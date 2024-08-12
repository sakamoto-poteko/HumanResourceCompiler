#include <spdlog/spdlog.h>

#include "lexer_helper.h"

#include "HRLLexer.h"

extern int yyleng;
extern char *yytext;
extern FILE *yyin;
extern int yylineno;
extern int yycolno;
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
        result.swap(r);
        return 0;
    }

    if (currentTokenId == ERROR) {
        return -1;
    }

    // this is not supposed to happen. tokenization ended but not with either END or ERROR.
    // this must be a bug
    return -1;
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

GCToken HRLLexer::tokenize()
{
    int val = yylex();
    TokenId tokenId = static_cast<TokenId>(val);
    int lineno = yylineno;

    switch (val) {
        // these are keywords and operators. no extra info stored in the token
    case IMPORT:
    case RETURN:
    case LET:
    case INIT:
    case FLOOR:
    case FLOOR_MAX:
    case FUNCTION:
    case SUBWORD:
    case IF:
    case ELSE:
    case WHILE:
    case FOR:
    case GE:
    case LE:
    case EE:
    case NE:
    case GT:
    case LT:
    case AND:
    case OR:
    case NOT:
    case ADDADD:
    case ADD:
    case SUBSUB:
    case SUB:
    case MUL:
    case DIV:
    case MOD:
    case EQ:
    case T:
    case OPEN_PAREN:
    case CLOSE_PAREN:
    case OPEN_BRACE:
    case CLOSE_BRACE:
    case OPEN_BRACKET:
    case CLOSE_BRACKET:
        return std::make_shared<Token>(tokenId, lineno, yycolno, yyleng, std::make_shared<std::string>(yytext));

    case BOOLEAN:
        return std::make_shared<BooleanToken>(tokenId, __currentToken.boolean, lineno, yycolno, yyleng, std::make_shared<std::string>(yytext));

    case INTEGER:
        return std::make_shared<IntegerToken>(tokenId, __currentToken.integer, lineno, yycolno, yyleng, std::make_shared<std::string>(yytext));

    case IDENTIFIER:
        return std::make_shared<IdentifierToken>(tokenId, __currentToken.identifier, lineno, yycolno, yyleng, std::make_shared<std::string>(yytext));

    case END:
        return std::make_shared<Token>(END, lineno, yycolno, yyleng, std::make_shared<std::string>(yytext));

    case ERROR:
        print_tokenization_error();
        return std::make_shared<Token>(ERROR, lineno, yycolno, yyleng, std::make_shared<std::string>(yytext));

    default:
        spdlog::critical("bug: unreachable tokenize default");
        abort();
    }

    return GCToken();
}

void HRLLexer::print_tokenization_error()
{
    spdlog::error("Unrecognized token `%s'", yytext);
}

CLOSE_LEXER_NAMESPACE