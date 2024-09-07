#include <cassert>
#include <memory>
#include <string>
#include <typeinfo>

#include <boost/format.hpp>

#include <spdlog/spdlog.h>

#include "ASTNode.h"
#include "ErrorManager.h"
#include "ScopeManager.h"
#include "SymbolTable.h"
#include "SymbolTableAnalyzer.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

#define BEGIN_VISIT()       \
    _ancestors.push(node);  \
    attach_scope_id(node);  \
    int result = 0, rc = 0; \
    UNUSED(rc)

#define END_VISIT()   \
    _ancestors.pop(); \
    return result

#define SET_RESULT_RC() \
    if (rc != 0) {      \
        result = rc;    \
    }

int SymbolTableAnalyzer::run()
{
    if (!_symbol_table) {
        _symbol_table = std::make_shared<SymbolTable>();
    }

    int result = visit(_root);
    return result;
}

int SymbolTableAnalyzer::visit(IntegerASTNodePtr node)
{
    // Implement visit logic for IntegerASTNode
    BEGIN_VISIT();
    END_VISIT();
}

int SymbolTableAnalyzer::visit(BooleanASTNodePtr node)
{
    // Implement visit logic for BooleanASTNode
    BEGIN_VISIT();
    END_VISIT();
}

int SymbolTableAnalyzer::visit(VariableDeclarationASTNodePtr node)
{
    // Implement visit logic for VariableDeclarationASTNode
    BEGIN_VISIT();

    rc = add_symbol_or_log_error(node->get_name(), SymbolType::VARIABLE, node);
    SET_RESULT_RC();

    rc = traverse(node->get_assignment());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(VariableAssignmentASTNodePtr node)
{
    // Implement visit logic for VariableAssignmentASTNode
    BEGIN_VISIT();

    rc = attach_symbol_or_log_error(node->get_name(), SymbolType::VARIABLE, node);
    SET_RESULT_RC();
    rc = traverse(node->get_value());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(VariableAccessASTNodePtr node)
{
    // Implement visit logic for VariableAccessASTNode
    BEGIN_VISIT();

    rc = attach_symbol_or_log_error(node->get_name(), SymbolType::VARIABLE, node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(FloorBoxInitStatementASTNodePtr node)
{
    // Implement visit logic for FloorBoxInitStatementASTNode
    BEGIN_VISIT();

    rc = traverse(node->get_assignment());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(FloorAssignmentASTNodePtr node)
{
    // Implement visit logic for FloorAssignmentASTNode
    BEGIN_VISIT();

    rc = traverse(node->get_floor_number());
    SET_RESULT_RC();

    rc = traverse(node->get_value());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(FloorAccessASTNodePtr node)
{
    // Implement visit logic for FloorAccessASTNode
    BEGIN_VISIT();

    rc = traverse(node->get_index_expr());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(NegativeExpressionASTNodePtr node)
{
    // Implement visit logic for NegativeExpressionASTNode
    BEGIN_VISIT();

    rc = traverse(node->get_operand());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(NotExpressionASTNodePtr node)
{
    // Implement visit logic for NotExpressionASTNode
    BEGIN_VISIT();

    rc = traverse(node->get_operand());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(IncrementExpressionASTNodePtr node)
{
    // Implement visit logic for IncrementExpressionASTNode
    BEGIN_VISIT();

    rc = attach_symbol_or_log_error(node->get_var_name(), SymbolType::VARIABLE, node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(DecrementExpressionASTNodePtr node)
{
    // Implement visit logic for DecrementExpressionASTNode
    BEGIN_VISIT();

    rc = attach_symbol_or_log_error(node->get_var_name(), SymbolType::VARIABLE, node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(AddExpressionASTNodePtr node)
{
    // Implement visit logic for AddExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(SubExpressionASTNodePtr node)
{
    // Implement visit logic for SubExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(MulExpressionASTNodePtr node)
{
    // Implement visit logic for MulExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(DivExpressionASTNodePtr node)
{
    // Implement visit logic for DivExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(ModExpressionASTNodePtr node)
{
    // Implement visit logic for ModExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(EqualExpressionASTNodePtr node)
{
    // Implement visit logic for EqualExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(NotEqualExpressionASTNodePtr node)
{
    // Implement visit logic for NotEqualExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(GreaterThanExpressionASTNodePtr node)
{
    // Implement visit logic for GreaterThanExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(GreaterEqualExpressionASTNodePtr node)
{
    // Implement visit logic for GreaterEqualExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(LessThanExpressionASTNodePtr node)
{
    // Implement visit logic for LessThanExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(LessEqualExpressionASTNodePtr node)
{
    // Implement visit logic for LessEqualExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(AndExpressionASTNodePtr node)
{
    // Implement visit logic for AndExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(OrExpressionASTNodePtr node)
{
    // Implement visit logic for OrExpressionASTNode
    BEGIN_VISIT();

    rc = visit_binary_expression(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(InvocationExpressionASTNodePtr node)
{
    // Implement visit logic for InvocationExpressionASTNode
    BEGIN_VISIT();

    rc = attach_symbol_or_log_error(node->get_func_name(), SymbolType::SUBROUTINE, node);
    SET_RESULT_RC();

    rc = traverse(node->get_argument());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(EmptyStatementASTNodePtr node)
{
    // Implement visit logic for EmptyStatementASTNode
    // nothing
    BEGIN_VISIT();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(IfStatementASTNodePtr node)
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

int SymbolTableAnalyzer::visit(WhileStatementASTNodePtr node)
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

int SymbolTableAnalyzer::visit(ForStatementASTNodePtr node)
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

int SymbolTableAnalyzer::visit(ReturnStatementASTNodePtr node)
{
    // Implement visit logic for ReturnStatementASTNode
    BEGIN_VISIT();

    rc = traverse(node->get_expression());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(BreakStatementASTNodePtr node)
{
    // Implement visit logic for BreakStatementASTNode
    BEGIN_VISIT();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(ContinueStatementASTNodePtr node)
{
    // Implement visit logic for ContinueStatementASTNode
    BEGIN_VISIT();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(StatementBlockASTNodePtr node)
{
    // Implement visit logic for StatementBlockASTNode
    const ASTNode &parent = *_ancestors.top().get();
    bool come_from_while_if_for
        = typeid(parent) == typeid(WhileStatementASTNode)
        || typeid(parent) == typeid(IfStatementASTNode)
        || typeid(parent) == typeid(ForStatementASTNode);

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

int SymbolTableAnalyzer::visit(SubprocDefinitionASTNodePtr node)
{
    // Implement visit logic for SubprocDefinitionASTNode
    BEGIN_VISIT();

    rc = visit_subroutine(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(FunctionDefinitionASTNodePtr node)
{
    // Implement visit logic for FunctionDefinitionASTNode
    BEGIN_VISIT();

    rc = visit_subroutine(node);
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit(CompilationUnitASTNodePtr node)
{
    // Implement visit logic for CompilationUnitASTNode
    BEGIN_VISIT();

    rc = traverse(node->get_var_decls());
    SET_RESULT_RC();

    rc = traverse(node->get_subroutines());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit_binary_expression(AbstractBinaryExpressionASTNodePtr node)
{
    BEGIN_VISIT();

    rc = traverse(node->get_left());
    SET_RESULT_RC();

    rc = traverse(node->get_right());
    SET_RESULT_RC();

    END_VISIT();
}

int SymbolTableAnalyzer::visit_subroutine(AbstractSubroutineASTNodePtr node)
{
    // 1. is function already defined? if not, add it to the symbol table
    // 2. enter the function scope
    // 3. add parameter to symbol table
    // 4. traverse function body
    BEGIN_VISIT();

    const StringPtr &function_name = node->get_name();

    rc = add_symbol_or_log_error(function_name, SymbolType::SUBROUTINE, node);
    SET_RESULT_RC();

    _scope_manager.enter_scope(*function_name);

    auto param = node->get_parameter();
    if (param) {
        rc = add_symbol_or_log_error(param, SymbolType::VARIABLE, node);
        SET_RESULT_RC();
    }

    // passthrough the block level
    rc = traverse(node->get_body()->get_statements());
    SET_RESULT_RC();

    _scope_manager.exit_scope();

    END_VISIT();
}

bool SymbolTableAnalyzer::lookup_symbol(const StringPtr &name, SymbolPtr &out_symbol)
{
    return _symbol_table->lookup_symbol(_scope_manager.get_current_scope_id(), name, true, out_symbol);
}

int SymbolTableAnalyzer::attach_symbol_or_log_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node)
{
    SymbolPtr symbol;
    if (!lookup_symbol(name, symbol)) {
        log_undefined_error(name, type, node);
        return 1;
    } else {
        node->set_attribute(SemAnalzyerASTNodeAttributeId::ATTR_SEMANALYZER_SYMBOL, symbol);
        return 0;
    }
}

int SymbolTableAnalyzer::add_symbol_or_log_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node)
{
    bool added = false;

    switch (type) {
    case SymbolType::VARIABLE:
        added = _symbol_table->add_variable_symbol(_scope_manager.get_current_scope_id(), name, _filename, node);
        break;
    case SymbolType::SUBROUTINE:
        added = _symbol_table->add_function_symbol(_scope_manager.get_current_scope_id(), name, _filename, node);
        break;
    }

    if (!added) {
        log_redefinition_error(name, type, node);
        return 1;
    } else {
        return 0;
    }
}

void SymbolTableAnalyzer::attach_scope_id(const ASTNodePtr &node)
{
    auto scope_id = _scope_manager.get_current_scope_id();
    auto scope_info = std::make_shared<ScopeInfoAttribute>(scope_id);
    node->set_attribute(SemAnalzyerASTNodeAttributeId::ATTR_SEMANALYZER_SCOPE_INFO, scope_info);
}

void SymbolTableAnalyzer::log_redefinition_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node)
{
    SymbolPtr defined_symbol;
    bool symbol_found = _symbol_table->lookup_symbol(_scope_manager.get_current_scope_id(), name, false, defined_symbol);
    UNUSED(symbol_found);
    assert(symbol_found); // won't happen

    ASTNodePtr first_appearance = WEAK_TO_SHARED(defined_symbol->definition);

    std::string first_filename = "TBD";
    std::string first_lineno = "?";
    std::string first_colno = "?";

    if (first_appearance) {
        first_lineno = std::to_string(first_appearance->lineno());
        first_colno = std::to_string(first_appearance->colno());
    }

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
        % type_str % *name % first_filename % first_lineno % first_colno;

    ErrorManager::instance().report(
        3001,
        ErrorSeverity::Error,
        ErrorLocation(*_filename, node->lineno(), node->colno(), name->size()),
        errstr.str());
}

void SymbolTableAnalyzer::log_undefined_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node)
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
