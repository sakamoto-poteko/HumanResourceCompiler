#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Symbol.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable() = default;

    /**
     * @brief Add \p symbol to the symbol table with \p scope_id
     *
     * @param scope_id The scope id
     * @param symbol The symbol to be added
     * @return true The symbol is successfully added
     * @return false The symbol failed to add. There is a conflict.
     */
    bool add_symbol(const std::string &scope_id, SymbolPtr symbol);

    /**
     * @brief Add a function to symbol table
     *
     * @param scope_id The scope id
     * @param function_name The function name
     * @param has_param If the function has the pramater
     * @param has_return If the function has return value
     * @param filename The file where the symbol was defined
     * @param definition The function definition ASTNode
     * @return true The symbol is successfully added
     * @return false The symbol failed to add. There is a conflict.
     */
    bool add_function_symbol(const std::string &scope_id, StringPtr function_name, bool has_param, bool has_return, StringPtr filename, const parser::ASTNodePtr &definition, SymbolPtr &out);
    bool add_function_symbol(const std::string &scope_id, StringPtr function_name, bool has_param, bool has_return, StringPtr filename, const parser::ASTNodePtr &definition);

    /**
     * @brief Add a variable to symbol table
     *
     * @param scope_id The scope id
     * @param variable_name The variable name
     * @param filename The file where the symbol was defined
     * @param definition The variable definition ASTNode
     * @return true The symbol is successfully added
     * @return false The symbol failed to add. There is a conflict.
     */
    bool add_variable_symbol(const std::string &scope_id, StringPtr variable_name, StringPtr filename, const parser::ASTNodePtr &definition, SymbolPtr &out);
    bool add_variable_symbol(const std::string &scope_id, StringPtr variable_name, StringPtr filename, const parser::ASTNodePtr &definition);

    /**
     * @brief Look up the symbol \p name in the symbol table.
     *
     * @param scope_id The current scope id
     * @param name The symbol name
     * @param lookup_ancestors True if look up the ancestor scopes of scope_id. False to look up only the \p scope_id
     * @param symbol_out [out] The result symbol
     * @param defined_scope_out [out] The symbol defined scope
     * @return true The symbol was found, and \p symbol_out is set
     * @return false The symbol was not found
     */
    bool lookup_symbol(const std::string &scope_id, const std::string &name, bool lookup_ancestors, SymbolPtr &symbol_out, std::string &defined_scope_out);
    bool lookup_symbol(const std::string &scope_id, const StringPtr &name, bool lookup_ancestors, SymbolPtr &symbol_out, std::string &defined_scope_out);

    // vector<pair<Symbol, sym_defined_scope>>
    void get_symbols_include_ancestors(const std::string &scope_id, std::vector<std::pair<SymbolPtr, std::string>> &out);
    void get_symbols_exclude_ancestors(const std::string &scope_id, std::vector<SymbolPtr> &out);

    // include parent scopes
    bool is_symbol_in_scope(const SymbolPtr &symbol, const std::string &scope_id);
    void strip_symbols_from_file(const StringPtr &filename);

    void clear_symbols();

private:
    // map<scope id, hash<symbol name, symbol>>
    std::map<std::string, std::unordered_map<std::string, SymbolPtr>> _scopes;

    /**
     * @brief Create outbox/inbox symbols
     *
     */
    void create_library_symbols();
};

using SymbolTablePtr = std::shared_ptr<SymbolTable>;

CLOSE_SEMANALYZER_NAMESPACE

#endif
