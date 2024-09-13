#include <cassert>
#include <string>

#include "ASTNodeForward.h"
#include "ErrorManager.h"
#include "ScopeManager.h"
#include "SemanticAnalysisErrors.h"
#include "SemanticAnalysisPass.h"
#include "Symbol.h"
#include "UseBeforeInitializationCheckPass.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

#define BEGIN_VISIT() \
    enter_node(node); \
    int rc = 0;       \
    UNUSED(rc);

#define END_VISIT() \
    leave_node();   \
    return 0;

#define RETURN_IF_FAIL_IN_VISIT(rc) \
    if (rc != 0) {                  \
        leave_node();               \
        return rc;                  \
    }

int UseBeforeInitializationCheckPass::run()
{
    init_symbol_table();
    clear_scope_tracker();
    return _root->accept(this);
}

int UseBeforeInitializationCheckPass::visit(parser::DecrementExpressionASTNodePtr node)
{
    BEGIN_VISIT();

    rc = check_node_symbol_assigned_or_report(node);
    RETURN_IF_FAIL_IN_VISIT(rc);

    END_VISIT();
}

int UseBeforeInitializationCheckPass::visit(parser::IncrementExpressionASTNodePtr node)
{
    BEGIN_VISIT();

    rc = check_node_symbol_assigned_or_report(node);
    RETURN_IF_FAIL_IN_VISIT(rc);

    END_VISIT();
}

int UseBeforeInitializationCheckPass::visit(parser::VariableDeclarationASTNodePtr node)
{
    BEGIN_VISIT();
    const auto &symbol = Symbol::get_from(node);
    create_var_init_at_current_scope(symbol);

    rc = traverse(node->get_assignment());
    RETURN_IF_FAIL_IN_VISIT(rc);

    END_VISIT();
}

int UseBeforeInitializationCheckPass::visit(parser::IfStatementASTNodePtr node)
{
    enter_node(node);

    NodeResult then_result, else_result, merged_result;
    auto &then_branch = node->get_then_branch();
    int rc_then = traverse(then_branch);
    RETURN_IF_FAIL_IN_VISIT(rc_then);
    get_var_init_result(then_branch, then_result);

    auto &else_branch = node->get_else_branch();
    if (else_branch) {
        int rc_else = traverse(else_branch);
        RETURN_IF_FAIL_IN_VISIT(rc_else);
        get_var_init_result(else_branch, else_result);
    }

    // process the var init results. if both are 1, the final is 1. if neither is 0, the final is 0.
    // they may not share the same size. one may define more vars and returned
    for (const auto &[symbol, _] : then_result) {
        merged_result[symbol] = 0;
    }

    for (const auto &[symbol, _] : else_result) {
        merged_result[symbol] = 0;
    }

    for (auto &[symbol, value] : merged_result) {
        int then_is_initialized = then_result[symbol];
        int else_is_initialized = else_result[symbol];

        if (then_is_initialized && else_is_initialized) {
            value = 1;
        }
    }

    strip_symbols_beyond_scope(merged_result, get_current_scope_id());
    set_var_init_at_current_scope(merged_result);

    leave_node();
    return 0;
}

int UseBeforeInitializationCheckPass::visit(parser::WhileStatementASTNodePtr node)
{
    BEGIN_VISIT();

    // There won't be var decl or asgn in cond
    rc = traverse(node->get_condition());
    RETURN_IF_FAIL_IN_VISIT(rc);

    NodeResult preloop_result; // assume the loop isn't executed
    get_var_init_result(node, preloop_result);

    rc = traverse(node->get_body());
    RETURN_IF_FAIL_IN_VISIT(rc);

    set_var_init_at_current_scope(preloop_result);

    END_VISIT();
}

