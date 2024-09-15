#include <boost/format.hpp>

#include "ASTNode.h"
#include "ControlFlowVerificationPass.h"
#include "ErrorManager.h"
#include "ErrorMessage.h"
#include "SemanticAnalysisErrors.h"
#include "SemanticAnalysisPass.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

#define BEGIN_VISIT()       \
    enter_node(node);       \
    int result = 0, rc = 0; \
    UNUSED(rc)

#define END_VISIT() \
    leave_node();   \
    return result

#define RETURN_IF_FAIL_IN_VISIT() \
    if (rc != 0) {                \
        result = rc;              \
        END_VISIT();              \
    }

int ControlFlowVerificationPass::run()
{
    _ancestor_types.clear();
    while (!_subroutine_requires_return.empty()) {
        _subroutine_requires_return.pop();
    }
    return SemanticAnalysisPass::visit(_root);
}

void ControlFlowVerificationPass::enter_node(const parser::ASTNodePtr &node)
{
    SemanticAnalysisPass::enter_node(node);
    _ancestor_types.push_back(node->get_node_type());
}

void ControlFlowVerificationPass::leave_node()
{
    _ancestor_types.pop_back();
    SemanticAnalysisPass::leave_node();
}

// int ControlFlowVerificationPass::visit(const parser::IfStatementASTNodePtr &node)
// {
//     BEGIN_VISIT();
//     END_VISIT();
// }

// int ControlFlowVerificationPass::visit(const parser::WhileStatementASTNodePtr &node)
// {
//     BEGIN_VISIT();
//     END_VISIT();
// }

// int ControlFlowVerificationPass::visit(const parser::ForStatementASTNodePtr &node)
// {
//     BEGIN_VISIT();
//     END_VISIT();
// }

// int ControlFlowVerificationPass::visit(const parser::ReturnStatementASTNodePtr &node)
// {
//     BEGIN_VISIT();
//     END_VISIT();
// }

int ControlFlowVerificationPass::visit(const parser::BreakStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    rc = check_loop_control_statements(node);
    RETURN_IF_FAIL_IN_VISIT();

    END_VISIT();
}

int ControlFlowVerificationPass::visit(const parser::ContinueStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    rc = check_loop_control_statements(node);
    RETURN_IF_FAIL_IN_VISIT();

    END_VISIT();
}

// int ControlFlowVerificationPass::visit(const parser::StatementBlockASTNodePtr &node)
// {
//     BEGIN_VISIT();
//     END_VISIT();
// }

// int ControlFlowVerificationPass::visit(const parser::SubprocDefinitionASTNodePtr &node)
// {
//     BEGIN_VISIT();
//     END_VISIT();
// }

// int ControlFlowVerificationPass::visit(const parser::FunctionDefinitionASTNodePtr &node)
// {
//     BEGIN_VISIT();
//     END_VISIT();
// }

int ControlFlowVerificationPass::check_loop_control_statements(const parser::ASTNodePtr &node)
{
    // Search the ancestors from top to bottom. The first for/while is what we break or continue
    // Ensure there's no function/sub in our search.
    for (auto it = _ancestors.rbegin(); it != _ancestors.rend(); ++it) {
        const parser::ASTNodePtr &ancestor_node = *it;
        auto ancestor_type = ancestor_node->get_node_type();
        if (ancestor_type == parser::ASTNodeType::WhileStatement || ancestor_type == parser::ASTNodeType::ForStatement) {
            // We hit our loop to break or exit. Attach the metadata
            auto attr = std::make_shared<ControlFlowAttribute>(ancestor_node);
            attr->set_to(node);
            return 0;
        }
        // this isn't necessary. our ebnf ensures that there's no function/sub inside a loop, but let's check anyway
        if (ancestor_type == parser::ASTNodeType::FunctionDefinition || ancestor_type == parser::ASTNodeType::SubprocDefinition) {
            break;
        }
    }

    log_invalid_loop_control_statement_error(node, "can only be used inside a loop (for/while).");
    return E_SEMA_INVALID_LOOP_CONTROL_CONTEXT;
}

void ControlFlowVerificationPass::log_invalid_loop_control_statement_error(const parser::ASTNodePtr &node, const std::string &cond_msg)
{
    const auto stmt_type = node->get_node_type() == parser::ASTNodeType::BreakStatement ? "break" : "for";
    auto errstr = boost::format("Invalid '%1%' statement. '%1%' %2%") % stmt_type % cond_msg;
    ErrorManager::instance().report(
        E_SEMA_INVALID_LOOP_CONTROL_CONTEXT,
        ErrorSeverity::Error,
        ErrorLocation(_filename, node->lineno(), node->colno(), 0),
        errstr.str());
}

CLOSE_SEMANALYZER_NAMESPACE
// end
