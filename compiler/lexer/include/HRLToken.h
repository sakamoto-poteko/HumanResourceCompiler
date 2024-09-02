#ifndef HRL_TOKEN
#define HRL_TOKEN

#include <memory>

#include "hrl_global.h"
#include "lexer_global.h"

OPEN_LEXER_NAMESPACE

enum TokenId : int {
    END = 0,

    IMPORT = 256,
    RETURN,
    LET,
    INIT,
    FLOOR,
    FLOOR_MAX,
    FUNCTION,
    SUBWORD,

    BOOLEAN,
    INTEGER,
    IDENTIFIER,

    IF,
    ELSE,
    WHILE,
    FOR,
    GE,
    LE,
    EE,
    NE,
    GT,
    LT,

    AND,
    OR,
    NOT,
    ADDADD,
    ADD,
    SUBSUB,
    SUB,
    MUL,
    DIV,
    MOD,
    EQ,

    T,

    OPEN_PAREN,
    CLOSE_PAREN,
    OPEN_BRACE,
    CLOSE_BRACE,
    OPEN_BRACKET,
    CLOSE_BRACKET,
    COMMA,

    ERROR = -1,
};

inline bool is_token_binary_operator(TokenId token)
{
    switch (token) {
    case GE:
    case LE:
    case EE:
    case NE:
    case GT:
    case LT:
    case AND:
    case OR:
    case NOT:
    case ADD:
    case SUB:
    case MUL:
    case DIV:
    case MOD:
        return true;
    default:
        return false;
    }
}

class Token : std::enable_shared_from_this<Token> {
public:
    Token(TokenId tokenId, int row, int col, int width, StringPtr text)
        : _token_id(tokenId)
        , _lineno(row)
        , _colno(col)
        , _width(width)
        , _text(text)
    {
    }

    virtual ~Token() { }

    const char *get_token_name() const;

    TokenId token_id() const { return _token_id; }

    StringPtr token_text() const { return _text; }

    int lineno() const { return _lineno; }

    int colno() const { return _colno; }

    int width() const { return _width; }

protected:
    TokenId _token_id;

    int _lineno;
    int _colno;
    int _width;
    StringPtr _text;
};

class BooleanToken : public Token {
public:
    BooleanToken(TokenId id, bool value, int row, int col, int width, StringPtr text)
        : Token(id, row, col, width, text)
        , _value(value)
    {
    }

    int get_value() const { return _value; }

protected:
    bool _value;
};

class IntegerToken : public Token {
public:
    IntegerToken(TokenId id, int value, int row, int col, int width, StringPtr text)
        : Token(id, row, col, width, text)
        , _value(value)
    {
    }

    int get_value() const { return _value; }

protected:
    int _value;
};

class IdentifierToken : public Token {
public:
    IdentifierToken(TokenId id, StringPtr value, int row, int col, int width, StringPtr text)
        : Token(id, row, col, width, text)
        , _value(value)
    {
    }

    StringPtr get_value() const { return _value; }

protected:
    StringPtr _value;
};

CLOSE_LEXER_NAMESPACE

#endif