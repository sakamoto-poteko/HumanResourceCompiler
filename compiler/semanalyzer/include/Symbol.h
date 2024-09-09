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

    bool get_var_initialized() { return type == SymbolType::VARIABLE ? false : false; }

    // [Group] Constant propagation functions
    // If the variable is set with a constant and was not changed afterwards,
    // the constant value could be propagated to the expression.
    // If it's changed, or it's initialized as a non-constant, the value is invalid.

    /**
     * @brief Get the variable value of the symbol. The value may be invalid. Be sure to
     * check get_var_value_valid().
     *
     * @return int
     */
    int get_var_value() { return type == SymbolType::VARIABLE ? _value : 0; }

    /**
     * @brief Set the variable value of the symbol. This will make the value valid.
     *
     * @param value
     */
    void set_var_value(int value)
    {
        _value = value;
        _value_valid = true;
    }

    /**
     * @brief Get if the variable value is valid.
     *
     * @return int
     */
    int get_var_value_valid() { return type == SymbolType::VARIABLE ? _value_valid : false; }

    /**
     * @brief Invalidate the variable value.
     *
     */
    void invalidate_var_value() { _value_valid = false; }

private:
    bool _has_param = false;
    bool _has_return = false;

    int _value = 0;
    bool _value_valid = false;
};

using SymbolPtr = std::shared_ptr<Symbol>;

CLOSE_SEMANALYZER_NAMESPACE

#endif