#ifndef SYMBOL_H
#define SYMBOL_H

#include "ASTNode.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

enum class SymbolType {
    VARIABLE,
    SUBROUTINE,
};

/**
 * @brief Represents a symbol in the program, including its type, name, and definition.
 *
 * The Symbol class stores information about a symbol in the program, such as variables,
 * functions, or subroutines.
 *
 * The class also tracks whether a function or subroutine has parameters and/or a return value.
 * It provides methods for retrieving the symbol's attributes, such as whether it has parameters
 * or a return value, and converts the symbol's details to a string format.
 */
class Symbol : public parser::ASTNodeAttribute {
public:
    /**
     * @brief The type of a symbol
     *
     */
    SymbolType type;

    /**
     * @brief The symbol name
     *
     */
    std::string name;

    /**
     * @brief The filename of which this symbol is defined.
     *
     */
    StringPtr filename;

    // It's weak to prevent circular reference.
    /**
     * @brief The weak pointer to original definition ASTNode
     *
     */
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

    /**
     * @brief Set the symbol (function/sub) has parameter
     *
     * @param has
     */
    void set_param(bool has) { _has_param = has; }

    /**
     * @brief Set the symbol (function/sub) has return value
     *
     * @param has
     */
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