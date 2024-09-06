#include <cassert>
#include <memory>
#include <string>

#include <boost/format.hpp>

#include <spdlog/spdlog.h>

#include "ASTNode.h"
#include "ASTNodeForward.h"
#include "SymbolTable.h"
#include "SymbolTableBuilder.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

#define BEGIN_VISIT() \
    int result = 0, rc = 0

#define SET_RESULT_RC() \
    if (rc != 0) {      \
        result = rc;    \
    }                   \
    0

bool SymbolTableBuilder::build(SymbolTablePtr &symbol_table)
{
    if (!_symbol_table) {
        _symbol_table = std::make_shared<SymbolTable>();
    }

    int result = visit(_root);
    if (result == 0) {
        symbol_table = _symbol_table;
        return true;
    } else {
        return false;
    }
}

int SymbolTableBuilder::visit(IntegerASTNodePtr node)
{
    // Implement visit logic for IntegerASTNode
    return 0;
}

int SymbolTableBuilder::visit(BooleanASTNodePtr node)
{
    // Implement visit logic for BooleanASTNode
    return 0;
}

int SymbolTableBuilder::visit(VariableDeclarationASTNodePtr node)
{
    // Implement visit logic for VariableDeclarationASTNode
    return 0;
}

int SymbolTableBuilder::visit(VariableAssignmentASTNodePtr node)
{
    // Implement visit logic for VariableAssignmentASTNode
    return 0;
}

int SymbolTableBuilder::visit(VariableAccessASTNodePtr node)
{
    // Implement visit logic for VariableAccessASTNode
    return 0;
}

int SymbolTableBuilder::visit(FloorBoxInitStatementASTNodePtr node)
{
    // Implement visit logic for FloorBoxInitStatementASTNode
    return 0;
}

int SymbolTableBuilder::visit(FloorAssignmentASTNodePtr node)
{
    // Implement visit logic for FloorAssignmentASTNode
    return 0;
}

int SymbolTableBuilder::visit(FloorAccessASTNodePtr node)
{
    // Implement visit logic for FloorAccessASTNode
    return 0;
}

