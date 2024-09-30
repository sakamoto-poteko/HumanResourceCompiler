#include <cassert>
#include <memory>
#include <string>

#include <boost/format.hpp>
#include <spdlog/spdlog.h>

#include "ASTNode.h"
#include "ErrorManager.h"
#include "ErrorMessage.h"
#include "ScopeManager.h"
#include "SemanticAnalysisErrors.h"
#include "Symbol.h"
#include "SymbolAnalysisPass.h"
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
    SymbolPtr symbol;
    if (!lookup_symbol_with_ancestors(name, symbol) || symbol->type != type) {
        log_undefined_error(name, type, node);
        return E_SEMA_SYM_UNDEFINED;
    } else {
        symbol->set_to(node);
        return 0;
    }
}

int SymbolAnalysisPass::add_subroutine_symbol_or_log_error(const StringPtr &name, bool has_param, bool has_return, const ASTNodePtr &node)
{
    SymbolPtr added;
    if (!_symbol_table->add_function_symbol(_scope_manager.get_current_scope_id(), name, has_param, has_return, _filename, node, added)) {
        log_redefinition_error(name, SymbolType::SUBROUTINE, node);
        return E_SEMA_SYM_REDEF;
    } else {
        added->set_to(node);
        return 0;
    }
}

int SymbolAnalysisPass::add_variable_symbol_or_log_error(const StringPtr &name, const ASTNodePtr &node)
{
    SymbolPtr symbol, added;
    bool found_in_ancestor_or_current = lookup_symbol_with_ancestors(name, symbol);
    if (!_symbol_table->add_variable_symbol(_scope_manager.get_current_scope_id(), name, _filename, node, added)) {
        // false indicate found in current
        log_redefinition_error(name, SymbolType::VARIABLE, node);
        return E_SEMA_SYM_REDEF;
    }

    if (found_in_ancestor_or_current) {
        // added to current scope and ancestor has it
        auto original_node = WEAK_TO_SHARED(symbol->definition);

        auto errstr = boost::format("variable '%1%' shadows a %2% from the outer scope")
            % *name % (symbol->type == SymbolType::VARIABLE ? "variable" : "function");

        ErrorManager::instance().report(
            W_SEMA_VAR_SHADOW_OUTER,
            ErrorSeverity::Warning,
            ErrorLocation(_filename, node->lineno(), node->colno(), 0),
            errstr.str());
        ErrorManager::instance().report_continued(
            ErrorSeverity::Warning,
            ErrorLocation(symbol->filename, original_node->lineno(), original_node->colno(), 0),
            "originally defined in");
    }

    // it's a warning so return 0
    added->set_to(node);
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

void SymbolAnalysisPass::enter_anonymous_scope()
{
    _scope_manager.enter_anonymous_scope();
}

void SymbolAnalysisPass::enter_scope(const StringPtr &name, ScopeType scope_type)
{
    _scope_manager.enter_scope(*name, scope_type);
}

void SymbolAnalysisPass::leave_scope()
{
    _scope_manager.exit_scope();
}

CLOSE_SEMANALYZER_NAMESPACE
// end
