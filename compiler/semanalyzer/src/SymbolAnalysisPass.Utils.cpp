#include <cassert>
#include <memory>
#include <string>
#include <typeinfo>

#include <boost/format.hpp>

#include <spdlog/spdlog.h>
#include <utility>
#include <vector>

#include "ASTNode.h"
#include "ErrorManager.h"
#include "ErrorMessage.h"
#include "ScopeManager.h"
#include "SemanticAnalysisErrors.h"
#include "Symbol.h"
#include "SymbolAnalysisPass.h"
#include "SymbolTable.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

bool SymbolAnalysisPass::lookup_symbol_with_ancestors(const StringPtr &name, SymbolPtr &out_symbol, std::string &out_def_scope)
{
    return _symbol_table->lookup_symbol(_scope_manager.get_current_scope_id(), name, true, out_symbol, out_def_scope);
}

bool SymbolAnalysisPass::lookup_symbol_with_ancestors(const StringPtr &name, SymbolPtr &out_symbol)
{
    std::string _;
    return lookup_symbol_with_ancestors(name, out_symbol, _);
}

int SymbolAnalysisPass::attach_symbol_or_log_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node)
{
    // SymbolPtr symbol;
    // if (!lookup_symbol_with_ancestors(name, symbol)) {
    //     log_undefined_error(name, type, node);
    //     return E_SEMA_SYM_UNDEFINED;
    // } else {
    //     node->set_attribute(SemAnalzyerASTNodeAttributeId::ATTR_SEMANALYZER_SYMBOL, symbol);
    //     return 0;
    // }
    return 0;
}

int SymbolAnalysisPass::add_subroutine_symbol_or_log_error(const StringPtr &name, bool has_param, bool has_return, const ASTNodePtr &node)
{
    if (!_symbol_table->add_function_symbol(_scope_manager.get_current_scope_id(), name, has_param, has_return, _filename, node)) {
        log_redefinition_error(name, SymbolType::SUBROUTINE, node);
        return E_SEMA_SYM_REDEF;
    } else {
        return 0;
    }
    return 0;
}

int SymbolAnalysisPass::add_variable_symbol_or_log_error(const StringPtr &name, const ASTNodePtr &node)
{
    SymbolPtr symbol;
    // bool found_in_ancestor_or_current = lookup_symbol_with_ancestors(name, symbol);
    if (!_symbol_table->add_variable_symbol(_scope_manager.get_current_scope_id(), name, _filename, node)) {
        // false indicate found in current
        // log_redefinition_error(name, SymbolType::VARIABLE, node);
        return E_SEMA_SYM_REDEF;
    // } else if (found_in_ancestor_or_current) {
    //     // added to current scope and ancestor has it
    //     auto original_node = WEAK_TO_SHARED(symbol->definition);

    //     auto errstr = boost::format("variable '%1%' shadows a variable from the outer scope") % *name;
    //     // auto errstr = ;
    //     ErrorManager::instance().report(
    //         W_SEMA_VAR_SHADOW_OUTER,
    //         ErrorSeverity::Warning,
    //         ErrorLocation(_filename, node->lineno(), node->colno(), 0),
    //         errstr.str());
    //     ErrorManager::instance().report_continued(
    //         ErrorSeverity::Warning,
    //         ErrorLocation(symbol->filename, original_node->lineno(), original_node->colno(), 0),
    //         "originally defined in");
    //     // it's a warning so return 0
    //     return 0;
    // } else {
    //     return 0;
    }
    return 0;
}

void SymbolAnalysisPass::attach_scope_id(const ASTNodePtr &node)
{
    auto scope_id = _scope_manager.get_current_scope_id();
    auto scope_info = std::make_shared<ScopeInfoAttribute>(scope_id, _scope_manager.get_current_scope_type());
    node->set_attribute(SemAnalzyerASTNodeAttributeId::ATTR_SEMANALYZER_SCOPE_INFO, scope_info);
}

