#include <memory>
#include <string>

#include <boost/format.hpp>

#include "ScopeManager.h"
#include "SymbolTable.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

SymbolTable::SymbolTable()
{
    create_library_symbols();
}

bool SymbolTable::add_symbol(const std::string &scope_id, SymbolPtr symbol)
{
    auto &scope = _scopes[scope_id];
    if (scope.contains(symbol->name)) {
        return false;
    }

    scope[symbol->name] = symbol;
    return true;
}

bool SymbolTable::add_function_symbol(const std::string &scope_id, StringPtr function_name, bool has_param, bool has_return, StringPtr filename, const parser::ASTNodePtr &definition)
{
    auto symbol = std::make_shared<Symbol>(SymbolType::SUBROUTINE, *function_name, filename, SHARED_TO_WEAK(definition));
    symbol->set_param(has_param);
    symbol->set_return(has_return);
    return add_symbol(scope_id, symbol);
}

bool SymbolTable::add_variable_symbol(const std::string &scope_id, StringPtr variable_name, StringPtr filename, const parser::ASTNodePtr &definition)
{
    auto symbol = std::make_shared<Symbol>(SymbolType::VARIABLE, *variable_name, filename, SHARED_TO_WEAK(definition));
    return add_symbol(scope_id, symbol);
}

bool SymbolTable::lookup_symbol(const std::string &scope_id, const std::string &name, bool lookup_ancestors, SymbolPtr &symbol_out)
{
    if (!lookup_ancestors) {
        auto &scope = _scopes[scope_id];
        auto it = scope.find(name);
        if (it == scope.end()) {
            return false;
        } else {
            symbol_out = it->second;
            return true;
        }
    } else {
        auto ancestors = ScopeManager::get_ancestor_scopes(scope_id);
        for (auto &current_scope_id : ancestors) {
            auto &scope = _scopes[current_scope_id];
            auto it = scope.find(name);

            if (it != scope.end()) {
                symbol_out = it->second;
                return true;
            }
        }

        // looked up everything but there's no
        return false;
    }
}

bool SymbolTable::lookup_symbol(const std::string &scope_id, const StringPtr &name, bool lookup_ancestors, SymbolPtr &symbol_out)
{
    return lookup_symbol(scope_id, *name, lookup_ancestors, symbol_out);
}

void SymbolTable::create_library_symbols()
{
    StringPtr outbox = std::make_shared<std::string>("outbox");
    StringPtr inbox = std::make_shared<std::string>("inbox");
    StringPtr libfile = std::make_shared<std::string>("@stdlib");
    // absolutely topmost scope
    add_function_symbol("", outbox, true, false, libfile, nullptr);
    add_function_symbol("", inbox, false, true, libfile, nullptr);
}

CLOSE_SEMANALYZER_NAMESPACE
// end
