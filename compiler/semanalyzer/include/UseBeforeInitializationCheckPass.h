#ifndef USEBEFOREINITIALIZATIONPASS_H
#define USEBEFOREINITIALIZATIONPASS_H

#include "ASTNodeForward.h"
#include "SemanticAnalysisPass.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

class UseBeforeInitializationCheckPass : public SemanticAnalysisPass {
public:
    UseBeforeInitializationCheckPass(StringPtr filename, parser::CompilationUnitASTNodePtr root)
        : SemanticAnalysisPass(std::move(filename), std::move(root))
    {
    }

    ~UseBeforeInitializationCheckPass() = default;

    // For all visit, the return value of 0 indicate success.
    int visit(parser::VariableDeclarationASTNodePtr node) override;
    int visit(parser::VariableAssignmentASTNodePtr node) override;
    int visit(parser::VariableAccessASTNodePtr node) override;
    int visit(parser::FloorBoxInitStatementASTNodePtr node) override;
    int visit(parser::FloorAssignmentASTNodePtr node) override;
    int visit(parser::FloorAccessASTNodePtr node) override;
    int visit(parser::NegativeExpressionASTNodePtr node) override;
    int visit(parser::NotExpressionASTNodePtr node) override;
    int visit(parser::IncrementExpressionASTNodePtr node) override;
    int visit(parser::DecrementExpressionASTNodePtr node) override;
    int visit(parser::IfStatementASTNodePtr node) override;
    int visit(parser::WhileStatementASTNodePtr node) override;
    int visit(parser::ForStatementASTNodePtr node) override;
    int visit(parser::StatementBlockASTNodePtr node) override;
    int visit(parser::SubprocDefinitionASTNodePtr node) override;
    int visit(parser::FunctionDefinitionASTNodePtr node) override;
    int visit(parser::CompilationUnitASTNodePtr node) override;

    /**
     * @brief Set the symbol table object
     *
     * @param symbol_table  The existing table. This can be useful when the program has imports.
     */
    void set_symbol_table(SymbolTablePtr &symbol_table) { _symbol_table = symbol_table; }

    const SymbolTablePtr &get_symbol_table() const { return _symbol_table; }

protected:
    void enter_node(parser::ASTNodePtr node) override;

    void leave_node() override;

private:
    SymbolTablePtr _symbol_table;

    // [Group] var use before init check
    using NodeResult = std::map<SymbolPtr, int>;
    std::map<SymbolPtr, std::stack<int>> _varinit_record_stacks;
    std::map<parser::ASTNodePtr, NodeResult> _varinit_record_results;
    std::string _last_scope_id;

    // if the entered a scope: copy all elements from parent's scope
    // if left the scope: return result to parent's scope
    // if scope is not changed: do nothing
    void update_varinit_record_stack_on_scope_change(const parser::ASTNodePtr &node);

    void create_var_init_at_current_scope(const SymbolPtr &symbol);
    void set_var_init_at_current_scope(const SymbolPtr &symbol, int initialized);
    int get_var_init_at_current_scope(const SymbolPtr &symbol);
    void get_var_init_result(const parser::ASTNodePtr &node_to_get_result, NodeResult &result);

    void log_use_before_initialization_error(const SymbolPtr &symbol, const parser::ASTNodePtr &node);
    // [End]
};

CLOSE_SEMANALYZER_NAMESPACE

#endif