void SymbolAnalysisPass::log_redefinition_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node)
{
    SymbolPtr defined_symbol;
    std::string _;
    bool symbol_found = _symbol_table->lookup_symbol(_scope_manager.get_current_scope_id(), name, false, defined_symbol, _);
    UNUSED(symbol_found);
    assert(symbol_found); // won't happen

    ASTNodePtr defined_node = WEAK_TO_SHARED(defined_symbol->definition);
    assert(defined_node);

    std::string type_str;
    switch (type) {
    case SymbolType::VARIABLE:
        type_str = "variable";
        break;
    case SymbolType::SUBROUTINE:
        type_str = "function/subprocedure";
        break;
    }

    auto errstr = boost::format("Redefinition of %1% '%2%'.")
        % type_str % *name;

    ErrorManager::instance().report(
        E_SEMA_SYM_REDEF,
        ErrorSeverity::Error,
        ErrorLocation(*_filename, node->lineno(), node->colno(), name->size()),
        errstr.str());

    ErrorManager::instance().report_continued(
        ErrorSeverity::Error,
        ErrorLocation(defined_symbol->filename, defined_node->lineno(), defined_node->colno(), name->size()),
        "Original defined in");
}

void SymbolAnalysisPass::log_undefined_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node)
{
    std::string type_str;
    switch (type) {
    case SymbolType::VARIABLE:
        type_str = "variable";
        break;
    case SymbolType::SUBROUTINE:
        type_str = "function/subprocedure";
        break;
    }

    auto errstr = boost::format("Undefined reference to '%2%'. The %1% '%2%' is not declared before use.")
        % type_str % *name;

    ErrorManager::instance().report(
        E_SEMA_SYM_UNDEFINED,
        ErrorSeverity::Error,
        ErrorLocation(*_filename, node->lineno(), node->colno(), name->size()),
        errstr.str());
}

void SymbolAnalysisPass::log_use_before_initialization_error(const StringPtr &name, const ASTNodePtr &node)
{
    SymbolPtr defined_symbol;

    bool symbol_found = lookup_symbol_with_ancestors(name, defined_symbol);
    UNUSED(symbol_found);
    assert(symbol_found); // won't happen

    ASTNodePtr defined_node = WEAK_TO_SHARED(defined_symbol->definition);
    assert(defined_node);
    assert(defined_symbol->type == SymbolType::VARIABLE);

    auto errstr = boost::format("Variable '%1%' may be used before assignment.") % *name;

    ErrorManager::instance().report(
        E_SEMA_VAR_USE_BEFORE_INIT,
        ErrorSeverity::Error,
        ErrorLocation(*_filename, node->lineno(), node->colno(), name->size()),
        errstr.str());

    ErrorManager::instance().report_continued(
        ErrorSeverity::Error,
        ErrorLocation(defined_symbol->filename, defined_node->lineno(), defined_node->colno(), 0),
        "Original defined in");
}

int SymbolAnalysisPass::get_varinit_record(const StringPtr &var_name)
{
    // SymbolPtr symbol;
    // std::string sym_defined_scope;
    // assert(lookup_symbol_with_ancestors(var_name, symbol, sym_defined_scope));

    // auto &stack = _varinit_record_stacks[SymbolScopeKey(sym_defined_scope, *var_name)];
    // assert(!stack.empty());
    // return stack.top();
}

void SymbolAnalysisPass::create_varinit_record(const StringPtr &var_name, int is_initialized)
{
    // SymbolPtr symbol;
    // std::string sym_defined_scope;
    // assert(lookup_symbol_with_ancestors(var_name, symbol, sym_defined_scope));

    // auto &stack = _varinit_record_stacks[SymbolScopeKey(sym_defined_scope, *var_name)];
    // assert(stack.empty());
    // stack.push(is_initialized);
}

void SymbolAnalysisPass::set_varinit_record(const SymbolScopeKey &key, int is_initialized)
{
    // // NOTE: symstack is not empty, because variable is declared first. the symstack is pushed at that time.
    // auto &stack = _varinit_record_stacks[key];
    // assert(!stack.empty());
    // stack.top() = is_initialized;
}

