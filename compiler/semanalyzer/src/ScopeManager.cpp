#include <string>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>

#include "ScopeManager.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

ScopeManager::ScopeManager()
{
    enter_scope("glb");
}

ScopeManager::~ScopeManager()
{
}

std::string ScopeManager::get_current_scope_id() const
{
    static const char delimiter[] = { DELIMITER, 0 };
    return boost::join(_scope_names, delimiter);
}

bool ScopeManager::enter_scope(const std::string &name)
{
    _scope_names.push_back(name);
    auto scope_id = get_current_scope_id();

    if (_all_scopes.contains(scope_id)) {
        _scope_names.pop_back();
        return false;
    }

    _all_scopes.insert(scope_id);
    _scope_id.push(0);
    return true;
}

bool ScopeManager::enter_scope(StringPtr name)
{
    return enter_scope(*name);
}

void ScopeManager::enter_anonymous_scope()
{
    int id = _scope_id.top();
    _scope_id.top() += 1;

    bool ok = enter_scope(std::to_string(id));
    UNUSED(ok);
    assert(ok); // not supposed to happen
}

void ScopeManager::exit_scope()
{
    _scope_names.pop_back();
    _scope_id.pop();
}

std::vector<std::string> ScopeManager::get_ancestor_scopes(const std::string &scope_id)
{
    std::vector<std::string> result;

    std::vector<std::string> parts;
    boost::split(parts, scope_id, [](char c) { return c == '.'; });

    std::string separator(1, DELIMITER);
    for (std::size_t i = parts.size(); i > 0; --i) {
        result.push_back(boost::algorithm::join(std::vector<std::string>(parts.begin(), parts.begin() + i), separator));
    }

    result.push_back("");
    return result;
}

int ScopeInfoAttribute::get_type()
{
    return SemAnalzyerASTNodeAttributeId::ATTR_SEMANALYZER_SYMBOL;
}

std::string ScopeInfoAttribute::to_string()
{
    return "scope: " + _scope_id;
}

CLOSE_SEMANALYZER_NAMESPACE
// end
