#ifndef SYMBOL_H
#define SYMBOL_H

#include "ASTNode.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

enum class SymbolType {
    VARIABLE,
    SUBROUTINE,
};

class Symbol : public parser::ASTNodeAttribute {
public:
    SymbolType type;
    std::string name;
    StringPtr filename;

    // It's weak to prevent circular reference.
    WEAK(parser::ASTNodePtr)
    definition;

    Symbol(SymbolType type, const std::string &name, StringPtr filename, WEAK(parser::ASTNodePtr) definition)
        : type(type)
        , name(name)
        , filename(std::move(filename))
        , definition(std::move(definition))
    {
    }

    Symbol(SymbolType type, StringPtr name, StringPtr filename, WEAK(parser::ASTNodePtr) definition)
        : type(type)
        , name(*name)
        , filename(std::move(filename))
        , definition(std::move(definition))
    {
    }

    int get_type() override { return SemAnalzyerASTNodeAttributeId::ATTR_SEMANALYZER_SYMBOL; }

    std::string to_string() override;

    void set_param(bool has) { _has_param = has; }

    void set_return(bool has) { _has_return = has; }

    bool has_param() const { return _has_param; }

    bool has_return() const { return _has_return; }

private:
    bool _has_param = false;
    bool _has_return = false;
};

using SymbolPtr = std::shared_ptr<Symbol>;

CLOSE_SEMANALYZER_NAMESPACE

#endif