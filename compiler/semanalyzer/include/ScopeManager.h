#ifndef SCOPEMANAGER_H
#define SCOPEMANAGER_H

#include <memory>
#include <set>
#include <stack>
#include <string>
#include <vector>

#include "ASTNode.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

class ScopeInfoAttribute : public parser::ASTNodeAttribute {
public:
    ScopeInfoAttribute(const std::string scope_name)
        : _scope_name(scope_name)
    {
    }

    ~ScopeInfoAttribute() override = default;

private:
    std::string _scope_name;
};

using ScopeInfoAttributePtr = std::shared_ptr<ScopeInfoAttribute>;

class ScopeManager {
public:
    ScopeManager();
    ~ScopeManager();

    std::string get_scope_id() const;

    /// @brief get all the ancestor scope ids of scope_id
    /// suppose scope_id is a.b.c.d, the function returns { "a.b.c.d", "a.b.c", "a.b", "a" }
    static std::vector<std::string> get_ancestor_scopes(const std::string &scope_id);

    /// @brief return false if conflict
    bool enter_scope(const std::string &name);
    /// @brief return false if conflict
    bool enter_anonymous_scope();

    void exit_scope();

    static const char DELIMITER = '.';

private:
    std::vector<std::string> _scope_names;
    std::stack<int> _scope_id;
    std::set<std::string> _all_scopes;
};

CLOSE_SEMANALYZER_NAMESPACE

#endif
