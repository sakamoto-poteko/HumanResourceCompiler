#ifndef CONTROLFLOWVERIFICATIONPASS_H
#define CONTROLFLOWVERIFICATIONPASS_H

#include <stack>
#include <vector>

#include <boost/format.hpp>
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
 - [ ] Validate that all control paths in non-void functions return a value.
 - [ ] Verify correct usage of `break`, `continue`, and `return` statements.
 - [ ] Ensure that all code paths in a function lead to a valid return statement.
 - [ ] Check that all functions with a return statement actually return a value
 *
 */
class ControlFlowVerificationPass : public SemanticAnalysisPass {
public:
    ControlFlowVerificationPass(StringPtr filename, parser::CompilationUnitASTNodePtr root)
        : SemanticAnalysisPass(std::move(filename), std::move(root))
    {
    }

    ~ControlFlowVerificationPass() = default;

    int run() override;
    void enter_node(const parser::ASTNodePtr &node) override;
    void leave_node() override;

    // For all visit, the return value of 0 indicate success.
    // int visit(const parser::IfStatementASTNodePtr &node) override;
    // int visit(const parser::WhileStatementASTNodePtr &node) override;
    // int visit(const parser::ForStatementASTNodePtr &node) override;
    // int visit(const parser::ReturnStatementASTNodePtr &node) override;
    int visit(const parser::BreakStatementASTNodePtr &node) override;
    int visit(const parser::ContinueStatementASTNodePtr &node) override;
    // int visit(const parser::StatementBlockASTNodePtr &node) override;
    // int visit(const parser::SubprocDefinitionASTNodePtr &node) override;
    // int visit(const parser::FunctionDefinitionASTNodePtr &node) override;

private:
    std::vector<parser::ASTNodeType> _ancestor_types;
    std::stack<bool> _subroutine_requires_return;

    int check_loop_control_statements(const parser::ASTNodePtr &node);

    void log_invalid_loop_control_statement_error(const parser::ASTNodePtr &node, const std::string &msg);
};

CLOSE_SEMANALYZER_NAMESPACE

#endif