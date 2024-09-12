#include "UseBeforeInitializationCheckPass.h"
#include "ASTNodeForward.h"
#include "ErrorManager.h"
#include "ScopeManager.h"
#include "SemanticAnalysisErrors.h"
#include "SemanticAnalysisPass.h"
#include "Symbol.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

#include <cassert>
#include <string>

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

void UseBeforeInitializationCheckPass::enter_node(parser::ASTNodePtr node)
{
    SemanticAnalysisPass::enter_node(node);
    update_varinit_record_stack_on_scope_change(node);
}

void UseBeforeInitializationCheckPass::leave_node()
{
    update_varinit_record_stack_on_scope_change(topmost_node());
    SemanticAnalysisPass::leave_node();
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
    merged_result = else_result;
    for (const auto &[sym_key, result_then] : then_result) {
        int result_else = else_result.at(sym_key);

        // Apply the merging rule: if either is 0, result is 0, otherwise result is 1
        int both_initialized = (result_then == 0 || result_else == 0) ? 0 : 1;

        // Insert the merged value into the result map
        merged_result[sym_key] = both_initialized;
    }

    set_var_init_at_current_scope(merged_result);

    leave_node();
    return 0;
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

void UseBeforeInitializationCheckPass::update_varinit_record_stack_on_scope_change(const parser::ASTNodePtr &node)
{
    const std::string &scope_id = ScopeInfoAttribute::get_scope(node)->get_scope_id();

    // >0: entered scope, <0: left scope, =0: same scope
    int scope_change = scope_id.size() - _last_scope_id.size();

    if (scope_change > 0) {
        // entered a scope
        // copy all elements from parent's scope
        _var_occured.emplace();
        for (auto &[symbol, stack] : _varinit_record_stacks) {
            assert(!stack.empty());
            stack.push(stack.top());
            _var_occured.top().insert(symbol);
        }
    } else if (scope_change < 0) {
        // left a scope
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
        strip_symbols_beyond_scope(result, node);
        _varinit_record_results[node] = result;
        _var_occured.pop();
    } else {
        // same scope. we do nothing
    }

    _last_scope_id = scope_id;
}

void UseBeforeInitializationCheckPass::strip_symbols_beyond_scope(NodeResult &results, const parser::ASTNodePtr &node)
{
    const auto &scope_id = ScopeInfoAttribute::get_scope(node)->get_scope_id();
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

int UseBeforeInitializationCheckPass::run()
{
    return _root->accept(this);
}

int UseBeforeInitializationCheckPass::visit(parser::WhileStatementASTNodePtr node)
{
    BEGIN_VISIT();

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

    rc = traverse(node->get_init());
    RETURN_IF_FAIL_IN_VISIT(rc);

    rc = traverse(node->get_condition());
    RETURN_IF_FAIL_IN_VISIT(rc);

    rc = traverse(node->get_update());
    RETURN_IF_FAIL_IN_VISIT(rc);

    rc = traverse(node->get_body());
    RETURN_IF_FAIL_IN_VISIT(rc);

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

    SymbolPtr symbol = Symbol::get_from(node);

    int assigned = get_var_init_at_current_scope(symbol);
    if (!assigned) {
        log_use_before_initialization_error(symbol, node);
        RETURN_IF_FAIL_IN_VISIT(E_SEMA_VAR_USE_BEFORE_INIT);
    }

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

CLOSE_SEMANALYZER_NAMESPACE
// end
