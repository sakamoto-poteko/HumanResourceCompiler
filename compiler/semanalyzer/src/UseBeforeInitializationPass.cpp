#include "UseBeforeInitializationPass.h"
#include "ASTNodeForward.h"
#include "ErrorManager.h"
#include "ScopeManager.h"
#include "SemanticAnalysisErrors.h"
#include "semanalyzer_global.h"

#include <cassert>
#include <string>

OPEN_SEMANALYZER_NAMESPACE

void UseBeforeInitializationPass::log_use_before_initialization_error(const StringPtr &name, const parser::ASTNodePtr &node)
{
    SymbolPtr symbol_out;
    std::string defined_scope_out;
    std::string scope_id = ScopeInfoAttribute::get_scope(node)->get_scope_id();

    bool symbol_found = _symbol_table->lookup_symbol(scope_id, name, true, symbol_out, defined_scope_out);
    UNUSED(symbol_found);
    assert(symbol_found); // won't happen

    parser::ASTNodePtr defined_node = WEAK_TO_SHARED(symbol_out->definition);
    assert(defined_node);
    assert(symbol_out->type == SymbolType::VARIABLE);

    auto errstr = boost::format("Variable '%1%' may be used before assignment.") % *name;

    ErrorManager::instance().report(
        E_SEMA_VAR_USE_BEFORE_INIT,
        ErrorSeverity::Error,
        ErrorLocation(*_filename, node->lineno(), node->colno(), name->size()),
        errstr.str());

    ErrorManager::instance().report_continued(
        ErrorSeverity::Error,
        ErrorLocation(symbol_out->filename, defined_node->lineno(), defined_node->colno(), 0),
        "Original defined in");
}

bool UseBeforeInitializationPass::lookup_symbol_with_ancestors(const parser::ASTNodePtr &node, const StringPtr &name, SymbolPtr &out_symbol, std::string &out_def_scope)
{
    auto scope = ScopeInfoAttribute::get_scope(node)->get_scope_id();
    return _symbol_table->lookup_symbol(scope, name, true, out_symbol, out_def_scope);
}

bool UseBeforeInitializationPass::lookup_symbol_with_ancestors(const parser::ASTNodePtr &node, const StringPtr &name, SymbolPtr &out_symbol)
{
    std::string _;
    return lookup_symbol_with_ancestors(node, name, out_symbol, _);
}

int UseBeforeInitializationPass::get_varinit_record(const StringPtr &var_name, const parser::ASTNodePtr &node_scope_level)
{
    SymbolPtr symbol;
    std::string sym_defined_scope;
    assert(lookup_symbol_with_ancestors(node_scope_level, var_name, symbol, sym_defined_scope));

    auto &stack = _varinit_record_stacks[SymbolScopeKey(var_name, sym_defined_scope)];
    assert(!stack.empty());
    return stack.top();
}

void UseBeforeInitializationPass::create_varinit_record(const StringPtr &var_name, int is_initialized, const parser::ASTNodePtr &node_scope_level)
{
    SymbolPtr symbol;
    std::string sym_defined_scope;
    assert(lookup_symbol_with_ancestors(node_scope_level, var_name, symbol, sym_defined_scope));

    auto &stack = _varinit_record_stacks[SymbolScopeKey(var_name, sym_defined_scope)];
    assert(stack.empty());
    stack.push(is_initialized);
}

void UseBeforeInitializationPass::set_varinit_record(const SymbolScopeKey &key, int is_initialized, const parser::ASTNodePtr &node_scope_level)
{
    // NOTE:
    // 1) when variable is declared in same or outer scope, the symstack is pushed at that time. we can modify the top
    // 2) when variable is declared in inner scope, the symstack doesn't have it, and we need to push
    auto &stack = _varinit_record_stacks[key];
    if (stack.empty()) {
        stack.push(is_initialized);
    } else {
        stack.top() = is_initialized;
    }
}

void UseBeforeInitializationPass::set_varinit_record(const StringPtr &var_name, int is_initialized, const parser::ASTNodePtr &node_scope_level)
{
    SymbolPtr symbol;
    std::string sym_defined_scope;
    assert(lookup_symbol_with_ancestors(node_scope_level, var_name, symbol, sym_defined_scope));
    set_varinit_record(SymbolScopeKey(var_name, sym_defined_scope), is_initialized, node_scope_level);
}

void UseBeforeInitializationPass::enter_scope_varinit_record(const parser::ASTNodePtr &node_scope_level)
{
    // 1. get all current scope variables (include ancestor defined)
    // 2. for each variable, look at the top of stack result
    // 3. push a new value=stack.top. The stack should not be empty - the variable must be declared before use
    std::vector<std::pair<SymbolPtr, std::string>> all_symbols;
    auto scope_id = _scope_manager.get_current_scope_id();
    _symbol_table->get_symbols_include_ancestors(scope_id, all_symbols);
    for (const auto &[symbol, sym_defined_scope] : all_symbols) {
        if (symbol->type == SymbolType::VARIABLE) {
            // NOTE: symstack is not empty, because variable is declared first. the symstack is pushed at that time.
            auto &stack = _varinit_record_stacks[SymbolScopeKey(std::make_shared<std::string>(symbol->name), sym_defined_scope)];
            assert(!stack.empty());
            int top = stack.top();
            stack.push(top);
        }
    }
    _varinit_record_stack_result.emplace();
}

void UseBeforeInitializationPass::leave_scope_varinit_record()
{
    SymbolScopedKeyValueHash result;

    // note the order. we're actually writing the result to parent's result stack slot
    // BUG: this is not right. When there are multiple scopes, it fails.
    // we should figure out a way to passthrough children's result to parent
    const auto &children_results = _varinit_record_stack_result.top();

    std::vector<std::pair<SymbolPtr, std::string>> all_symbols;
    auto scope_id = _scope_manager.get_current_scope_id();
    _symbol_table->get_symbols_include_ancestors(scope_id, all_symbols);

    for (const auto &[symbol, sym_defined_scope] : all_symbols) {
        if (symbol->type == SymbolType::VARIABLE) {
            auto symkey = SymbolScopeKey(std::make_shared<std::string>(symbol->name), sym_defined_scope);
            auto &stack = _varinit_record_stacks[symkey];
            // FIXME: we need to return this result
            // we also need to filter out those who has the scope_id of self,
            // because our ancestors don't care inner scoped vars! they can't access anyway.
            assert(!stack.empty());
            int sym_init_result = stack.top();
            stack.pop();
            if (sym_defined_scope != scope_id) {
                result[symkey] = sym_init_result;
            }
        }
    }

    for (const auto &[children_symkey, children_init_result] : children_results) {
        result[children_symkey] = children_init_result;
    }

    _varinit_record_stack_result.pop();
    _varinit_record_stack_result.top().swap(result);
}

void UseBeforeInitializationPass::get_child_varinit_records(SymbolScopedKeyValueHash &result)
{
    result = _varinit_record_stack_result.top();
}

void UseBeforeInitializationPass::set_child_varinit_records(const SymbolScopedKeyValueHash &records)
{
    _varinit_record_stack_result.top() = records;
}

CLOSE_SEMANALYZER_NAMESPACE
// end
