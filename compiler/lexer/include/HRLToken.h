#ifndef HRL_TOKEN
#define HRL_TOKEN

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

    ERROR = -1,
};

class Token {
public:
    Token(TokenId tokenId, int row, int col, int width, GCString text)
        : _tokenId(tokenId)
        , _row(row)
        , _col(col)
        , _width(width)
        , _text(text)
    {
    }
    virtual ~Token() { }

protected:
    TokenId _tokenId;

    int _row;
    int _col;
    int _width;
    GCString _text;
};

class BooleanToken : public Token {
public:
    BooleanToken(TokenId id, bool value, int row, int col, int width, GCString text)
        : Token(id, row, col, width, text)
        , _value(value)
    {
    }

protected:
    bool _value;
};

class IntegerToken : public Token {
public:
    IntegerToken(TokenId id, int value, int row, int col, int width, GCString text)
        : Token(id, row, col, width, text)
        , _value(value)
    {
    }

protected:
    int _value;
};

class IdentifierToken : public Token {
public:
    IdentifierToken(TokenId id, GCString value, int row, int col, int width, GCString text)
        : Token(id, row, col, width, text)
        , _value(value)
    {
    }

protected:
    GCString _value;
};

CLOSE_LEXER_NAMESPACE

#endif