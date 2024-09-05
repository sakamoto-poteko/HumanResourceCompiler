#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

enum class SymbolType {
    VARIABLE,
    SUBROUTINE,
};

struct Symbol {
    SymbolType type;
    std::string name;

    Symbol(SymbolType type, const std::string &name)
        : type(type)
        , name(name)
    {
    }

    Symbol(SymbolType type, StringPtr name)
        : type(type)
        , name(*name)
    {
    }
};

using SymbolPtr = std::shared_ptr<Symbol>;

class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable();

    /// @brief false if conflict
    bool add_symbol(const std::string &scope_id, SymbolPtr symbol);

    bool lookup_symbol(const std::string &scope_id, const std::string &name, SymbolPtr symbol_out);

private:
    // map<scope id, hash<symbol name, symbol>>
    std::map<std::string, std::unordered_map<std::string, SymbolPtr>> _scopes;
};

CLOSE_SEMANALYZER_NAMESPACE

#endif