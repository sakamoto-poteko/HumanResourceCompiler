#ifndef CONTROLFLOWVERIFICATIONPASS_H
#define CONTROLFLOWVERIFICATIONPASS_H

#include <queue>
#include <stack>
#include <string>
#include <vector>

#include <boost/format.hpp>
#include <boost/graph/directed_graph.hpp>
#include <spdlog/spdlog.h>

#include "ASTNode.h"
#include "ASTNodeAttribute.h"
#include "ASTNodeForward.h"
#include "SemanticAnalysisPass.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

class ControlFlowAttribute : public parser::ASTNodeAttribute, public parser::GetSetAttribute<ControlFlowAttribute> {
public:
    ControlFlowAttribute(const parser::ASTNodePtr &context_node)
        : _context(std::move(context_node))
    {
    }

    std::string to_string() override
    {
        std::string node_type;
        int lineno = 0, colno = 0;
        if (_context) {
            lineno = _context->lineno();
            colno = _context->colno();

            switch (_context->get_node_type()) {
            case parser::ASTNodeType::WhileStatement:
                node_type = "while";
                break;
            case parser::ASTNodeType::ForStatement:
                node_type = "for";
                break;
            case parser::ASTNodeType::SubprocDefinition:
                node_type = "sub";
                break;
            case parser::ASTNodeType::FunctionDefinition:
                node_type = "function";
                break;
            default:
                spdlog::error("Invalid node type seen in ControlFlowAttribute. {}", __PRETTY_FUNCTION__);
                throw;
            }
        } else {
            node_type = "(null)";
        }

        auto format = boost::format("<%1%> %2%:%3%") % node_type % lineno % colno;
        return format.str();
    }

    static int get_attribute_id() { return ATTR_SEMANALYZER_CONTROL_CONTEXT_INFO; }

    const parser::ASTNodePtr &get_context() const { return _context; }

private:
    parser::ASTNodePtr _context;
};

/**
 * @brief
 - [x] Verify correct usage of `break`, `continue` statements.
 - [ ] Ensure that all code paths in a function lead to a valid return statement and returns a value if required.
 *
 */
class ControlFlowVerificationPass : public SemanticAnalysisPass {
public:
    ControlFlowVerificationPass(StringPtr filename, parser::CompilationUnitASTNodePtr root);

    ~ControlFlowVerificationPass() = default;

    bool generate_return_graph(const std::string &dot_path);

    int run() override;
    void enter_node(const parser::ASTNodePtr &node) override;
    void leave_node() override;

    // For all visit, the return value of 0 indicate success.
    int visit(const parser::IfStatementASTNodePtr &node) override;
    // int visit(const parser::WhileStatementASTNodePtr &node) override;
    // int visit(const parser::ForStatementASTNodePtr &node) override;
    int visit(const parser::ReturnStatementASTNodePtr &node) override;
    int visit(const parser::BreakStatementASTNodePtr &node) override;
    int visit(const parser::ContinueStatementASTNodePtr &node) override;
    int visit(const parser::StatementBlockASTNodePtr &node) override;
    int visit(const parser::SubprocDefinitionASTNodePtr &node) override;
    int visit(const parser::FunctionDefinitionASTNodePtr &node) override;
    int visit(const parser::CompilationUnitASTNodePtr &node) override;

private:
    std::stack<bool> _subroutine_requires_return;

    /* How to ensure each each ends with a return?
     * A white node is created at the beginning of the subroutine.
     * When encountered a return statement, render the node black.
     * When encountered a if statement, diverge into two nodes, one in then, one in else. They merges back after if statement.
     * When encountered a loop statement, diverge into two nodes, one for loop execution, one skipping the loop. They merge back after the loop statement.
     * The merging only happens when both nodes are white. If one node is black, it becomes the leaf.
     * DFS at end of function definition. Find white leaf
     */
    struct ControlFlowInfo {
        bool is_returned;
        parser::ASTNodePtr node;
        std::string msg;

        ControlFlowInfo(bool is_returned, const parser::ASTNodePtr node)
            : is_returned(is_returned)
            , node(node)
        {
        }

        ControlFlowInfo(bool is_returned, const parser::ASTNodePtr node, const std::string &msg)
            : is_returned(is_returned)
            , node(node)
            , msg(msg)
        {
        }
    };

    using ControlFlowReturnedGraph = boost::directed_graph<ControlFlowInfo>;
    using CFRGVertex = ControlFlowReturnedGraph::vertex_descriptor;
    ControlFlowReturnedGraph _control_flow_return_graph;
    std::stack<CFRGVertex> _return_graph_traversal_history;
    CFRGVertex _current_return_graph_node; // only to be used in enter_node and leave_node, for edging purpose
    std::map<parser::ASTNodePtr, CFRGVertex> _ast_node_to_return_node;
    bool _expected_return; // subproc? function?

    int visit_subroutine(const parser::AbstractSubroutineASTNodePtr &node, bool expect_return);

    int check_loop_control_statements(const parser::ASTNodePtr &node);

    int log_invalid_loop_control_context_error(const parser::ASTNodePtr &node);
    int log_invalid_return_context_error(const parser::ASTNodePtr &node);
    int log_invalid_return_value_error(const std::string &func_name, const parser::ReturnStatementASTNodePtr &return_node, bool expect_value);
    int log_not_all_path_return_error(const std::string &func_name, const parser::ASTNodePtr &first_block_node);
};

CLOSE_SEMANALYZER_NAMESPACE

#endif