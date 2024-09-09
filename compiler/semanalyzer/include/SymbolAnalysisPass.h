#ifndef SYMBOLTABLEBUILDER_H
#define SYMBOLTABLEBUILDER_H

#include <cassert>

#include <map>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include "ASTNode.h"
#include "ASTNodeForward.h"
#include "ASTNodeVisitor.h"
#include "ScopeManager.h"
#include "SemanticAnalysisPass.h"
#include "SymbolTable.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

using namespace parser;

/**
 * @brief This builder traverses the AST to construct the symbol table, annotating the scope id of each node.
 * It also verifies performs signature check, variable shadowing check (warning)
 */
class SymbolAnalysisPass : public SemanticAnalysisPass {
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

    /**
     * @brief Set the symbol table object
     *
     * @param symbol_table  The existing table. This can be useful when the program has imports.
     */
    void set_symbol_table(SymbolTablePtr &symbol_table) { _symbol_table = symbol_table; }

    const SymbolTablePtr &get_symbol_table() const { return _symbol_table; }

    // For all visit, the return value of 0 indicate success.
    int visit(IntegerASTNodePtr node) override;
    int visit(BooleanASTNodePtr node) override;
    int visit(VariableDeclarationASTNodePtr node) override;
    int visit(VariableAssignmentASTNodePtr node) override;
    int visit(VariableAccessASTNodePtr node) override;
    int visit(FloorBoxInitStatementASTNodePtr node) override;
    int visit(FloorAssignmentASTNodePtr node) override;
    int visit(FloorAccessASTNodePtr node) override;
    int visit(NegativeExpressionASTNodePtr node) override;
    int visit(NotExpressionASTNodePtr node) override;
    int visit(IncrementExpressionASTNodePtr node) override;
    int visit(DecrementExpressionASTNodePtr node) override;
    int visit(AddExpressionASTNodePtr node) override;
    int visit(SubExpressionASTNodePtr node) override;
    int visit(MulExpressionASTNodePtr node) override;
    int visit(DivExpressionASTNodePtr node) override;
    int visit(ModExpressionASTNodePtr node) override;
    int visit(EqualExpressionASTNodePtr node) override;
    int visit(NotEqualExpressionASTNodePtr node) override;
    int visit(GreaterThanExpressionASTNodePtr node) override;
    int visit(GreaterEqualExpressionASTNodePtr node) override;
    int visit(LessThanExpressionASTNodePtr node) override;
    int visit(LessEqualExpressionASTNodePtr node) override;
    int visit(AndExpressionASTNodePtr node) override;
    int visit(OrExpressionASTNodePtr node) override;
    int visit(InvocationExpressionASTNodePtr node) override;
    int visit(EmptyStatementASTNodePtr node) override;
    int visit(IfStatementASTNodePtr node) override;
    int visit(WhileStatementASTNodePtr node) override;
    int visit(ForStatementASTNodePtr node) override;
    int visit(ReturnStatementASTNodePtr node) override;
    int visit(BreakStatementASTNodePtr node) override;
    int visit(ContinueStatementASTNodePtr node) override;
    int visit(StatementBlockASTNodePtr node) override;
    int visit(SubprocDefinitionASTNodePtr node) override;
    int visit(FunctionDefinitionASTNodePtr node) override;
    int visit(CompilationUnitASTNodePtr node) override;

private:
    SymbolTablePtr _symbol_table;
    ScopeManager _scope_manager;
    std::queue<InvocationExpressionASTNodePtr> _pending_invocation_check;

    // [Group] scopes
    void enter_anonymous_scope();
    void enter_scope(const StringPtr &name, ScopeType scope_type);
    void leave_scope();

    // [End]

    // [Group] var use before init check
    struct SymbolScopeKey {
        std::string name;
        std::string scope;

        SymbolScopeKey(std::string name, std::string scope)
            : name(std::move(name))
            , scope(std::move(scope))
        {
        }

        bool operator==(const SymbolScopeKey &other) const
        {
            return scope == other.scope && name == other.name;
        }
    };

    struct SymbolScopeKeyHashProvider {
        std::size_t operator()(const SymbolScopeKey &obj) const
        {
            std::size_t h1 = std::hash<std::string> {}(obj.name);
            std::size_t h2 = std::hash<std::string> {}(obj.scope);
            return h1 ^ (h2 << 1);
        }
    };

    using SymbolScopedKeyValueHash = std::unordered_map<SymbolScopeKey, int, SymbolScopeKeyHashProvider>;
    std::unordered_map<SymbolScopeKey, std::stack<int>, SymbolScopeKeyHashProvider> _varinit_record_stacks;
    std::stack<SymbolScopedKeyValueHash> _varinit_record_stack_result;
    int get_varinit_record(const StringPtr &var_name);
    void create_varinit_record(const StringPtr &var_name, int is_initialized);
    void set_varinit_record(const StringPtr &var_name, int is_initialized);
    void set_varinit_record(const SymbolScopeKey &key, int is_initialized);
    void enter_scope_varinit_record();
    void leave_scope_varinit_record();
    void get_child_varinit_records(SymbolScopedKeyValueHash &result);
    // [End]

    // [Group] Visit helpers
    int visit_binary_expression(AbstractBinaryExpressionASTNodePtr node);
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
    void log_use_before_initialization_error(const StringPtr &name, const ASTNodePtr &node);
    // [End]

    bool lookup_symbol_with_ancestors(const StringPtr &name, SymbolPtr &out_symbol, std::string &out_def_scope);
    bool lookup_symbol_with_ancestors(const StringPtr &name, SymbolPtr &out_symbol);

private:
};

CLOSE_SEMANALYZER_NAMESPACE

#endif