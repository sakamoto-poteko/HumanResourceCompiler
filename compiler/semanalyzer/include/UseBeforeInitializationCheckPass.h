#ifndef USEBEFOREINITIALIZATIONPASS_H
#define USEBEFOREINITIALIZATIONPASS_H

#include <set>
#include <stack>
#include <string>

#include "SemanticAnalysisPass.h"
#include "Symbol.h"
#include "WithScopeTracker.h"
#include "WithSymbolTable.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

class UseBeforeInitializationCheckPass : public SemanticAnalysisPass, public WithSymbolTable, public WithScopeTracker {
public:
    UseBeforeInitializationCheckPass(StringPtr filename, parser::CompilationUnitASTNodePtr root)
        : SemanticAnalysisPass(std::move(filename), std::move(root))
    {
    }

    ~UseBeforeInitializationCheckPass() = default;

    int run() override;

    // For all visit, the return value of 0 indicate success.
    int visit(const parser::VariableDeclarationASTNodePtr &node) override;
    int visit(const parser::VariableAssignmentASTNodePtr &node) override;
    int visit(const parser::VariableAccessASTNodePtr &node) override;
    int visit(const parser::IncrementExpressionASTNodePtr &node) override;
    int visit(const parser::DecrementExpressionASTNodePtr &node) override;
    int visit(const parser::IfStatementASTNodePtr &node) override;
    int visit(const parser::WhileStatementASTNodePtr &node) override;
    int visit(const parser::ForStatementASTNodePtr &node) override;
    int visit(const parser::StatementBlockASTNodePtr &node) override;
    int visit(const parser::SubprocDefinitionASTNodePtr &node) override;
    int visit(const parser::FunctionDefinitionASTNodePtr &node) override;

protected:
    void enter_node(const parser::ASTNodePtr &node) override;
    void leave_node() override;

private:
    // [Group] var use before init check
    using NodeResult = std::map<SymbolPtr, int>;
    // map<symbol, stack<initialized_in_scope>>
    std::map<SymbolPtr, std::stack<int>> _varinit_record_stacks;
    // map<node, result>
    std::map<parser::ASTNodePtr, NodeResult> _varinit_record_results;
    // scope stack<seen symbols>
    std::stack<std::set<SymbolPtr>> _var_occured;

    void create_var_init_at_current_scope(const SymbolPtr &symbol);
    void set_var_init_at_current_scope(const SymbolPtr &symbol, int initialized);
    void set_var_init_at_current_scope(const NodeResult &result);
    int get_var_init_at_current_scope(const SymbolPtr &symbol);
    NodeResult get_var_init_at_current_scope();
    void get_var_init_result(const parser::ASTNodePtr &node_to_get_result, NodeResult &result);

    void strip_symbols_beyond_scope(NodeResult &results, const std::string &scope_id);

    void log_use_before_initialization_error(const SymbolPtr &symbol, const parser::ASTNodePtr &node);
    // [End]

    int visit_subroutine(parser::AbstractSubroutineASTNodePtr node);
    int check_node_symbol_assigned_or_report(const parser::ASTNodePtr &node);

    // if the entered a scope: copy all elements from parent's scope
    // if left the scope: return result to parent's scope
    // if scope is not changed: do nothing
    void on_scope_enter(const parser::ASTNodePtr &node, const std::string &current_scope_id) override;
    void on_scope_exit(const parser::ASTNodePtr &node, const std::string &current_scope_id) override;
};

CLOSE_SEMANALYZER_NAMESPACE

#endif