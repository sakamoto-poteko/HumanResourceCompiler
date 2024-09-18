#include <cassert>

#include <boost/format.hpp>

#include "ErrorManager.h"
#include "ErrorMessage.h"
#include "SemanticAnalysisErrors.h"
#include "SemanticAnalysisPass.h"
#include "Symbol.h"
#include "UnusedSymbolAnalysisPass.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

#define BEGIN_VISIT()                          \
    enter_node(node);                          \
    SymbolPtr symbol = Symbol::get_from(node); \
    int rc = 0;                                \
    UNUSED(rc);

#define END_VISIT() \
    leave_node();   \
    return 0;

#define RETURN_IF_FAIL_IN_VISIT(rc) \
    if (rc != 0) {                  \
        leave_node();               \
        return rc;                  \
    }

int UnusedSymbolAnalysisPass::run()
{
    _unused_symbols.clear();
    std::vector<std::pair<SymbolPtr, std::string>> all_symbols;
    _symbol_table->get_all_symbols(all_symbols);
    for (const auto &[symbol, define_scope] : all_symbols) {
        // care about self only?
        // FIXME: consider remove this after importing all files?
        if (symbol->filename == _filename) {
            _unused_symbols.insert(symbol);
        }
    }

    _visit_state = VisitState::CollectionPass;
    int rc = visit(_root);
    if (rc != 0) {
        return rc;
    }

    _visit_state = VisitState::MutationPass;
    rc = visit(_root);
    if (rc != 0) {
        return rc;
    }

    return 0;
}

void UnusedSymbolAnalysisPass::enter_node(const parser::ASTNodePtr &node)
{
    SemanticAnalysisPass::enter_node(node);
}

int UnusedSymbolAnalysisPass::visit(const parser::VariableAccessASTNodePtr &node)
{
    return visit_using_variable(node);
}

int UnusedSymbolAnalysisPass::visit(const parser::VariableDeclarationASTNodePtr &node)
{
    BEGIN_VISIT();
    assert(symbol);

    switch (_visit_state) {
    case VisitState::CollectionPass: {
    } break;
    case VisitState::MutationPass: {
        // we skip global vars. this is done by visit compilation unit without visiting globals
        if (_unused_symbols.contains(symbol)) {
            log_unused_variable(node);
            request_to_remove_self();
            // it's a warning so we return 0
        }
    } break;
    }

    END_VISIT();
}

int UnusedSymbolAnalysisPass::visit(const parser::IncrementExpressionASTNodePtr &node)
{
    return visit_using_variable(node);
}

int UnusedSymbolAnalysisPass::visit(const parser::DecrementExpressionASTNodePtr &node)
{
    return visit_using_variable(node);
}

int hrl::semanalyzer::UnusedSymbolAnalysisPass::visit(const parser::VariableAssignmentASTNodePtr &node)
{
    return visit_using_variable(node);
}

int hrl::semanalyzer::UnusedSymbolAnalysisPass::visit_using_variable(const parser::ASTNodePtr &node)
{
    BEGIN_VISIT();
    assert(symbol);

    switch (_visit_state) {
    case VisitState::CollectionPass: {
        _unused_symbols.erase(symbol);
    } break;
    case VisitState::MutationPass: {
    } break;
    }

    END_VISIT();
}

int UnusedSymbolAnalysisPass::visit(const parser::CompilationUnitASTNodePtr &node)
{
    BEGIN_VISIT();

    rc = traverse_multiple(node->get_subroutines());
    RETURN_IF_FAIL_IN_VISIT(rc);

    END_VISIT();
}

int UnusedSymbolAnalysisPass::log_unused_variable(const parser::VariableDeclarationASTNodePtr &node)
{
    auto errstr = boost::format("Variable '%1%' defined but not used.") % *node->get_name();
    ErrorManager::instance().report(
        W_SEMA_VAR_DEFINED_BUT_UNUSED,
        ErrorSeverity::Warning,
        ErrorLocation(_filename, node->lineno(), node->colno(), 0),
        errstr.str());
    return W_SEMA_VAR_DEFINED_BUT_UNUSED;
}

CLOSE_SEMANALYZER_NAMESPACE
// end
