#include <functional>
#include <optional>
#include <string>

#include <boost/format.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/directed_graph.hpp>
#include <boost/graph/graphviz.hpp>

#include "ASTNode.h"
#include "ASTNodeForward.h"
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
    UNUSED(rc);             \
    // CFRGVertex node_vertex = _return_graph_traversal_history.top(); \
    // UNUSED(node_vertex)

#define END_VISIT() \
    leave_node();   \
    return result

#define RETURN_IF_FAIL_IN_VISIT() \
    if (rc != 0) {                \
        result = rc;              \
        END_VISIT();              \
    }

ControlFlowVerificationPass::ControlFlowVerificationPass(StringPtr filename, parser::CompilationUnitASTNodePtr root)
    : SemanticAnalysisPass(std::move(filename), std::move(root))
{
}

int ControlFlowVerificationPass::run()
{
    while (!_subroutine_requires_return.empty()) {
        _subroutine_requires_return.pop();
    }

    CFRGVertex root_vertex = _control_flow_return_graph.add_vertex(ControlFlowInfo(false, nullptr));
    _return_graph_traversal_history.push(root_vertex);
    // _previous_return_graph_nodes.push(root_vertex);

    return visit(_root);
}

ControlFlowVerificationPass::CFRGVertex ControlFlowVerificationPass::enter_return_node_branch(const parser::ASTNodePtr &node)
{
    // auto last_vert = _return_graph_traversal_history.top();
    // inherit the parent's color
    CFRGVertex vertex = _control_flow_return_graph.add_vertex(ControlFlowInfo(false, node));
    // while (!_previous_return_graph_nodes.empty()) {
    //     CFRGVertex previous_vertex = _previous_return_graph_nodes.top();
    //     _previous_return_graph_nodes.pop();
    //     // Return marks an end of flow
    //     if (!_control_flow_return_graph[previous_vertex].is_returned) {
    //         _control_flow_return_graph.add_edge(previous_vertex, vertex);
    //     }
    // }

    _return_graph_traversal_history.push(vertex);
    // _previous_return_graph_nodes.push(vertex);

    return vertex;
}

void ControlFlowVerificationPass::enter_node(const parser::ASTNodePtr &node)
{
    SemanticAnalysisPass::enter_node(node);
}

ControlFlowVerificationPass::CFRGVertex ControlFlowVerificationPass::leave_return_node_branch()
{
    auto vertex = _return_graph_traversal_history.top();
    // _ast_node_to_return_node[_ancestors.back()] = vertex;
    _return_graph_traversal_history.pop();
    return vertex;
}

void ControlFlowVerificationPass::leave_node()
{
    SemanticAnalysisPass::leave_node();
}

int ControlFlowVerificationPass::visit(const parser::IfStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    auto then_branch = node->get_then_branch();
    auto else_branch = node->get_else_branch();

    std::optional<bool> then_returned, else_returned;

    if (then_branch) {
        push_return_record();
        rc = traverse(then_branch);
        RETURN_IF_FAIL_IN_VISIT();
        then_returned = get_return_record();
        pop_return_record();
    }

    if (else_branch) {
        // _previous_return_graph_nodes.top() = node_vertex;
        enter_return_node_branch(node);
        push_return_record();
        rc = traverse(else_branch);
        RETURN_IF_FAIL_IN_VISIT();
        else_returned = get_return_record();
        pop_return_record();
    }

    if (then_returned == true && else_returned == true) {
        set_return_record(true);
    }

    END_VISIT();
}

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

// Three checks:
// Add return context info.
// Mark returned.
// Check return type.
int ControlFlowVerificationPass::visit(const parser::ReturnStatementASTNodePtr &node)
{
    BEGIN_VISIT();

    // 1. Add context info.
    // Search the ancestors from top to bottom. The first sub or function is what we return
    bool found_context = false;
    parser::AbstractSubroutineASTNodePtr subroutine_node = nullptr;

    for (auto it = _ancestors.rbegin(); it != _ancestors.rend(); ++it) {
        const parser::ASTNodePtr &ancestor_node = *it;
        auto ancestor_type = ancestor_node->get_node_type();
        if (ancestor_type == parser::ASTNodeType::FunctionDefinition || ancestor_type == parser::ASTNodeType::SubprocDefinition) {
            auto attr = std::make_shared<ControlFlowAttribute>(ancestor_node);
            attr->set_to(node);
            found_context = true;
            break;
        }
    }

    // no context found?
    if (!found_context) {
        rc = log_invalid_return_context_error(node);
        RETURN_IF_FAIL_IN_VISIT();
    }

    // 2. Mark return graph's node's color
    set_return_record(true);

    // 3. Check if returns a value
    bool has_return = node->get_expression().operator bool();
    if (_expected_return != has_return) {
        rc = log_invalid_return_value_error(*subroutine_node->get_name(), node, _expected_return);
        RETURN_IF_FAIL_IN_VISIT();
    }

    END_VISIT();
}

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

int ControlFlowVerificationPass::visit(const parser::SubprocDefinitionASTNodePtr &node)
{
    return visit_subroutine(node, false);
}

int ControlFlowVerificationPass::visit(const parser::FunctionDefinitionASTNodePtr &node)
{
    return visit_subroutine(node, true);
}

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

    return log_invalid_loop_control_context_error(node);
}