int UseBeforeInitializationCheckPass::visit(parser::ForStatementASTNodePtr node)
{
    BEGIN_VISIT();

    NodeResult preloop_result; // assume the loop isn't executed
    get_var_init_result(node, preloop_result);

    NodeResult for_stage_results = preloop_result; // we collect the result after each for stage, and pass it through to the loop

    auto &init_node = node->get_init();
    auto &cond_node = node->get_condition();
    auto &update_node = node->get_update();
    auto &body_node = node->get_body();

    // ensure for stmts share the same scope, so it doesn't pop between init, cond, update and body
    // body is not nullable
    track_scope_enter_manually(node, ScopeInfoAttribute::get_scope(body_node)->get_scope_id());

    if (init_node) {
        rc = traverse(init_node);
        if (rc != 0) {
            track_scope_leave_manually(node);
            RETURN_IF_FAIL_IN_VISIT(rc);
        }
    }

    if (cond_node) {
        rc = traverse(cond_node);
        if (rc != 0) {
            track_scope_leave_manually(node);
            RETURN_IF_FAIL_IN_VISIT(rc);
        }
        // There won't be var decl or asgn in cond
    }

    if (update_node) {
        rc = traverse(update_node);
        if (rc != 0) {
            track_scope_leave_manually(node);
            RETURN_IF_FAIL_IN_VISIT(rc);
        }
        // There won't be var decl or asgn in update
    }

    if (body_node) {
        rc = traverse(body_node);
        if (rc != 0) {
            track_scope_leave_manually(node);
            RETURN_IF_FAIL_IN_VISIT(rc);
        }
    }

    track_scope_leave_manually(node);

    // set back to pre loop
    set_var_init_at_current_scope(preloop_result);

    END_VISIT();
}

int UseBeforeInitializationCheckPass::visit(parser::VariableAssignmentASTNodePtr node)
{
    BEGIN_VISIT();

    rc = traverse(node->get_value());
    RETURN_IF_FAIL_IN_VISIT(rc);

    SymbolPtr symbol = Symbol::get_from(node);
    set_var_init_at_current_scope(symbol, 1);

    END_VISIT();
}

int UseBeforeInitializationCheckPass::visit(parser::VariableAccessASTNodePtr node)
{
    BEGIN_VISIT();

    rc = check_node_symbol_assigned_or_report(node);
    RETURN_IF_FAIL_IN_VISIT(rc);

    END_VISIT();
}

int UseBeforeInitializationCheckPass::visit(parser::StatementBlockASTNodePtr node)
{
    BEGIN_VISIT();

    // get the result of traversed node, and put to our own scope
    auto passthrough_result = [this](const parser::ASTNodePtr &nodeparam) {
        // these are the nodes with statement blocks
        // can't be FunctionDefinitionASTNode nor SubprocDefinitionASTNode because we're in a statement block
        bool visit_defined_with_statement_block
            = is_ptr_type<parser::WhileStatementASTNode>(nodeparam)
            || is_ptr_type<parser::IfStatementASTNode>(nodeparam)
            || is_ptr_type<parser::ForStatementASTNode>(nodeparam);

        // we skip this for those defined their own visits
        if (!visit_defined_with_statement_block) {
            NodeResult result;
            get_var_init_result(nodeparam, result);
            strip_symbols_beyond_scope(result, get_current_scope_id());
            set_var_init_at_current_scope(result);
        }
    };

    rc = traverse(node->get_statements(), passthrough_result);
    RETURN_IF_FAIL_IN_VISIT(rc);

    END_VISIT();
}

int UseBeforeInitializationCheckPass::visit(parser::SubprocDefinitionASTNodePtr node)
{
    return visit_subroutine(node);
}

int UseBeforeInitializationCheckPass::visit(parser::FunctionDefinitionASTNodePtr node)
{
    return visit_subroutine(node);
}

int UseBeforeInitializationCheckPass::visit_subroutine(parser::AbstractSubroutineASTNodePtr node)
{
    BEGIN_VISIT();

    NodeResult prefunc_result; // assume the function isn't executed
    get_var_init_result(node, prefunc_result);

    rc = traverse_multiple(node->get_parameter(), node->get_body());
    RETURN_IF_FAIL_IN_VISIT(rc);

    set_var_init_at_current_scope(prefunc_result);

    END_VISIT();
}

int UseBeforeInitializationCheckPass::check_node_symbol_assigned_or_report(const parser::ASTNodePtr &node)
{
    SymbolPtr symbol = Symbol::get_from(node);
    assert(symbol);

    int assigned = get_var_init_at_current_scope(symbol);
    if (!assigned) {
        log_use_before_initialization_error(symbol, node);
        return E_SEMA_VAR_USE_BEFORE_INIT;
    }

    return 0;
}

