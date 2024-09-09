#include <cassert>
#include <memory>
#include <string>
#include <typeinfo>

#include <boost/format.hpp>

#include <spdlog/spdlog.h>

#include "ASTNode.h"
#include "ErrorManager.h"
#include "ErrorMessage.h"
#include "ScopeManager.h"
#include "Symbol.h"
#include "SymbolAnalysisPass.h"
#include "SymbolTable.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

#define BEGIN_VISIT()       \
    enter_node(node);       \
    attach_scope_id(node);  \
    int result = 0, rc = 0; \
    UNUSED(rc)

#define END_VISIT() \
    leave_node();   \
    return result

#define SET_RESULT_RC() \
    if (rc != 0) {      \
        result = rc;    \
    }

int SymbolAnalysisPass::run()
{
    if (!_symbol_table) {
        _symbol_table = std::make_shared<SymbolTable>();
    }

    int result = 0, rc = 0;
    rc = visit(_root);
    SET_RESULT_RC();
    rc = check_pending_invocations();
    SET_RESULT_RC();
    return result;
}

int SymbolAnalysisPass::check_pending_invocations()
{
    int result = 0, rc = 0;
    while (!_pending_invocation_check.empty()) {
        auto node = _pending_invocation_check.front();
        _pending_invocation_check.pop();

        StringPtr func_name = node->get_func_name();
        rc = attach_symbol_or_log_error(func_name, SymbolType::SUBROUTINE, node);
        SET_RESULT_RC();

        // symbol not found. let's proceed to next item.
        if (rc) {
            continue;
        }

        // signature check
        SymbolPtr symbol;
        lookup_symbol(func_name, symbol);

        bool def_has_param = symbol->has_param();
        bool def_has_return = symbol->has_return();
        bool node_has_param = node->get_argument().operator bool();

        auto def_astnode = WEAK_TO_SHARED(symbol->definition);

        if (def_has_param != node_has_param) {
            auto errstr = boost::format("signature mismatch: invoked as '%4%(%5%)' but defined as '%1% %2%(%3%)'")
                % (def_has_return ? "function" : "sub")
                % *func_name
                % (def_has_param ? "arg" : "")
                % *func_name
                % (node_has_param ? "arg" : "");
            // % symbol->filename % def_astnode->lineno() % def_astnode->colno();

            ErrorManager::instance().report(
                3005,
                ErrorSeverity::Error,
                ErrorLocation(*_filename, node->lineno(), node->colno(), func_name->size()),
                errstr.str());
            ErrorManager::instance().report_continued(
                ErrorSeverity::Error,
                ErrorLocation(symbol->filename, def_astnode->lineno(), def_astnode->colno(), 0),
                "originally defined as");
            rc = 3005;
            SET_RESULT_RC();
        }
    }
    return result;
}

// WARNING: Do not use `return SemanticAnalysisPass::visit(node)`
// Because we need to attach scope id to each node!

int SymbolAnalysisPass::visit(IntegerASTNodePtr node)
{
    // Implement visit logic for IntegerASTNode
    BEGIN_VISIT();
    END_VISIT();
}

int SymbolAnalysisPass::visit(BooleanASTNodePtr node)
{
    // Implement visit logic for BooleanASTNode
    BEGIN_VISIT();
    END_VISIT();
}

