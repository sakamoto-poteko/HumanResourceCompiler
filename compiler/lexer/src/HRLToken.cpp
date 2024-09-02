#include "HRLToken.h"

OPEN_LEXER_NAMESPACE

const char *Token::get_token_name() const
{
    switch (_token_id) {
    case END:
        return "END";
    case IMPORT:
        return "IMPORT";
    case RETURN:
        return "RETURN";
    case LET:
        return "LET";
    case INIT:
        return "INIT";
    case FLOOR:
        return "FLOOR";
    case FLOOR_MAX:
        return "FLOOR_MAX";
    case FUNCTION:
        return "FUNCTION";
    case SUBWORD:
        return "SUBWORD";
    case BOOLEAN:
        return "BOOLEAN";
    case INTEGER:
        return "INTEGER";
    case IDENTIFIER:
        return "IDENTIFIER";
    case IF:
        return "IF";
    case ELSE:
        return "ELSE";
    case WHILE:
        return "WHILE";
    case FOR:
        return "FOR";
    case GE:
        return "GE";
    case LE:
        return "LE";
    case EE:
        return "EE";
    case NE:
        return "NE";
    case GT:
        return "GT";
    case LT:
        return "LT";
    case AND:
        return "AND";
    case OR:
        return "OR";
    case NOT:
        return "NOT";
    case ADDADD:
        return "ADDADD";
    case ADD:
        return "ADD";
    case SUBSUB:
        return "SUBSUB";
    case SUB:
        return "SUB";
    case MUL:
        return "MUL";
    case DIV:
        return "DIV";
    case MOD:
        return "MOD";
    case EQ:
        return "EQ";
    case T:
        return "T";
    case OPEN_PAREN:
        return "OPEN_PAREN";
    case CLOSE_PAREN:
        return "CLOSE_PAREN";
    case OPEN_BRACE:
        return "OPEN_BRACE";
    case CLOSE_BRACE:
        return "CLOSE_BRACE";
    case OPEN_BRACKET:
        return "OPEN_BRACKET";
    case CLOSE_BRACKET:
        return "CLOSE_BRACKET";
    case COMMA:
        return "COMMA";
    case ERROR:
        return "ERROR";
    default:
        return "UNKNOWN_TOKEN";
    }
}

CLOSE_LEXER_NAMESPACE