int SymbolTableBuilder::visit(NegativeExpressionASTNodePtr node)
{
    // Implement visit logic for NegativeExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(NotExpressionASTNodePtr node)
{
    // Implement visit logic for NotExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(IncrementExpressionASTNodePtr node)
{
    // Implement visit logic for IncrementExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(DecrementExpressionASTNodePtr node)
{
    // Implement visit logic for DecrementExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(AddExpressionASTNodePtr node)
{
    // Implement visit logic for AddExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(SubExpressionASTNodePtr node)
{
    // Implement visit logic for SubExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(MulExpressionASTNodePtr node)
{
    // Implement visit logic for MulExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(DivExpressionASTNodePtr node)
{
    // Implement visit logic for DivExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(ModExpressionASTNodePtr node)
{
    // Implement visit logic for ModExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(EqualExpressionASTNodePtr node)
{
    // Implement visit logic for EqualExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(NotEqualExpressionASTNodePtr node)
{
    // Implement visit logic for NotEqualExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(GreaterThanExpressionASTNodePtr node)
{
    // Implement visit logic for GreaterThanExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(GreaterEqualExpressionASTNodePtr node)
{
    // Implement visit logic for GreaterEqualExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(LessThanExpressionASTNodePtr node)
{
    // Implement visit logic for LessThanExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(LessEqualExpressionASTNodePtr node)
{
    // Implement visit logic for LessEqualExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(AndExpressionASTNodePtr node)
{
    // Implement visit logic for AndExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(OrExpressionASTNodePtr node)
{
    // Implement visit logic for OrExpressionASTNode
    return 0;
}

int SymbolTableBuilder::visit(InvocationExpressionASTNodePtr node)
{
    // Implement visit logic for InvocationExpressionASTNode
    BEGIN_VISIT();

    rc = lookup_and_attach_symbol_to_node_or_report_error(node->get_func_name(), SymbolType::SUBROUTINE, node);
    SET_RESULT_RC();

    rc = traverse(node->get_argument());

    return result;
}

int SymbolTableBuilder::visit(EmptyStatementASTNodePtr node)
{
    // Implement visit logic for EmptyStatementASTNode
    // nothing
    return 0;
}

int SymbolTableBuilder::visit(IfStatementASTNodePtr node)
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

    return result;
}

int SymbolTableBuilder::visit(WhileStatementASTNodePtr node)
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
    return result;
}

int SymbolTableBuilder::visit(ForStatementASTNodePtr node)
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
    return result;
}

int SymbolTableBuilder::visit(ReturnStatementASTNodePtr node)
{
    // Implement visit logic for ReturnStatementASTNode

    return traverse(node->get_expression());
}

int SymbolTableBuilder::visit(BreakStatementASTNodePtr node)
{
    // Implement visit logic for BreakStatementASTNode
    // nothing
    return 0;
}

int SymbolTableBuilder::visit(ContinueStatementASTNodePtr node)
{
    // Implement visit logic for ContinueStatementASTNode
    // nothing
    return 0;
}

int SymbolTableBuilder::visit(StatementBlockASTNodePtr node)
{
    // Implement visit logic for StatementBlockASTNode
    BEGIN_VISIT();

    _scope_manager.enter_anonymous_scope();

    rc = traverse(node->get_statements());
    SET_RESULT_RC();

    _scope_manager.exit_scope();

    return result;
}

int SymbolTableBuilder::visit(SubprocDefinitionASTNodePtr node)
{
    // Implement visit logic for SubprocDefinitionASTNode
    return visit_subroutine(node);
}

int SymbolTableBuilder::visit(FunctionDefinitionASTNodePtr node)
{
    // Implement visit logic for FunctionDefinitionASTNode
    return visit_subroutine(node);
}

int SymbolTableBuilder::visit(CompilationUnitASTNodePtr node)
{
    // Implement visit logic for CompilationUnitASTNode
    BEGIN_VISIT();

    rc = traverse(node->get_var_decls());
    SET_RESULT_RC();

    rc = traverse(node->get_subroutines());
    SET_RESULT_RC();

    return result;
}

int SymbolTableBuilder::visit_subroutine(AbstractSubroutineASTNodePtr node)
{
    // 1. is function already defined? if not, add it to the symbol table
    // 2. enter the function scope
    // 3. add parameter to symbol table
    // 4. traverse function body

    int result = 0;
    const StringPtr &function_name = node->get_name();

    auto current_scope = _scope_manager.get_current_scope_id();
    bool func_added = _symbol_table->add_function_symbol(current_scope, function_name, node);
    if (!func_added) {
        log_redefinition_error(function_name, SymbolType::SUBROUTINE, node);
        result = 1;
    }

    _scope_manager.enter_scope(*function_name);
    current_scope = _scope_manager.get_current_scope_id();

    auto param = node->get_parameter();
    if (param) {
        bool param_added = _symbol_table->add_variable_symbol(current_scope, param, node);
        if (!param_added) {
            log_redefinition_error(param, SymbolType::VARIABLE, node);
            result = 1;
        }
    }

    // passthrough the block level
    int rc = traverse(node->get_body()->get_statements());

    _scope_manager.exit_scope();
    return rc;
}

bool SymbolTableBuilder::lookup_symbol(const StringPtr &name, SymbolPtr &out_symbol)
{
    return _symbol_table->lookup_symbol(_scope_manager.get_current_scope_id(), name, true, out_symbol);
}

int SymbolTableBuilder::lookup_and_attach_symbol_to_node_or_report_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node)
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

void SymbolTableBuilder::log_redefinition_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node)
{
    SymbolPtr defined_symbol;
    bool symbol_found = _symbol_table->lookup_symbol(_scope_manager.get_current_scope_id(), name, false, defined_symbol);
    assert(symbol_found);

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

    auto fmt = boost::format("Redefinition of %8% '%1%', originally defined at %2%:%3%:%4%, redefined at %5%:%6%:%7%")
        % *name % first_filename % first_lineno % first_colno % "TBD" % node->lineno() % node->colno() % type_str;

    // Log an error about the redefinition
    spdlog::error(fmt.str());
}

void SymbolTableBuilder::log_undefined_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node)
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

    auto fmt = boost::format("Undefined reference to '%2%' at %3%:%4%:%5%. The %1% '%2%' is not declared before use.")
        % type_str % *name % "TBD" % node->lineno() % node->colno();

    // Log an error about the redefinition
    spdlog::error(fmt.str());
}

CLOSE_SEMANALYZER_NAMESPACE
// end
