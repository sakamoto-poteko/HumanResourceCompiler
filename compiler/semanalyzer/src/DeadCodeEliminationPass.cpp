#include <cassert>
#include <iterator>

#include "ASTNode.h"
#include "ConstantFoldingPass.h"
#include "DeadCodeEliminationPass.h"
#include "ErrorManager.h"
#include "ErrorMessage.h"
#include "SemanticAnalysisErrors.h"
#include "SemanticAnalysisPass.h"
#include "semanalyzer_global.h"

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

int DeadCodeEliminationPass::run()
{
    return SemanticAnalysisPass::visit(_root);
}

int DeadCodeEliminationPass::visit(const parser::EmptyStatementASTNodePtr &node)
{
    BEGIN_VISIT();
    request_to_remove_self();
    END_VISIT();
}

// int DeadCodeEliminationPass::visit(const parser::IfStatementASTNodePtr &node)
// {
//     BEGIN_VISIT();
//     END_VISIT();
// }

int DeadCodeEliminationPass::visit(const parser::WhileStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    const auto &const_result = ConstantFoldingAttribute::get_from(node->get_condition());
    if (const_result) {
        if (const_result->get_value()) {
            _while_true_statements.insert(node);
            // it's while (true);
        } else {
            report_dead_code(node->get_body(), node->get_condition(), DeadCodeReason::ConstantFalse);
            request_to_remove_self();
            // it's while (false). let's skip this one.
            END_VISIT();
        }
    }

    rc = traverse_multiple(node->get_condition(), node->get_body());
    RETURN_IF_FAIL_IN_VISIT(rc);

    END_VISIT();
}

// int DeadCodeEliminationPass::visit(const parser::ForStatementASTNodePtr &node)
// {
//     BEGIN_VISIT();
//     END_VISIT();
// }

int DeadCodeEliminationPass::visit(const parser::StatementBlockASTNodePtr &node)
{
    BEGIN_VISIT();

    parser::StatementsVector &statements = node->get_statements();
    for (auto stmt_it = statements.begin(); stmt_it != statements.end(); ++stmt_it) {
        const auto &current_stmt = *stmt_it;
        bool end_of_control_flow
            = parser::is_ptr_type<parser::ReturnStatementASTNode>(current_stmt)
            || parser::is_ptr_type<parser::BreakStatementASTNode>(current_stmt)
            || parser::is_ptr_type<parser::ContinueStatementASTNode>(current_stmt);

        if (end_of_control_flow) {
            auto begin_unreachable = std::next(stmt_it);
            report_dead_code(*begin_unreachable, *std::prev(statements.end()), current_stmt, DeadCodeReason::EndOfFlow);
            statements.erase(begin_unreachable, statements.end());
            break;
        }

        if (_while_true_statements.contains(*stmt_it)) {
            auto begin_unreachable = std::next(stmt_it);
            report_dead_code(*begin_unreachable, *std::prev(statements.end()), current_stmt, DeadCodeReason::AfterInfiniteLoop);
            statements.erase(begin_unreachable, statements.end());
            break;
        }

        rc = traverse(*stmt_it);
        RETURN_IF_FAIL_IN_VISIT(rc);
    }

    END_VISIT();
}

void DeadCodeEliminationPass::report_dead_code(const parser::ASTNodePtr &begin_node, const parser::ASTNodePtr &end_node, const parser::ASTNodePtr &reason_node, DeadCodeReason reason)
{
    auto warning_msg = boost::format(
                           "Dead code detected from line %1% to line %2%. Code is unreachable due to '%3%' of line %4%.")
        % begin_node->lineno() % end_node->lineno() % dead_code_reason_to_str(reason) % reason_node->lineno();
    ErrorManager::instance().report(
        E_SEMA_DEAD_CODE,
        ErrorSeverity::Warning,
        ErrorLocation(_filename, begin_node->lineno(), begin_node->colno(), -1),
        warning_msg.str());
}

void DeadCodeEliminationPass::report_dead_code(const parser::ASTNodePtr &single_node, const parser::ASTNodePtr &reason_node, DeadCodeReason reason)
{
    auto warning_msg = boost::format(
                           "Dead code detected on line %1%. Code is unreachable due to '%2%' of line %3%.")
        % single_node->lineno() % dead_code_reason_to_str(reason) % reason_node->lineno();
    ErrorManager::instance().report(
        E_SEMA_DEAD_CODE,
        ErrorSeverity::Warning,
        ErrorLocation(_filename, single_node->lineno(), single_node->colno(), -1),
        warning_msg.str());
}

const char *DeadCodeEliminationPass::dead_code_reason_to_str(DeadCodeReason reason)
{
    switch (reason) {
    case EndOfFlow:
        return "end of control flow";
    case ConstantFalse:
        return "constant false condition";
    case ConstantTrue:
        return "constant true condition";
    case AfterInfiniteLoop:
        return "after infinite loop";
    }
}

CLOSE_SEMANALYZER_NAMESPACE
// end