int SymbolAnalysisPass::visit(VariableDeclarationASTNodePtr node)
{
    // Implement visit logic for VariableDeclarationASTNode
    BEGIN_VISIT();

    rc = add_variable_symbol_or_log_error(node->get_name(), node);
    SET_RESULT_RC();

    rc = traverse(node->get_assignment());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(VariableAssignmentASTNodePtr node)
{
    // Implement visit logic for VariableAssignmentASTNode
    BEGIN_VISIT();

    rc = attach_symbol_or_log_error(node->get_name(), SymbolType::VARIABLE, node);
    SET_RESULT_RC();
    rc = traverse(node->get_value());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(VariableAccessASTNodePtr node)
{
    // Implement visit logic for VariableAccessASTNode
    BEGIN_VISIT();

    rc = attach_symbol_or_log_error(node->get_name(), SymbolType::VARIABLE, node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(FloorBoxInitStatementASTNodePtr node)
{
    // Implement visit logic for FloorBoxInitStatementASTNode
    BEGIN_VISIT();

    rc = traverse(node->get_assignment());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(FloorAssignmentASTNodePtr node)
{
    // Implement visit logic for FloorAssignmentASTNode
    BEGIN_VISIT();

    rc = traverse(node->get_floor_number());
    SET_RESULT_RC();

    rc = traverse(node->get_value());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(FloorAccessASTNodePtr node)
{
    // Implement visit logic for FloorAccessASTNode
    BEGIN_VISIT();

    rc = traverse(node->get_index_expr());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(NegativeExpressionASTNodePtr node)
{
    // Implement visit logic for NegativeExpressionASTNode
    BEGIN_VISIT();

    rc = traverse(node->get_operand());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(NotExpressionASTNodePtr node)
{
    // Implement visit logic for NotExpressionASTNode
    BEGIN_VISIT();

    rc = traverse(node->get_operand());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(IncrementExpressionASTNodePtr node)
{
    // Implement visit logic for IncrementExpressionASTNode
    BEGIN_VISIT();

    rc = attach_symbol_or_log_error(node->get_var_name(), SymbolType::VARIABLE, node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(DecrementExpressionASTNodePtr node)
{
    // Implement visit logic for DecrementExpressionASTNode
    BEGIN_VISIT();

    rc = attach_symbol_or_log_error(node->get_var_name(), SymbolType::VARIABLE, node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(AddExpressionASTNodePtr node)
{
    // Implement visit logic for AddExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(SubExpressionASTNodePtr node)
{
    // Implement visit logic for SubExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(MulExpressionASTNodePtr node)
{
    // Implement visit logic for MulExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(DivExpressionASTNodePtr node)
{
    // Implement visit logic for DivExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(ModExpressionASTNodePtr node)
{
    // Implement visit logic for ModExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(EqualExpressionASTNodePtr node)
{
    // Implement visit logic for EqualExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(NotEqualExpressionASTNodePtr node)
{
    // Implement visit logic for NotEqualExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(GreaterThanExpressionASTNodePtr node)
{
    // Implement visit logic for GreaterThanExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(GreaterEqualExpressionASTNodePtr node)
{
    // Implement visit logic for GreaterEqualExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(LessThanExpressionASTNodePtr node)
{
    // Implement visit logic for LessThanExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(LessEqualExpressionASTNodePtr node)
{
    // Implement visit logic for LessEqualExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(AndExpressionASTNodePtr node)
{
    // Implement visit logic for AndExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(OrExpressionASTNodePtr node)
{
    // Implement visit logic for OrExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(InvocationExpressionASTNodePtr node)
{
    // Implement visit logic for InvocationExpressionASTNode
    BEGIN_VISIT();

    _pending_invocation_check.push(node);

    rc = traverse(node->get_argument());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(EmptyStatementASTNodePtr node)
{
    // Implement visit logic for EmptyStatementASTNode
    // nothing
    BEGIN_VISIT();

    END_VISIT();
}

int SymbolAnalysisPass::visit(IfStatementASTNodePtr node)
{
    // Implement visit logic for IfStatementASTNode
    // For an if stmt, the condition is not in its body scope
    // Then and Else are different scopes
    BEGIN_VISIT();

    rc = traverse(node->get_condition());
    SET_RESULT_RC();

    _scope_manager.enter_anonymous_scope();
    rc = traverse(node->get_then_branch());
    SET_RESULT_RC();
    _scope_manager.exit_scope();

    _scope_manager.enter_anonymous_scope();
    rc = traverse(node->get_else_branch());
    SET_RESULT_RC();
    _scope_manager.exit_scope();

    END_VISIT();
}

int SymbolAnalysisPass::visit(WhileStatementASTNodePtr node)
{
    // Implement visit logic for WhileStatementASTNode

    // For a while stmt, the condition is not in its body scope
    BEGIN_VISIT();

    rc = traverse(node->get_condition());
    SET_RESULT_RC();

    _scope_manager.enter_anonymous_scope();

    rc = traverse(node->get_body());
    SET_RESULT_RC();

    _scope_manager.exit_scope();
    END_VISIT();
}

int SymbolAnalysisPass::visit(ForStatementASTNodePtr node)
{
    // Implement visit logic for ForStatementASTNode
    BEGIN_VISIT();

    _scope_manager.enter_anonymous_scope();

    rc = traverse(node->get_init());
    SET_RESULT_RC();

    rc = traverse(node->get_condition());
    SET_RESULT_RC();

    rc = traverse(node->get_update());
    SET_RESULT_RC();

    rc = traverse(node->get_body());
    SET_RESULT_RC();

    _scope_manager.exit_scope();
    END_VISIT();
}

int SymbolAnalysisPass::visit(ReturnStatementASTNodePtr node)
{
    // Implement visit logic for ReturnStatementASTNode
    BEGIN_VISIT();

    rc = traverse(node->get_expression());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(BreakStatementASTNodePtr node)
{
    // Implement visit logic for BreakStatementASTNode
    BEGIN_VISIT();

    END_VISIT();
}

int SymbolAnalysisPass::visit(ContinueStatementASTNodePtr node)
{
    // Implement visit logic for ContinueStatementASTNode
    BEGIN_VISIT();

    END_VISIT();
}

int SymbolAnalysisPass::visit(StatementBlockASTNodePtr node)
{
    // Implement visit logic for StatementBlockASTNode
    const ASTNodePtr &parent = _ancestors.top();
    bool come_from_while_if_for
        = is_ptr_type<WhileStatementASTNode>(parent)
        || is_ptr_type<IfStatementASTNode>(parent)
        || is_ptr_type<ForStatementASTNode>(parent);

    BEGIN_VISIT();

    // if entered from while/if/for, skip the new scope since these stmts entered already

    if (!come_from_while_if_for) {
        _scope_manager.enter_anonymous_scope();
    }

    rc = traverse(node->get_statements());
    SET_RESULT_RC();

    if (!come_from_while_if_for) {
        _scope_manager.exit_scope();
    }

    END_VISIT();
}

int SymbolAnalysisPass::visit(SubprocDefinitionASTNodePtr node)
{
    // Implement visit logic for SubprocDefinitionASTNode
    BEGIN_VISIT();

    rc = visit_subroutine(node, false);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(FunctionDefinitionASTNodePtr node)
{
    // Implement visit logic for FunctionDefinitionASTNode
    BEGIN_VISIT();

    rc = visit_subroutine(node, true);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit(CompilationUnitASTNodePtr node)
{
    // Implement visit logic for CompilationUnitASTNode
    BEGIN_VISIT();

    rc = traverse(node->get_var_decls());
    SET_RESULT_RC();

    rc = traverse(node->get_subroutines());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit_binary_expression(AbstractBinaryExpressionASTNodePtr node)
{
    BEGIN_VISIT();

    rc = traverse(node->get_left());
    SET_RESULT_RC();

    rc = traverse(node->get_right());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolAnalysisPass::visit_subroutine(AbstractSubroutineASTNodePtr node, bool has_return)
{
    // 1. is function already defined? if not, add it to the symbol table
    // 2. enter the function scope
    // 3. add parameter to symbol table
    // 4. traverse function body
    BEGIN_VISIT();

    const StringPtr &function_name = node->get_name();
    auto param = node->get_parameter();
    bool has_param = param.operator bool();

    rc = add_subroutine_symbol_or_log_error(function_name, has_param, has_return, node);
    SET_RESULT_RC();

    _scope_manager.enter_scope(*function_name, ScopeType::Subroutine);

    if (param) {
        rc = add_variable_symbol_or_log_error(param, node);
        SET_RESULT_RC();
    }

    // passthrough the block level
    rc = traverse(node->get_body()->get_statements());
    SET_RESULT_RC();

    _scope_manager.exit_scope();

    END_VISIT();
}

bool SymbolAnalysisPass::lookup_symbol(const StringPtr &name, SymbolPtr &out_symbol)
{
    return _symbol_table->lookup_symbol(_scope_manager.get_current_scope_id(), name, true, out_symbol);
}

int SymbolAnalysisPass::attach_symbol_or_log_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node)
{
    SymbolPtr symbol;
    if (!lookup_symbol(name, symbol)) {
        log_undefined_error(name, type, node);
        return 3002;
    } else {
        node->set_attribute(SemAnalzyerASTNodeAttributeId::ATTR_SEMANALYZER_SYMBOL, symbol);
        return 0;
    }
}

int SymbolAnalysisPass::add_subroutine_symbol_or_log_error(const StringPtr &name, bool has_param, bool has_return, const ASTNodePtr &node)
{
    if (!_symbol_table->add_function_symbol(_scope_manager.get_current_scope_id(), name, has_param, has_return, _filename, node)) {
        log_redefinition_error(name, SymbolType::SUBROUTINE, node);
        return 3001;
    } else {
        return 0;
    }
}

int SymbolAnalysisPass::add_variable_symbol_or_log_error(const StringPtr &name, const ASTNodePtr &node)
{
    SymbolPtr symbol;
    bool found_in_ancestor_or_current = _symbol_table->lookup_symbol(_scope_manager.get_current_scope_id(), name, true, symbol);
    if (!_symbol_table->add_variable_symbol(_scope_manager.get_current_scope_id(), name, _filename, node)) {
        // false indicate found in current
        log_redefinition_error(name, SymbolType::VARIABLE, node);
        return 3001;
    } else if (found_in_ancestor_or_current) {
        // added to current scope and ancestor has it
        auto original_node = WEAK_TO_SHARED(symbol->definition);

        auto errstr = boost::format("variable '%1%' shadows a variable from the outer scope") % *name;
        // auto errstr = ;
        ErrorManager::instance().report(
            3006,
            ErrorSeverity::Warning,
            ErrorLocation(_filename, node->lineno(), node->colno(), 0),
            errstr.str());
        ErrorManager::instance().report_continued(
            ErrorSeverity::Warning,
            ErrorLocation(symbol->filename, original_node->lineno(), original_node->colno(), 0),
            "originally defined in");
        return 0;
    } else {
        return 0;
    }
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
    bool symbol_found = _symbol_table->lookup_symbol(_scope_manager.get_current_scope_id(), name, false, defined_symbol);
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

    auto errstr = boost::format("Redefinition of %1% '%2%', originally defined at %2%:%3%:%4%")
        % type_str % *name;

    ErrorManager::instance().report(
        3001,
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
        3002,
        ErrorSeverity::Error,
        ErrorLocation(*_filename, node->lineno(), node->colno(), name->size()),
        errstr.str());
}

CLOSE_SEMANALYZER_NAMESPACE
// end
