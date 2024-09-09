#ifndef SCOPEMANAGER_H
#define SCOPEMANAGER_H

#include <memory>
#include <set>
#include <stack>
#include <string>
#include <vector>

#include "ASTNode.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

enum class ScopeType : int {
    /**
     * @brief The global scope, which is at the root of compilation unit.
     *
     */
    Global,
    /**
     * @brief The subroutine scope, which is at the root of subroutine.
     *
     */
    Subroutine,
    /**
     * @brief The block scope.
     *
     */
    Block,
};

/**
 * @brief Stores information about the scope of an AST node.
 *
 * The ScopeInfoAttribute class holds metadata about an AST node's scope,
 * including the scope identifier and the type of the scope (Global, Subroutine, or Block).
 * This information is used during semantic analysis to track the scope of variables
 * and ensure that scoping rules are followed correctly.
 */
class ScopeInfoAttribute : public parser::ASTNodeAttribute {
public:
    ScopeInfoAttribute(const std::string &scope_id, ScopeType scope_type)
        : _scope_id(scope_id)
        , _type(scope_type)
    {
    }

    ~ScopeInfoAttribute() override = default;

    int get_type() override;
    std::string to_string() override;

    const std::string &get_scope_id() const { return _scope_id; }

    ScopeType get_scope_type() const { return _type; }

private:
    std::string _scope_id;
    ScopeType _type;
};

using ScopeInfoAttributePtr = std::shared_ptr<ScopeInfoAttribute>;

/**
 * @brief Manages scoping during compilation.
 *
 * The ScopeManager class handles the scoping of variables in global/function/block
 * level, tracking the current scope and its type during the compilation process. It
 * facilitates entering named or anonymous scopes, exiting scopes, and checking for
 * conflicts in scope names. Additionally, it provides utility functions to retrieve
 * the current scope's ID, type, and ancestor scopes.
 *
 */
class ScopeManager {
public:
    ScopeManager();
    ~ScopeManager();

    std::string get_current_scope_id() const;

    ScopeType get_current_scope_type() const { return _current_scopes.back().second; }

    /**
     * @brief Get the ancestor scopes of \p scope_id.
     *
     * @param scope_id
     * @return std::vector<std::string> Suppose scope_id is a.b.c.d, the function returns { "a.b.c.d", "a.b.c", "a.b", "a", "" }
     */
    static std::vector<std::string> get_ancestor_scopes(const std::string &scope_id);

    /**
     * @brief Enter a scope with \p name.
     *
     * @param name The name of the current scope
     * @return true Successfully entered the scope
     * @return false There is a conflict in the scope name
     */
    bool enter_scope(const std::string &name, ScopeType scope_type);

    /**
     * @copydoc enter_scope(const std::string &name, ScopeType scope_type)
     */
    bool enter_scope(StringPtr name, ScopeType scope_type);

    /**
     * @brief Enter a scope without a name. Usually this is a block.
     *
     * @return true Successfully enetered the scope
     */
    void enter_anonymous_scope();

    /**
     * @brief Exit the scope.
     *
     */
    void exit_scope();

    static const char DELIMITER = '.';

private:
    std::vector<std::pair<std::string, ScopeType>> _current_scopes;
    std::stack<int> _scope_id;
    std::set<std::string> _all_scopes;
};

CLOSE_SEMANALYZER_NAMESPACE

#endif
