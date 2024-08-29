#include "FirstFollowElement.h"

const char *FirstSetElement::type_str(Type type)
{
    switch (type) {
    case Literal:
        return "Literal";
    case Epsilon:
        return "Epsilon";
    case Reference:
        return "Reference";
    case Token:
        return "Token";
    }
    return "ERROR";
}
