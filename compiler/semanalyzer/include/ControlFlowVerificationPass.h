#ifndef CONTROLFLOWVERIFICATIONPASS_H
#define CONTROLFLOWVERIFICATIONPASS_H

#include <stack>
#include <string>

#include <boost/format.hpp>
#include <spdlog/spdlog.h>

#include "ASTNode.h"
#include "ASTNodeAttribute.h"
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
                spdlog::critical("Invalid node type seen in ControlFlowAttribute. {}", __PRETTY_FUNCTION__);
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

class ControlFlowVerificationPass : public SemanticAnalysisPass {
public:
    ControlFlowVerificationPass(StringPtr filename, parser::CompilationUnitASTNodePtr root);

    ~ControlFlowVerificationPass() = default;

    int run() override;

    // For all visit, the return value of 0 indicate success.
    int visit(const parser::IfStatementASTNodePtr &node) override;
    int visit(const parser::WhileStatementASTNodePtr &node) override;
    int visit(const parser::ForStatementASTNodePtr &node) override;
    int visit(const parser::ReturnStatementASTNodePtr &node) override;
    int visit(const parser::BreakStatementASTNodePtr &node) override;
    int visit(const parser::ContinueStatementASTNodePtr &node) override;
    // int visit(const parser::StatementBlockASTNodePtr &node) override;
    int visit(const parser::SubprocDefinitionASTNodePtr &node) override;
    int visit(const parser::FunctionDefinitionASTNodePtr &node) override;
    // int visit(const parser::CompilationUnitASTNodePtr &node) override;

private:
    std::stack<bool> _subroutine_requires_return;
    std::stack<bool> _returned_record_stack;
    void push_return_record();
    void set_return_record(bool returned);
    bool get_return_record();
    void pop_return_record();

    /* How to ensure each each ends with a return?
     * Push `is_returned` of false to the stack when enter a new function/sub
     * Push `is_returned` of previous to the stack when enter a new selection/loop structure
     * When encountered a return statement, set `is_returned` to true
     * When encountered a if statement, set `is_returned` to true iif both branches returns
     * When encountered a loop statement, ignore the loop's `is_returned`
     * Check `is_returned` at the end of function traversal
     * Pop the `is_returned` from stack
     */
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