void SymbolAnalysisPass::set_varinit_record(const StringPtr &var_name, int is_initialized)
{
    // SymbolPtr symbol;
    // std::string sym_defined_scope;
    // assert(lookup_symbol_with_ancestors(var_name, symbol, sym_defined_scope));
    // set_varinit_record(SymbolScopeKey(sym_defined_scope, *var_name), is_initialized);
}

void SymbolAnalysisPass::enter_scope_varinit_record()
{
    // // 1. get all current scope variables (include ancestor defined)
    // // 2. for each variable, look at the top of stack result
    // // 3. push a new value=stack.top. The stack should not be empty - the variable must be declared before use
    // std::vector<std::pair<SymbolPtr, std::string>> all_symbols;
    // auto scope_id = _scope_manager.get_current_scope_id();
    // _symbol_table->get_symbols_include_ancestors(scope_id, all_symbols);
    // for (const auto &[symbol, sym_defined_scope] : all_symbols) {
    //     if (symbol->type == SymbolType::VARIABLE) {
    //         // NOTE: symstack is not empty, because variable is declared first. the symstack is pushed at that time.
    //         auto &stack = _varinit_record_stacks[SymbolScopeKey(sym_defined_scope, symbol->name)];
    //         assert(!stack.empty());
    //         int top = stack.top();
    //         stack.push(top);
    //     }
    // }
    // _varinit_record_stack_result.emplace();
}

void SymbolAnalysisPass::leave_scope_varinit_record()
{
    // SymbolScopedKeyValueHash result;

    // // note the order. we're actually writing the result to parent's result stack slot
    // // BUG: this is not right. When there are multiple scopes, it fails.
    // // we should figure out a way to passthrough children's result to parent
    // const auto &children_results = _varinit_record_stack_result.top();

    // std::vector<std::pair<SymbolPtr, std::string>> all_symbols;
    // auto scope_id = _scope_manager.get_current_scope_id();
    // _symbol_table->get_symbols_include_ancestors(scope_id, all_symbols);

    // for (const auto &[symbol, sym_defined_scope] : all_symbols) {
    //     if (symbol->type == SymbolType::VARIABLE) {
    //         auto symkey = SymbolScopeKey(sym_defined_scope, symbol->name);
    //         auto &stack = _varinit_record_stacks[symkey];
    //         // FIXME: we need to return this result
    //         // we also need to filter out those who has the scope_id of self,
    //         // because our ancestors don't care inner scoped vars! they can't access anyway.
    //         assert(!stack.empty());
    //         int sym_init_result = stack.top();
    //         stack.pop();
    //         if (sym_defined_scope != scope_id) {
    //             result[symkey] = sym_init_result;
    //         }
    //     }
    // }

    // for (const auto &[children_symkey, children_init_result] : children_results) {
    //     result[children_symkey] = children_init_result;
    // }

    // _varinit_record_stack_result.pop();
    // _varinit_record_stack_result.top().swap(result);
}

void SymbolAnalysisPass::get_child_varinit_records(SymbolScopedKeyValueHash &result)
{
    // result = _varinit_record_stack_result.top();
}

void SymbolAnalysisPass::set_child_varinit_records(const SymbolScopedKeyValueHash &records)
{
    // _varinit_record_stack_result.top() = records;
}

void SymbolAnalysisPass::enter_anonymous_scope()
{
    _scope_manager.enter_anonymous_scope();
    enter_scope_varinit_record();
}

void SymbolAnalysisPass::enter_scope(const StringPtr &name, ScopeType scope_type)
{
    _scope_manager.enter_scope(*name, scope_type);
    enter_scope_varinit_record();
}

void SymbolAnalysisPass::leave_scope()
{
    leave_scope_varinit_record();
    _scope_manager.exit_scope();
}

CLOSE_SEMANALYZER_NAMESPACE
// end
