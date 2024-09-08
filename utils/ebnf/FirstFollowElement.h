#ifndef FIRSTFOLLOWELEMENT_H
#define FIRSTFOLLOWELEMENT_H

#include <string>

struct FirstSetElement {
    enum Type {
        Literal,
        Epsilon,
        Token,
        Reference,
    };

    // value is literal value, token value referenced name of another production
    std::string value;
    Type type;
    // produced_by is the production id after expansion
    std::string produced_by;

    explicit FirstSetElement(const std::string &value, Type type)
        : value(value)
        , type(type)
    {
    }

    explicit FirstSetElement(const std::string &value, Type type, const std::string &produced_by)
        : value(value)
        , type(type)
        , produced_by(produced_by)
    {
    }

    static const char *type_str(Type type);

    bool operator<(const FirstSetElement &other) const
    {
        if (type == other.type) {
            return value < other.value;
        }
        return type < other.type;
    }
};

struct FollowSetElement {
    enum Type {
        Literal,
        Epsilon,
        Token,
        Reference,
    };

    // value is literal value, token value referenced name of another production
    std::string value;
    Type type;
    // produced_by is the production id after expansion
    std::string produced_by;

    explicit FollowSetElement(const std::string &value, Type type)
        : value(value)
        , type(type)
    {
    }

    explicit FollowSetElement(const std::string &value, Type type, const std::string &produced_by)
        : value(value)
        , type(type)
        , produced_by(produced_by)
    {
    }

    static const char *type_str(Type type);

    bool operator<(const FollowSetElement &other) const
    {
        if (type == other.type) {
            return value < other.value;
        }
        return type < other.type;
    }

    Type get_type_from_first_set(FirstSetElement::Type first_type)
    {
        switch (first_type) {
        case FirstSetElement::Literal:
            return Type::Literal;
        case FirstSetElement::Epsilon:
            return Type::Epsilon;
        case FirstSetElement::Token:
            return Type::Token;
        case FirstSetElement::Reference:
            return Type::Reference;
        }
    }
};

#endif