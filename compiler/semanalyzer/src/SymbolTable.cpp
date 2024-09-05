#include "SymbolTable.h"
#include "ScopeManager.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

bool SymbolTable::add_symbol(const std::string &scope_id, SymbolPtr symbol)
{
    auto &scope = _scopes[scope_id];
    if (scope.contains(symbol->name)) {
        return false;
    }

    scope[symbol->name] = symbol;
    return true;
}

bool SymbolTable::lookup_symbol(const std::string &scope_id, const std::string &name, SymbolPtr symbol_out)
{
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

CLOSE_SEMANALYZER_NAMESPACE
// end
