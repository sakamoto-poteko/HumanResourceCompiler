#ifndef HRL_TOKEN
#define HRL_TOKEN

#include <memory>
#include <vector>

#include "hrl_global.h"
#include "lexer_global.h"
#include "lexer_helper.h"

OPEN_LEXER_NAMESPACE

enum TokenId : int {
    END = 0,

    IMPORT = 256,
    
    LET,
    INIT,
    FLOOR,
    FLOOR_MAX,
    FUNCTION,
    SUBWORD,

    BOOLEAN,
    INTEGER,
    IDENTIFIER,

    RETURN,
    BREAK,
    CONTINUE,

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

    COMMENT = 999,
    NEWLINE,

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
    Token(TokenId tokenId, int row, int col, int width, StringPtr text, const std::vector<TokenMetadata> &metadata)
        : _token_id(tokenId)
        , _lineno(row)
        , _colno(col)
        , _width(width)
        , _text(text)
        , _metadata(metadata)
    {
    }

    virtual ~Token() { }

    const char *get_token_name() const;

    TokenId token_id() const { return _token_id; }

    StringPtr token_text() const { return _text; }

    int lineno() const { return _lineno; }

    int colno() const { return _colno; }

    int width() const { return _width; }

    const std::vector<TokenMetadata> &metadata() const { return _metadata; }

    void prepend_metadata_newline() { _metadata.insert(_metadata.begin(), TokenMetadata { .type = TokenMetadata::Newline }); }

protected:
    TokenId _token_id;

    int _lineno;
    int _colno;
    int _width;
    StringPtr _text;
    std::vector<TokenMetadata> _metadata;
};

class BooleanToken : public Token {
public:
    BooleanToken(TokenId id, bool value, int row, int col, int width, StringPtr text, const std::vector<TokenMetadata> &metadata)
        : Token(id, row, col, width, text, metadata)
        , _value(value)
    {
    }

    int get_value() const { return _value; }

protected:
    bool _value;
};

class IntegerToken : public Token {
public:
    IntegerToken(TokenId id, int value, int row, int col, int width, StringPtr text, const std::vector<TokenMetadata> &metadata)
        : Token(id, row, col, width, text, metadata)
        , _value(value)
    {
    }

    int get_value() const { return _value; }

protected:
    int _value;
};

class IdentifierToken : public Token {
public:
    IdentifierToken(TokenId id, StringPtr value, int row, int col, int width, StringPtr text, const std::vector<TokenMetadata> &metadata)
        : Token(id, row, col, width, text, metadata)
        , _value(value)
    {
    }

    StringPtr get_value() const { return _value; }

protected:
    StringPtr _value;
};

CLOSE_LEXER_NAMESPACE

#endif