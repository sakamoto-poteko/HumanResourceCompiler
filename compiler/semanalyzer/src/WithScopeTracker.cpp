#include <cassert>
#include <stack>
#include <string>

#include "ScopeManager.h"
#include "WithScopeTracker.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

WithScopeTracker::WithScopeTracker()
{
    clear_scope_tracker();
}

WithScopeTracker::~WithScopeTracker()
{
}

void WithScopeTracker::track_scope_node_enter(const parser::ASTNodePtr &node)
{
    auto node_scope = ScopeInfoAttribute::get_scope(node)->get_scope_id();
    check_scope_enter(node, node_scope);
}

void WithScopeTracker::track_scope_node_leave(const parser::ASTNodePtr &node)
{
    // auto node_scope = ScopeInfoAttribute::get_scope(node)->get_scope_id();
    auto current_scope = get_current_scope_id();
    check_scope_leave(node, current_scope);
}

void WithScopeTracker::clear_scope_tracker()
{
    std::stack<std::string> empty;
    empty.push("");
    empty.swap(_scope_tracker);
}

const std::string &WithScopeTracker::get_current_scope_id() const
{
    return _scope_tracker.top();
}

void WithScopeTracker::track_scope_enter_manually(const parser::ASTNodePtr &node)
{
    track_scope_node_enter(node);
}

void WithScopeTracker::track_scope_leave_manually(const parser::ASTNodePtr &node)
{
    track_scope_node_leave(node);
}

void WithScopeTracker::track_scope_enter_manually(const parser::ASTNodePtr &node, const std::string &scope_id)
{
    // Invalid scope entered: not a children scope nor the same
    assert(scope_id.size() >= _scope_tracker.top().size());
    check_scope_enter(node, scope_id);
}

void hrl::semanalyzer::WithScopeTracker::check_scope_enter(const parser::ASTNodePtr &node, const std::string scope_id)
{
    auto current_scope = scope_id;
    auto last_scope = _scope_tracker.top();
    _scope_tracker.push(current_scope);
    if (current_scope.size() > last_scope.size()) {
        on_scope_enter(node, current_scope);
    }
}

void hrl::semanalyzer::WithScopeTracker::check_scope_leave(const parser::ASTNodePtr &node, const std::string scope_id)
{
    _scope_tracker.pop();
    auto last_scope = _scope_tracker.top();
    if (scope_id.size() > last_scope.size()) {
        on_scope_exit(node, scope_id);
    }
}

CLOSE_SEMANALYZER_NAMESPACE
// end
