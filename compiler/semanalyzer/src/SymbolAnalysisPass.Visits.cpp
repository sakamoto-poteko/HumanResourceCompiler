#include <cassert>
#include <memory>
#include <string>

#include <boost/format.hpp>
#include <spdlog/spdlog.h>

#include "ASTNode.h"
#include "ASTNodeForward.h"
#include "ErrorManager.h"
#include "ErrorMessage.h"
#include "ScopeManager.h"
#include "SemanticAnalysisErrors.h"
#include "SemanticAnalysisPass.h"
#include "Symbol.h"
#include "SymbolAnalysisPass.h"
#include "SymbolTable.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

#define BEGIN_VISIT()       \
    enter_node(node);       \
    int result = 0, rc = 0; \
    UNUSED(rc)

#define END_VISIT() \
    leave_node();   \
    return result

// We're making it fail fast now. This is required to avoid a lot of bugs.
#define SET_RESULT_RC_AND_RETURN_IN_VISIT() \
    if (rc != 0) {                          \
        result = rc;                        \
        END_VISIT();                        \
    }

#define RETURN_IF_FAIL() \
    if (rc != 0) {       \
        return rc;       \
    }

int SymbolAnalysisPass::run()
{
    init_symbol_table();

    int result = 0, rc = 0;
    rc = visit(_root);
    RETURN_IF_FAIL();
    rc = check_pending_invocations();
    RETURN_IF_FAIL();
    return result;
}

// reside in this file because it requires macros. pretty much like the visits
int SymbolAnalysisPass::check_pending_invocations()
{
    int result = 0, rc = 0;
    while (!_pending_invocation_check.empty()) {
        auto node = _pending_invocation_check.front();
        _pending_invocation_check.pop();

        StringPtr func_name = node->get_func_name();
        rc = attach_symbol_or_log_error(func_name, SymbolType::SUBROUTINE, node);
        RETURN_IF_FAIL();

        // symbol not found. let's proceed to next item.
        if (rc) {
            continue;
        }

        // signature check
        SymbolPtr symbol;
        lookup_symbol_with_ancestors(func_name, symbol);

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
                E_SEMA_SUBROUTINE_SIGNATURE_MISMATCH,
                ErrorSeverity::Error,
                ErrorLocation(*_filename, node->lineno(), node->colno(), func_name->size()),
                errstr.str());
            ErrorManager::instance().report_continued(
                ErrorSeverity::Error,
                ErrorLocation(symbol->filename, def_astnode->lineno(), def_astnode->colno(), 0),
                "originally defined as");
            rc = E_SEMA_SUBROUTINE_SIGNATURE_MISMATCH;
            RETURN_IF_FAIL();
        }
    }
    return result;
}

int SymbolAnalysisPass::visit(const IntegerASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const BooleanASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const VariableDeclarationASTNodePtr &node)
{
    // Implement visit logic for VariableDeclarationASTNode
    BEGIN_VISIT();

    rc = add_variable_symbol_or_log_error(node->get_name(), node);
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    rc = traverse(node->get_assignment());
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    END_VISIT();
}

int SymbolAnalysisPass::visit(const VariableAssignmentASTNodePtr &node)
{
    // Implement visit logic for VariableAssignmentASTNode
    BEGIN_VISIT();

    rc = attach_symbol_or_log_error(node->get_name(), SymbolType::VARIABLE, node);
    SET_RESULT_RC_AND_RETURN_IN_VISIT();
    rc = traverse(node->get_value());
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    END_VISIT();
}

int SymbolAnalysisPass::visit(const VariableAccessASTNodePtr &node)
{
    // Implement visit logic for VariableAccessASTNode
    BEGIN_VISIT();

    rc = attach_symbol_or_log_error(node->get_name(), SymbolType::VARIABLE, node);
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    END_VISIT();
}

int SymbolAnalysisPass::visit(const FloorBoxInitStatementASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const FloorAssignmentASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const FloorAccessASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const NegativeExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const NotExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const IncrementExpressionASTNodePtr &node)
{
    // Implement visit logic for IncrementExpressionASTNode
    BEGIN_VISIT();

    rc = attach_symbol_or_log_error(node->get_var_name(), SymbolType::VARIABLE, node);
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    END_VISIT();
}

