#include <sstream>

#include <spdlog/spdlog.h>

#include "TerminalColor.h"
#include "lexer_helper.h"
#include "HRLLexer.h"

extern int yyleng;
extern char *yytext;
extern FILE *yyin;
extern int yylineno;
extern int yycolno;
int yylex();

OPEN_LEXER_NAMESPACE

HRLLexer::HRLLexer()
{
}

HRLLexer::~HRLLexer()
{
}

int HRLLexer::lex(FILE *in, const std::string &filepath, std::vector<GCToken> &result)
{
    lexer_finalize(); // clean up if there's previous lexing context
    lexer_initialize(in);

    std::vector<std::string> lines;
    get_file_lines(in, lines);

    std::vector<GCToken> r;

    // begin tokenization
    TokenId currentTokenId = END;

    do {
        GCToken token = tokenize();
        currentTokenId = token->get_token_id();
        r.push_back(token);
    } while (currentTokenId > 0);

    if (currentTokenId == END) {
        result.swap(r);
        return 0;
    }

    if (currentTokenId == ERROR) {
        const auto &token = r.back();
        print_tokenization_error(filepath, token->get_lineno(), token->get_col(), token->get_width(), token->get_token_text(), lines);
        return -1;
    }
    // this is not supposed to happen. tokenization ended but not with either END or ERROR.
    // this must be a bug
    return -1;
}

int HRLLexer::lexer_initialize(FILE *in)
{
    yyin = in;
    __currentToken.boolean = false;
    __currentToken.identifier = GCString();
    __currentToken.integer = 0;
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
    int lineno = yylineno; // line starts from 1
    int colno = yycolno - yyleng + 1; // colno starts from 1
    int width = yyleng;

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
    case COMMA:
        return std::make_shared<Token>(tokenId, lineno, colno, width, std::make_shared<std::string>(yytext));

    // tokens has some payloads
    case BOOLEAN:
        return std::make_shared<BooleanToken>(tokenId, __currentToken.boolean, lineno, colno, width, std::make_shared<std::string>(yytext));

    case INTEGER:
        return std::make_shared<IntegerToken>(tokenId, __currentToken.integer, lineno, colno, width, std::make_shared<std::string>(yytext));

    case IDENTIFIER:
        return std::make_shared<IdentifierToken>(tokenId, __currentToken.identifier, lineno, colno, width, std::make_shared<std::string>(yytext));

    case END:
        return std::make_shared<Token>(END, lineno, colno, width, std::make_shared<std::string>(yytext));

    case ERROR:
        return std::make_shared<Token>(ERROR, lineno, colno, width, std::make_shared<std::string>(yytext));

    default:
        spdlog::critical("bug: unreachable tokenize default");
        abort();
    }

    return GCToken();
}

void HRLLexer::print_tokenization_error(const std::string &filepath, int lineno, int colno, int width, const GCString &text, const std::vector<std::string> &lines)
{
    spdlog::error("{}:{}:{}:{}Unrecognized token `{}'{}", filepath, lineno, colno, __tc.COLOR_HIGHLIGHT, *(text.get()), __tc.COLOR_RESET);
    spdlog::error(lines.at(lineno - 1)); // line starts from 1
    std::stringstream ss;
    for (int i = 1; i < colno; ++i) {
        ss << ' ';
    }
    for (int i = 0; i < width; ++i) {
        ss << '^';
    }
    spdlog::error("{}{}{}", __tc.COLOR_LIGHT_GREEN, ss.str(), __tc.COLOR_RESET);
}

void HRLLexer::get_file_lines(FILE *in, std::vector<std::string> &rows)
{
    std::vector<std::string> source_rows;

    char buf[1024];
    long current_pos = ftell(in);
    char *line;
    std::fseek(in, current_pos, SEEK_SET);

    std::string current_line;
    current_line.reserve(128);

    while (!std::feof(in)) {
        size_t bytes_read = fread(buf, 1, sizeof(buf), in);

        for (size_t i = 0; i < bytes_read; ++i) {
            if (buf[i] == '\n') {
                source_rows.push_back(current_line);
                current_line.clear();
            } else {
                current_line.push_back(buf[i]);
            }
        }
    }

    source_rows.push_back(current_line);

    fseek(in, current_pos, SEEK_SET);

    rows.swap(source_rows);
}

CLOSE_LEXER_NAMESPACE