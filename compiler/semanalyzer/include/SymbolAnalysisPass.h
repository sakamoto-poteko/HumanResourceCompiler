#ifndef SYMBOLTABLEBUILDER_H
#define SYMBOLTABLEBUILDER_H

#include <cassert>

#include <queue>
#include <string>

#include "ScopeManager.h"
#include "SemanticAnalysisPass.h"
#include "SymbolTable.h"
#include "WithSymbolTable.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

using namespace parser;

/**
 * @brief This builder traverses the AST to construct the symbol table, annotating the scope id of each node.
 * It also verifies performs signature check, variable shadowing check (warning)
 */
class SymbolAnalysisPass : public SemanticAnalysisPass, public WithSymbolTable {
public:
    /**
     * @brief Construct a new Symbol Table Builder object
     *
     * @param filename The filename of this compilation unit
     * @param root The root node of AST
     */
    SymbolAnalysisPass(StringPtr filename, CompilationUnitASTNodePtr root)
        : SemanticAnalysisPass(std::move(filename), std::move(root))
    {
    }

    ~SymbolAnalysisPass() override = default;

    int run() override;

    // For all visit, the return value of 0 indicate success.
    int visit(const IntegerASTNodePtr &node) override;
    int visit(const BooleanASTNodePtr &node) override;
    int visit(const VariableDeclarationASTNodePtr &node) override;
    int visit(const VariableAssignmentASTNodePtr &node) override;
    int visit(const VariableAccessASTNodePtr &node) override;
    int visit(const FloorBoxInitStatementASTNodePtr &node) override;
    int visit(const FloorAssignmentASTNodePtr &node) override;
    int visit(const FloorAccessASTNodePtr &node) override;
    int visit(const NegativeExpressionASTNodePtr &node) override;
    int visit(const NotExpressionASTNodePtr &node) override;
    int visit(const IncrementExpressionASTNodePtr &node) override;
    int visit(const DecrementExpressionASTNodePtr &node) override;
    int visit(const AddExpressionASTNodePtr &node) override;
    int visit(const SubExpressionASTNodePtr &node) override;
    int visit(const MulExpressionASTNodePtr &node) override;
    int visit(const DivExpressionASTNodePtr &node) override;
    int visit(const ModExpressionASTNodePtr &node) override;
    int visit(const EqualExpressionASTNodePtr &node) override;
    int visit(const NotEqualExpressionASTNodePtr &node) override;
    int visit(const GreaterThanExpressionASTNodePtr &node) override;
    int visit(const GreaterEqualExpressionASTNodePtr &node) override;
    int visit(const LessThanExpressionASTNodePtr &node) override;
    int visit(const LessEqualExpressionASTNodePtr &node) override;
    int visit(const AndExpressionASTNodePtr &node) override;
    int visit(const OrExpressionASTNodePtr &node) override;
    int visit(const InvocationExpressionASTNodePtr &node) override;
    int visit(const EmptyStatementASTNodePtr &node) override;
    int visit(const IfStatementASTNodePtr &node) override;
    int visit(const WhileStatementASTNodePtr &node) override;
    int visit(const ForStatementASTNodePtr &node) override;
    int visit(const ReturnStatementASTNodePtr &node) override;
    int visit(const BreakStatementASTNodePtr &node) override;
    int visit(const ContinueStatementASTNodePtr &node) override;
    int visit(const StatementBlockASTNodePtr &node) override;
    int visit(const SubprocDefinitionASTNodePtr &node) override;
    int visit(const FunctionDefinitionASTNodePtr &node) override;
    int visit(const CompilationUnitASTNodePtr &node) override;

protected:
    void enter_node(parser::ASTNodePtr node) override;

private:
    // SymbolTablePtr _symbol_table;
    ScopeManager _scope_manager;
    std::queue<InvocationExpressionASTNodePtr> _pending_invocation_check;

    // [Group] scopes
    void enter_anonymous_scope();
    void enter_scope(const StringPtr &name, ScopeType scope_type);
    void leave_scope();

    // [End]

    // [Group] Visit helpers
    int visit_subroutine(AbstractSubroutineASTNodePtr node, bool has_return);
    // [End]

    // [Group] Invocation symbol and signature check
    int check_pending_invocations();
    // [End]

    // [Group] Attaching node metadata
    int attach_symbol_or_log_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node);
    int add_subroutine_symbol_or_log_error(const StringPtr &name, bool has_param, bool has_return, const ASTNodePtr &node);
    int add_variable_symbol_or_log_error(const StringPtr &name, const ASTNodePtr &node);
    void attach_scope_id(const ASTNodePtr &node);
    // [End]

    // [Group] Log errors
    void log_redefinition_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node);
    void log_undefined_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node);
    // [End]

    bool lookup_symbol_with_ancestors(const StringPtr &name, SymbolPtr &out_symbol, std::string &out_def_scope);
    bool lookup_symbol_with_ancestors(const StringPtr &name, SymbolPtr &out_symbol);

private:
};

CLOSE_SEMANALYZER_NAMESPACE

#endif