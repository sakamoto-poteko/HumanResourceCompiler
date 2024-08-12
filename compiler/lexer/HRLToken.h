#ifndef HRL_TOKEN
#define HRL_TOKEN

#include "lexer_global.h"

OPEN_LEXER_NAMESPACE

enum TokenId {
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
    Token(TokenId tokenId, GCString text)
        : _tokenId(tokenId)
        , _text(text)
    {
    }

protected:
    TokenId _tokenId;
    GCString _text;

private:
};

CLOSE_LEXER_NAMESPACE

#endif