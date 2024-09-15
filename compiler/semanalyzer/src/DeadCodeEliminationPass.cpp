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
    return rc;

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

int DeadCodeEliminationPass::visit(const parser::IfStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    // if condition is evaluated to a constant
    const auto &condition = node->get_condition();
    const auto &evaluated_condition = ConstantFoldingAttribute::get_from(condition);
    if (evaluated_condition) {
        if (evaluated_condition->get_value()) {
            // it's an if (true)
            // remove else branch
            // traverse then branch
            // pull then branch up node itself
            auto &else_branch = node->get_else_branch();
            if (else_branch) {
                report_dead_code(else_branch, condition, DeadCodeReason::ConstantTrue);
                else_branch = nullptr;
            }
            auto &then_branch = node->get_then_branch();
            rc = traverse(then_branch);
            RETURN_IF_FAIL_IN_VISIT(rc);
            if (then_branch) {
                request_to_replace_self(then_branch);
            }
        } else {
            // it's an if (false)
            // we'll check if there's an else branch, since it's the only one get executed
            // if there is, we pull it up as node itself
            // if there is not, this if is empty
            report_dead_code(node->get_then_branch(), condition, DeadCodeReason::ConstantFalse);
            // node->get_then_branch() is always not null
            auto &else_branch = node->get_else_branch();
            if (else_branch) {
                rc = traverse(else_branch);
                RETURN_IF_FAIL_IN_VISIT(rc);
                request_to_replace_self(else_branch);
            } else {
                request_to_remove_self();
            }
        }
    } else {
        rc = traverse_multiple(node->get_condition(), node->get_then_branch(), node->get_else_branch());
        RETURN_IF_FAIL_IN_VISIT(rc);
    }

    END_VISIT();
}

int DeadCodeEliminationPass::visit(const parser::WhileStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    const auto &evaluated_condition = ConstantFoldingAttribute::get_from(node->get_condition());
    // if condition is evaluated to a constant
    if (evaluated_condition) {
        if (evaluated_condition->get_value()) {
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

int DeadCodeEliminationPass::visit(const parser::StatementBlockASTNodePtr &node)
{
    BEGIN_VISIT();

    parser::StatementsVector &statements = node->get_statements();
    for (auto statement_it = statements.begin(); statement_it != statements.end(); ++statement_it) {
        const auto &current_stmt = *statement_it;
        auto stmt_type = current_stmt->get_node_type();
        bool is_end_of_control_flow_statement
            = stmt_type == parser::ASTNodeType::ReturnStatement
            || stmt_type == parser::ASTNodeType::BreakStatement
            || stmt_type == parser::ASTNodeType::ContinueStatement;

        if (is_end_of_control_flow_statement) {
            auto begin_unreachable = std::next(statement_it);
            report_dead_code(*begin_unreachable, *std::prev(statements.end()), current_stmt, DeadCodeReason::EndOfFlow);
            // WARN: erase invalidates iterators and must return
            statements.erase(begin_unreachable, statements.end());
            break;
        }

        rc = traverse(*statement_it);
        if (rc != 0) {
            // fail fast. erase if and return rc
            break;
        }

        if (*statement_it == nullptr) {
            // traversal may make it null
            continue;
        }

        // After traversal, we check if current statement is a while(true);
        auto next_stmt_it = std::next(statement_it);
        if (_while_true_statements.contains(*statement_it) && next_stmt_it != statements.end()) {
            report_dead_code(*next_stmt_it, *std::prev(statements.end()), current_stmt, DeadCodeReason::AfterInfiniteLoop);
            // WARN: erase invalidates iterators and must return
            statements.erase(next_stmt_it, statements.end());
            break;
        }
    }

    std::erase_if(statements, [](const auto &ptr) { return !ptr; });
    END_VISIT();
}

void DeadCodeEliminationPass::report_dead_code(const parser::ASTNodePtr &begin_node, const parser::ASTNodePtr &end_node, const parser::ASTNodePtr &reason_node, DeadCodeReason reason)
{
    auto warning_msg = boost::format(
                           "Dead code detected from line %1% to line %2%. Code is unreachable due to '%3%' of line %4%.")
        % begin_node->lineno() % end_node->lineno() % dead_code_reason_to_str(reason) % reason_node->lineno();
    ErrorManager::instance().report(
        W_SEMA_DEAD_CODE,
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
        W_SEMA_DEAD_CODE,
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
    throw;
}

CLOSE_SEMANALYZER_NAMESPACE
// end
