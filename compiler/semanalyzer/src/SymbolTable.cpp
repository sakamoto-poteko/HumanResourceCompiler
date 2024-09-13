#include <memory>
#include <string>
#include <vector>

#include <boost/format.hpp>

#include "ScopeManager.h"
#include "Symbol.h"
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
    SymbolPtr out;
    return add_function_symbol(scope_id, function_name, has_param, has_return, filename, definition, out);
}

bool SymbolTable::add_function_symbol(const std::string &scope_id, StringPtr function_name, bool has_param, bool has_return, StringPtr filename, const parser::ASTNodePtr &definition, SymbolPtr &out)
{
    auto symbol = std::make_shared<Symbol>(SymbolType::SUBROUTINE, *function_name, filename, SHARED_TO_WEAK(definition));
    symbol->set_param(has_param);
    symbol->set_return(has_return);
    bool ok = add_symbol(scope_id, symbol);
    if (ok) {
        out = symbol;
        return true;
    } else {
        return false;
    }
}

bool SymbolTable::add_variable_symbol(const std::string &scope_id, StringPtr variable_name, StringPtr filename, const parser::ASTNodePtr &definition, SymbolPtr &out)
{
    auto symbol = std::make_shared<Symbol>(SymbolType::VARIABLE, *variable_name, filename, SHARED_TO_WEAK(definition));
    bool ok = add_symbol(scope_id, symbol);
    if (ok) {
        out = symbol;
        return true;
    } else {
        return false;
    }
}

bool SymbolTable::add_variable_symbol(const std::string &scope_id, StringPtr variable_name, StringPtr filename, const parser::ASTNodePtr &definition)
{
    SymbolPtr out;
    return add_variable_symbol(scope_id, variable_name, filename, definition, out);
}

bool SymbolTable::lookup_symbol(const std::string &scope_id, const std::string &name, bool lookup_ancestors, SymbolPtr &symbol_out, std::string &defined_scope_out)
{
    if (!lookup_ancestors) {
        auto &scope = _scopes[scope_id];
        auto it = scope.find(name);
        if (it == scope.end()) {
            return false;
        } else {
            symbol_out = it->second;
            defined_scope_out = scope_id;
            return true;
        }
    } else {
        auto ancestors = ScopeManager::get_ancestor_scopes(scope_id);
        for (auto &current_scope_id : ancestors) {
            auto &scope = _scopes[current_scope_id];
            auto it = scope.find(name);

            if (it != scope.end()) {
                symbol_out = it->second;
                defined_scope_out = current_scope_id;
                return true;
            }
        }

        // looked up everything but there's no
        return false;
    }
}

bool SymbolTable::lookup_symbol(const std::string &scope_id, const StringPtr &name, bool lookup_ancestors, SymbolPtr &symbol_out, std::string &defined_scope_out)
{
    return lookup_symbol(scope_id, *name, lookup_ancestors, symbol_out, defined_scope_out);
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

void hrl::semanalyzer::SymbolTable::get_symbols_include_ancestors(const std::string &scope_id, std::vector<std::pair<SymbolPtr, std::string>> &out)
{
    std::vector<std::pair<SymbolPtr, std::string>> result;

    for (const auto &[scope_id_of_symbol, symbols] : _scopes) {
        if (scope_id.starts_with(scope_id_of_symbol)) {
            for (const auto &[_, symbol] : symbols) {
                result.push_back(std::make_pair(symbol, scope_id_of_symbol));
            }
        }
    }

    out.swap(result);
}

void hrl::semanalyzer::SymbolTable::get_symbols_exclude_ancestors(const std::string &scope_id, std::vector<SymbolPtr> &out)
{
    std::vector<SymbolPtr> result;

    for (const auto &pair : _scopes) {
        const auto &id = pair.first;
        if (id == scope_id) {
            for (const auto &[_, symbol] : pair.second) {
                result.push_back(symbol);
            }
        }
    }

    out.swap(result);
}

bool SymbolTable::is_symbol_in_scope(const SymbolPtr &symbol, const std::string &scope_id)
{
    auto ancestors = ScopeManager::get_ancestor_scopes(scope_id);
    for (auto &current_scope_id : ancestors) {
        auto &scope = _scopes[current_scope_id];
        auto it = scope.find(symbol->name);

        // same name, then check if same ptr
        if (it != scope.end() && it->second == symbol) {
            return true;
        }
    }

    // looked up everything but there's no
    return false;
}

CLOSE_SEMANALYZER_NAMESPACE
// end
