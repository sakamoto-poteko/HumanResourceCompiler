#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stack>

#include "ASTNodeForward.h"
#include "Accumulator.h"
#include "SemanticAnalysisPass.h"
#include "WithSymbolTable.h"
#include "hrint_global.h"
#include "hrl_global.h"

OPEN_HRINT_NAMESPACE

class Interpreter : public hrl::semanalyzer::SemanticAnalysisPass, public semanalyzer::WithSymbolTable {
public:
    Interpreter(StringPtr filename, hrl::parser::CompilationUnitASTNodePtr root, Accumulator &accumulator)
        : hrl::semanalyzer::SemanticAnalysisPass(std::move(filename), std::move(root))
        , _accumulator(accumulator)
    {
    }

    ~Interpreter();

    int run() override;

    int visit(const hrl::parser::IntegerASTNodePtr &node) override;
    int visit(const hrl::parser::BooleanASTNodePtr &node) override;
    int visit(const hrl::parser::VariableDeclarationASTNodePtr &node) override;
    int visit(const hrl::parser::VariableAssignmentASTNodePtr &node) override;
    int visit(const hrl::parser::VariableAccessASTNodePtr &node) override;
    int visit(const hrl::parser::FloorBoxInitStatementASTNodePtr &node) override;
    int visit(const hrl::parser::FloorAssignmentASTNodePtr &node) override;
    int visit(const hrl::parser::FloorAccessASTNodePtr &node) override;
    int visit(const hrl::parser::NegativeExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::NotExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::IncrementExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::DecrementExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::AddExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::SubExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::MulExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::DivExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::ModExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::EqualExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::NotEqualExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::GreaterThanExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::GreaterEqualExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::LessThanExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::LessEqualExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::AndExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::OrExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::InvocationExpressionASTNodePtr &node) override;
    int visit(const hrl::parser::EmptyStatementASTNodePtr &node) override;
    int visit(const hrl::parser::IfStatementASTNodePtr &node) override;
    int visit(const hrl::parser::WhileStatementASTNodePtr &node) override;
    int visit(const hrl::parser::ForStatementASTNodePtr &node) override;
    int visit(const hrl::parser::ReturnStatementASTNodePtr &node) override;
    int visit(const hrl::parser::BreakStatementASTNodePtr &node) override;
    int visit(const hrl::parser::ContinueStatementASTNodePtr &node) override;
    int visit(const hrl::parser::StatementBlockASTNodePtr &node) override;
    int visit(const hrl::parser::SubprocDefinitionASTNodePtr &node) override;
    int visit(const hrl::parser::FunctionDefinitionASTNodePtr &node) override;
    int visit(const hrl::parser::CompilationUnitASTNodePtr &node) override;

protected:
    void enter_node(const parser::ASTNodePtr &node) override;
    void leave_node() override;

private:
    Accumulator &_accumulator;

    enum ControlFlowState : int {
        CF_Normal = 0,
        CF_ReturnRequested = -1,
        CF_ContinueRequested = -2,
        CF_BreakRequested = -3,
    };

    int invoke_library_function(const std::string &name);

    int invoke_inbox();
    int invoke_outbox();

    int visit_binary_expression(const parser::AbstractBinaryExpressionASTNodePtr &node);
    void ensure_non_zero(int value);

    std::stack<parser::AbstractSubroutineASTNodePtr> _call_stack;
};

CLOSE_HRINT_NAMESPACE

#endif