int SymbolAnalysisPass::visit(const DecrementExpressionASTNodePtr &node)
{
    // Implement visit logic for DecrementExpressionASTNode
    BEGIN_VISIT();

    rc = attach_symbol_or_log_error(node->get_var_name(), SymbolType::VARIABLE, node);
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    END_VISIT();
}

int SymbolAnalysisPass::visit(const AddExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const SubExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const MulExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const DivExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const ModExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const EqualExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const NotEqualExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const GreaterThanExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const GreaterEqualExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const LessThanExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const LessEqualExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const AndExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const OrExpressionASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const InvocationExpressionASTNodePtr &node)
{
    // Implement visit logic for InvocationExpressionASTNode
    BEGIN_VISIT();

    _pending_invocation_check.push(node);

    rc = traverse(node->get_argument());
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    END_VISIT();
}

int SymbolAnalysisPass::visit(const EmptyStatementASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const IfStatementASTNodePtr &node)
{
    // Implement visit logic for IfStatementASTNode
    // For an if stmt, the condition is not in its body scope
    // Then and Else are different scopes
    BEGIN_VISIT();

    rc = traverse(node->get_condition());
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    enter_anonymous_scope();
    rc = traverse(node->get_then_branch());
    SET_RESULT_RC_AND_RETURN_IN_VISIT();
    leave_scope();

    enter_anonymous_scope();
    rc = traverse(node->get_else_branch());
    SET_RESULT_RC_AND_RETURN_IN_VISIT();
    leave_scope();

    END_VISIT();
}

int SymbolAnalysisPass::visit(const WhileStatementASTNodePtr &node)
{
    // Implement visit logic for WhileStatementASTNode

    // For a while stmt, the condition is not in its body scope
    BEGIN_VISIT();

    rc = traverse(node->get_condition());
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    enter_anonymous_scope();

    rc = traverse(node->get_body());
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    leave_scope();
    END_VISIT();
}

int SymbolAnalysisPass::visit(const ForStatementASTNodePtr &node)
{
    // Implement visit logic for ForStatementASTNode
    BEGIN_VISIT();

    enter_anonymous_scope();

    rc = traverse(node->get_init());
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    rc = traverse(node->get_condition());
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    rc = traverse(node->get_update());
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    rc = traverse(node->get_body());
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    leave_scope();
    END_VISIT();
}

int SymbolAnalysisPass::visit(const ReturnStatementASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const BreakStatementASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const ContinueStatementASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

int SymbolAnalysisPass::visit(const StatementBlockASTNodePtr &node)
{
    // Implement visit logic for StatementBlockASTNode
    const ASTNodePtr &parent = topmost_node();
    // is it from while/if/for/function/subroutine which is scope structured?
    auto parent_type = parent->get_node_type();
    bool come_from_scoped_structure
        = parent_type == parser::ASTNodeType::WhileStatement
        || parent_type == parser::ASTNodeType::IfStatement
        || parent_type == parser::ASTNodeType::ForStatement
        || parent_type == parser::ASTNodeType::FunctionDefinition
        || parent_type == parser::ASTNodeType::SubprocDefinition;

    BEGIN_VISIT();

    // if entered from while/if/for, skip the new scope since these stmts entered already

    if (!come_from_scoped_structure) {
        enter_anonymous_scope();
        // re-attach required because we entered a scope
        attach_scope_id(node);
    }

    rc = traverse(node->get_statements());
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    if (!come_from_scoped_structure) {
        leave_scope();
    }

    END_VISIT();
}

int SymbolAnalysisPass::visit(const SubprocDefinitionASTNodePtr &node)
{
    // Implement visit logic for SubprocDefinitionASTNode
    return visit_subroutine(node, false);
}

int SymbolAnalysisPass::visit(const FunctionDefinitionASTNodePtr &node)
{
    // Implement visit logic for FunctionDefinitionASTNode
    return visit_subroutine(node, true);
}

int SymbolAnalysisPass::visit(const CompilationUnitASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
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
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    enter_scope(function_name, ScopeType::Subroutine);

    rc = traverse(param);
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    rc = traverse(node->get_body());
    SET_RESULT_RC_AND_RETURN_IN_VISIT();

    // pop them up, and set the return value!
    leave_scope();

    END_VISIT();
}

void SymbolAnalysisPass::enter_node(const parser::ASTNodePtr &node)
{
    SemanticAnalysisPass::enter_node(node);
    attach_scope_id(node);
}

CLOSE_SEMANALYZER_NAMESPACE
// end
