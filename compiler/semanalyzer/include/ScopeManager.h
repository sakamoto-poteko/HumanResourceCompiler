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
    Global,
    Subroutine,
    Block,
};

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

private:
    std::string _scope_id;
    ScopeType _type;
};

using ScopeInfoAttributePtr = std::shared_ptr<ScopeInfoAttribute>;

class ScopeManager {
public:
    ScopeManager();
    ~ScopeManager();

    std::string get_current_scope_id() const;

    ScopeType get_current_scope_type() const { return _current_scopes.back().second; }

    /**
     * @brief Get the ancestor scopes of \p scope_id
     *
     * @param scope_id
     * @return std::vector<std::string> Suppose scope_id is a.b.c.d, the function returns { "a.b.c.d", "a.b.c", "a.b", "a", "" }
     */
    static std::vector<std::string> get_ancestor_scopes(const std::string &scope_id);

    /**
     * @brief Enter a scope with \p name
     *
     * @param name The name of the current scope
     * @return true Successfully entered the scope
     * @return false There is a conflict in the scope name
     */
    bool enter_scope(const std::string &name, ScopeType scope_type);

    bool enter_scope(StringPtr name, ScopeType scope_type);

    /**
     * @brief Enter a scope without a name
     *
     * @return true Successfully netered the scope
     */
    void enter_anonymous_scope();

    void exit_scope();

    static const char DELIMITER = '.';

private:
    std::vector<std::pair<std::string, ScopeType>> _current_scopes;
    std::stack<int> _scope_id;
    std::set<std::string> _all_scopes;
};

CLOSE_SEMANALYZER_NAMESPACE

#endif