void UseBeforeInitializationCheckPass::on_scope_enter(const parser::ASTNodePtr &node, const std::string &scope_id)
{
    UNUSED(node);
    UNUSED(scope_id);
    // copy all elements from parent's scope
    std::set<SymbolPtr> occured_vars;
    for (auto &[symbol, stack] : _varinit_record_stacks) {
        // Did we see this symbol in our parent scope? If not, we skip this one (defined in other scopes)
        if (!_var_occured.top().contains(symbol)) {
            continue;
        }
        assert(!stack.empty());
        stack.push(stack.top());
        occured_vars.insert(symbol);
    }
    _var_occured.push(occured_vars);
}

void UseBeforeInitializationCheckPass::on_scope_exit(const parser::ASTNodePtr &node, const std::string &current_scope_id)
{
    UNUSED(current_scope_id);
    // return result to parent's scope
    NodeResult result;
    for (auto &[symbol, stack] : _varinit_record_stacks) {
        // record stacks has all symbols. lookup only what we see in current scope
        if (!_var_occured.top().contains(symbol)) {
            continue;
        }
        assert(!stack.empty());
        int initialized = stack.top();
        stack.pop();
        result[symbol] = initialized;
    }
    strip_symbols_beyond_scope(result, current_scope_id);
    _varinit_record_results[node] = result;

    _var_occured.pop();
}

void UseBeforeInitializationCheckPass::get_var_init_result(const parser::ASTNodePtr &node_to_get_result, NodeResult &result)
{
    for (const auto &[symbol, _] : result) {
        assert(_var_occured.top().contains(symbol));
    }
    result = _varinit_record_results[node_to_get_result];
}

void UseBeforeInitializationCheckPass::set_var_init_at_current_scope(const NodeResult &result)
{
    for (const auto &[sym_key, sym_initialized] : result) {
        set_var_init_at_current_scope(sym_key, sym_initialized);
    }
}

void UseBeforeInitializationCheckPass::create_var_init_at_current_scope(const SymbolPtr &symbol)
{
    assert(symbol);
    auto &stack = _varinit_record_stacks[symbol];
    assert(stack.empty());

    stack.push(0);
    _var_occured.top().insert(symbol);
}

void UseBeforeInitializationCheckPass::set_var_init_at_current_scope(const SymbolPtr &symbol, int initialized)
{
    assert(symbol);
    auto &stack = _varinit_record_stacks[symbol];
    assert(!stack.empty());
    stack.top() = initialized;
    _var_occured.top().insert(symbol);
}

int UseBeforeInitializationCheckPass::get_var_init_at_current_scope(const SymbolPtr &symbol)
{
    assert(symbol);
    assert(_var_occured.top().contains(symbol));
    auto &stack = _varinit_record_stacks[symbol];
    assert(!stack.empty());
    return stack.top();
}

void UseBeforeInitializationCheckPass::strip_symbols_beyond_scope(NodeResult &results, const std::string &scope_id)
{
    NodeResult stripped;
    for (const auto &[symbol, is_initialized] : results) {
        if (_symbol_table->is_symbol_in_scope(symbol, scope_id)) {
            stripped[symbol] = is_initialized;
        }
    }

    results.swap(stripped);
}

void UseBeforeInitializationCheckPass::log_use_before_initialization_error(const SymbolPtr &symbol, const parser::ASTNodePtr &node)
{
    assert(symbol);
    parser::ASTNodePtr defined_node = WEAK_TO_SHARED(symbol->definition);
    assert(defined_node);
    assert(symbol->type == SymbolType::VARIABLE);

    auto errstr = boost::format("Variable '%1%' may be used before assignment.") % symbol->name;

    ErrorManager::instance().report(
        E_SEMA_VAR_USE_BEFORE_INIT,
        ErrorSeverity::Error,
        ErrorLocation(*_filename, node->lineno(), node->colno(), symbol->name.size()),
        errstr.str());

    ErrorManager::instance().report_continued(
        ErrorSeverity::Error,
        ErrorLocation(symbol->filename, defined_node->lineno(), defined_node->colno(), 0),
        "Original defined in");
}

void UseBeforeInitializationCheckPass::enter_node(parser::ASTNodePtr node)
{
    SemanticAnalysisPass::enter_node(node);
    track_scope_node_enter(node);
}

void UseBeforeInitializationCheckPass::leave_node()
{
    const auto &current_node = topmost_node();

    track_scope_node_leave(current_node);
    SemanticAnalysisPass::leave_node();
}

CLOSE_SEMANALYZER_NAMESPACE

// end
