#ifndef WITHSCOPETRACKER_H
#define WITHSCOPETRACKER_H

#include <stack>
#include <string>

#include "ASTNodeForward.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

class WithScopeTracker {
public:
    WithScopeTracker();
    virtual ~WithScopeTracker();

protected:
    virtual void on_scope_enter(const parser::ASTNodePtr &node, const std::string &scope_id) = 0;
    virtual void on_scope_exit(const parser::ASTNodePtr &node, const std::string &scope_id) = 0;

    void track_scope_node_enter(const parser::ASTNodePtr &node);
    void track_scope_node_leave(const parser::ASTNodePtr &node);

    void track_scope_enter_manually(const parser::ASTNodePtr &node, const std::string &scope_id);
    void track_scope_enter_manually(const parser::ASTNodePtr &node);
    void track_scope_leave_manually(const parser::ASTNodePtr &node);

    const std::string &get_current_scope_id() const;

    void clear_scope_tracker();

private:
    void check_scope_enter(const parser::ASTNodePtr &node, const std::string scope_id);
    void check_scope_leave(const parser::ASTNodePtr &node, const std::string scope_id);

    std::stack<std::string> _scope_tracker;
};

CLOSE_SEMANALYZER_NAMESPACE

#endif