int ControlFlowVerificationPass::log_invalid_loop_control_context_error(const parser::ASTNodePtr &node)
{
    const auto stmt_type = node->get_node_type() == parser::ASTNodeType::BreakStatement ? "break" : "for";
    auto errstr = boost::format("Invalid '%1%' statement. '%1%' can only be used inside a loop (for/while).") % stmt_type;
    ErrorManager::instance().report(
        E_SEMA_INVALID_LOOP_CONTROL_CONTEXT,
        ErrorSeverity::Error,
        ErrorLocation(_filename, node->lineno(), node->colno(), 0),
        errstr.str());
    return E_SEMA_INVALID_LOOP_CONTROL_CONTEXT;
}

int ControlFlowVerificationPass::log_invalid_return_context_error(const parser::ASTNodePtr &node)
{
    ErrorManager::instance().report(
        E_SEMA_INVALID_RETURN_CONTEXT,
        ErrorSeverity::Error,
        ErrorLocation(_filename, node->lineno(), node->colno(), 0),
        "Invalid 'return' statement. 'return' can only be used inside a function or subprocedure.");
    return E_SEMA_INVALID_RETURN_CONTEXT;
}

int ControlFlowVerificationPass::log_invalid_return_value_error(const std::string &func_name, const parser::ReturnStatementASTNodePtr &return_node, bool expect_value)
{
    if (expect_value) {
        auto errstr
            = boost::format("Missing return value in function '%1%'. The function '%1%' is expected to return a value, but no value is returned.")
            % func_name;
        ErrorManager::instance().report(
            E_SEMA_EXPECT_RETURN_VALUE_BUT_NO,
            ErrorSeverity::Error,
            ErrorLocation(_filename, return_node->lineno(), return_node->colno(), 0),
            errstr.str());
        return E_SEMA_EXPECT_RETURN_VALUE_BUT_NO;
    } else {
        auto errstr
            = boost::format("Invalid return value in subprocedure '%1%'. The subprocedure '%1%' cannot return a value, but a return value is provided.")
            % func_name;
        ErrorManager::instance().report(
            E_SEMA_EXPECT_RETURN_NO_VAL_BUT_GIVEN,
            ErrorSeverity::Error,
            ErrorLocation(_filename, return_node->lineno(), return_node->colno(), 0),
            errstr.str());
        return E_SEMA_EXPECT_RETURN_NO_VAL_BUT_GIVEN;
    }
}

int hrl::semanalyzer::ControlFlowVerificationPass::log_not_all_path_return_error(const std::string &func_name, const parser::ASTNodePtr &first_block_node)
{
    auto errstr = boost::format("Not all code paths in function '%1%' return a value.") % func_name;
    ErrorManager::instance().report(
        E_SEMA_NOT_ALL_PATH_RETURN_VALUE,
        ErrorSeverity::Error,
        ErrorLocation(_filename, first_block_node->lineno(), first_block_node->colno(), 0),
        errstr.str());
    return E_SEMA_NOT_ALL_PATH_RETURN_VALUE;
}

int ControlFlowVerificationPass::visit_subroutine(const parser::AbstractSubroutineASTNodePtr &node, bool expect_return)
{
    BEGIN_VISIT();

    _expected_return = expect_return;

    push_return_record();
    rc = traverse(node->get_body());
    RETURN_IF_FAIL_IN_VISIT();
    auto returned = get_return_record();
    pop_return_record();

    // only check the function (which expects return)
    if (_expected_return && !returned) {
        rc = E_SEMA_NOT_ALL_PATH_RETURN_VALUE;
        RETURN_IF_FAIL_IN_VISIT();
    }

    END_VISIT();
}

int ControlFlowVerificationPass::visit(const parser::StatementBlockASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
    // BEGIN_VISIT();

    // rc = traverse(node->get_statements(), [this, &node_vertex](const parser::ASTNodePtr &) {
    //     _current_return_graph_node = node_vertex;
    // });
    // RETURN_IF_FAIL_IN_VISIT();

    // END_VISIT();
}

int ControlFlowVerificationPass::visit(const parser::CompilationUnitASTNodePtr &node)
{
    return SemanticAnalysisPass::visit(node);
}

bool ControlFlowVerificationPass::generate_return_graph(const std::string &dot_path)
{
    std::stringstream dotfile;
    boost::write_graphviz(
        dotfile,
        _control_flow_return_graph,
        // vertex
        [this](std::ostream &out, CFRGVertex &v) {
            const auto &vert = _control_flow_return_graph[v];
            out << "[label=";
            if (vert.node) {
                out << "\"(" << parser::ast_node_type_to_string(vert.node->get_node_type()) << ")" << vert.node->lineno() << ":" << vert.node->colno() << ",\\n";
                if (!vert.msg.empty()) {
                    out << vert.msg << "\\n";
                }
                out << (vert.is_returned ? "RETURNED" : "N/A") << "\"";
            } else {
                out << "\"(nullptr)\\n"
                    << (vert.is_returned ? "RETURNED" : "N/A") << "\"";
            }
            out << "]";
        },
        // edge
        [](std::ostream &out, const auto &e) {
            //
            UNUSED(out);
            UNUSED(e);
        },
        // graph
        [](std::ostream &out) { out << "node[ordering=out];\n"; });

    std::ofstream out(dot_path);
    if (out) {
        out << dotfile.str();
        out.close();
        return true;
    } else {
        return false;
    }
}

CLOSE_SEMANALYZER_NAMESPACE
// end
