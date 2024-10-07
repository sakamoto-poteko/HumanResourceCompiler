#ifndef AST_INTERPRETER_H
#define AST_INTERPRETER_H

#include <list>

#include "ASTNodeForward.h"
#include "AbstractInterpreter.h"
#include "InterpreterAccumulator.h"
#include "InterpreterIOManager.h"
#include "InterpreterMemoryManager.h"
#include "SemanticAnalysisPass.h"
#include "SymbolTable.h"
#include "WithSymbolTable.h"
#include "hrl_global.h"
#include "interpreter_global.h"

OPEN_INTERPRETER_NAMESPACE

class ASTInterpreter : public AbstractInterpreter, private semanalyzer::SemanticAnalysisPass, private semanalyzer::WithSymbolTable {
public:
    ASTInterpreter(
        StringPtr filename,
        parser::CompilationUnitASTNodePtr root,
        semanalyzer::SymbolTablePtr symbol_table,
        InterpreterIOManager &ioman,
        InterpreterMemoryManager &memman)
        : AbstractInterpreter(ioman, memman)
        , semanalyzer::SemanticAnalysisPass(std::move(filename), std::move(root))
    {
        semanalyzer::WithSymbolTable::set_symbol_table(symbol_table);
    }

    ~ASTInterpreter() = default;

    int exec() override;

private:
    enum ControlFlowState : int {
        CF_Normal = 0,
        CF_ReturnRequested = -1,
        CF_ContinueRequested = -2,
        CF_BreakRequested = -3,
    };

    InterpreterAccumulator _accumulator;

    void ensure_non_zero(int value);

    int invoke_library_function(const std::string &name);
    int invoke_inbox();
    int invoke_outbox();

    // [Group] SemanticAnalysisPass
    int run() override;

    void enter_node(const parser::ASTNodePtr &node) override;
    void leave_node() override;

    int visit(const parser::IntegerASTNodePtr &node) override;
    int visit(const parser::BooleanASTNodePtr &node) override;
    int visit(const parser::VariableDeclarationASTNodePtr &node) override;
    int visit(const parser::VariableAssignmentASTNodePtr &node) override;
    int visit(const parser::VariableAccessASTNodePtr &node) override;
    int visit(const parser::FloorBoxInitStatementASTNodePtr &node) override;
    int visit(const parser::FloorAssignmentASTNodePtr &node) override;
    int visit(const parser::FloorAccessASTNodePtr &node) override;
    int visit(const parser::NegativeExpressionASTNodePtr &node) override;
    int visit(const parser::NotExpressionASTNodePtr &node) override;
    int visit(const parser::IncrementExpressionASTNodePtr &node) override;
    int visit(const parser::DecrementExpressionASTNodePtr &node) override;
    int visit(const parser::AddExpressionASTNodePtr &node) override;
    int visit(const parser::SubExpressionASTNodePtr &node) override;
    int visit(const parser::MulExpressionASTNodePtr &node) override;
    int visit(const parser::DivExpressionASTNodePtr &node) override;
    int visit(const parser::ModExpressionASTNodePtr &node) override;
    int visit(const parser::EqualExpressionASTNodePtr &node) override;
    int visit(const parser::NotEqualExpressionASTNodePtr &node) override;
    int visit(const parser::GreaterThanExpressionASTNodePtr &node) override;
    int visit(const parser::GreaterEqualExpressionASTNodePtr &node) override;
    int visit(const parser::LessThanExpressionASTNodePtr &node) override;
    int visit(const parser::LessEqualExpressionASTNodePtr &node) override;
    int visit(const parser::AndExpressionASTNodePtr &node) override;
    int visit(const parser::OrExpressionASTNodePtr &node) override;
    int visit(const parser::InvocationExpressionASTNodePtr &node) override;
    int visit(const parser::EmptyStatementASTNodePtr &node) override;
    int visit(const parser::IfStatementASTNodePtr &node) override;
    int visit(const parser::WhileStatementASTNodePtr &node) override;
    int visit(const parser::ForStatementASTNodePtr &node) override;
    int visit(const parser::ReturnStatementASTNodePtr &node) override;
    int visit(const parser::BreakStatementASTNodePtr &node) override;
    int visit(const parser::ContinueStatementASTNodePtr &node) override;
    int visit(const parser::StatementBlockASTNodePtr &node) override;
    int visit(const parser::SubprocDefinitionASTNodePtr &node) override;
    int visit(const parser::FunctionDefinitionASTNodePtr &node) override;
    int visit(const parser::CompilationUnitASTNodePtr &node) override;

    int visit_binary_expression(const parser::AbstractBinaryExpressionASTNodePtr &node);
    int visit_subroutine(const parser::AbstractSubroutineASTNodePtr &node);

    // [End Group]

    struct CallFrame {
        parser::AbstractSubroutineASTNodePtr ast_node;
        std::map<semanalyzer::SymbolPtr, HRMByte> variables;
    };

    void set_variable(const semanalyzer::SymbolPtr &symbol, HRMByte value);
    bool get_variable(const semanalyzer::SymbolPtr &symbol, HRMByte &value);
    std::map<semanalyzer::SymbolPtr, HRMByte> _global_variables;
    std::list<CallFrame> _call_stack;
};

CLOSE_INTERPRETER_NAMESPACE

#endif
