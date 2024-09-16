#include <iterator>
#include <ranges>
#include <string>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <spdlog/spdlog.h>

#include "ScopeManager.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

ScopeManager::ScopeManager()
{
    enter_scope("glb", ScopeType::Global);
}

ScopeManager::~ScopeManager()
{
}

std::string ScopeManager::get_current_scope_id() const
{
    static const char delimiter[] = { DELIMITER, 0 };
    std::string result;

    for (auto it = _current_scopes.begin(); it != _current_scopes.end(); ++it) {
        if (std::next(it) == _current_scopes.end()) {
            break;
        }

        result += it->first;
        result += delimiter;
    }

    result += _current_scopes.back().first;

    return result;
}

bool ScopeManager::enter_scope(const std::string &name, ScopeType scope_type)
{
    _current_scopes.push_back(std::make_pair(name, scope_type));
    auto scope_id = get_current_scope_id();

    if (_all_scopes.contains(scope_id)) {
        _current_scopes.pop_back();
        return false;
    }

    _all_scopes.insert(scope_id);
    _scope_id.push(0);
    return true;
}

bool ScopeManager::enter_scope(StringPtr name, ScopeType scope_type)
{
    return enter_scope(*name, scope_type);
}

void ScopeManager::enter_anonymous_scope()
{
    int id = _scope_id.top();
    _scope_id.top() += 1;

    bool ok = enter_scope(std::to_string(id), ScopeType::Block);
    UNUSED(ok);
    assert(ok); // not supposed to happen
}

void ScopeManager::exit_scope()
{
    _current_scopes.pop_back();
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

std::string ScopeInfoAttribute::to_string()
{
    switch (_type) {
    case ScopeType::Global:
        return "scope: [global]";
    case ScopeType::Subroutine:
        return "scope: [sub]" + _scope_id;
    case ScopeType::Block:
        return "scope: [blk]" + _scope_id;
    }
    // not supposed to happen
    spdlog::critical("unrecognized scope type: {}. {}", static_cast<int>(_type), __PRETTY_FUNCTION__);

    throw;
}

CLOSE_SEMANALYZER_NAMESPACE
// end
