#ifndef USEBEFOREINITIALIZATIONPASS_H
#define USEBEFOREINITIALIZATIONPASS_H

#include "ASTNodeForward.h"
#include "SemanticAnalysisPass.h"
#include "SymbolTable.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

class UseBeforeInitializationPass : public SemanticAnalysisPass {
public:
    UseBeforeInitializationPass(StringPtr filename, parser::CompilationUnitASTNodePtr root)
        : SemanticAnalysisPass(std::move(filename), std::move(root))
    {
    }

    ~UseBeforeInitializationPass() = default;

    // For all visit, the return value of 0 indicate success.
    int visit(parser::IntegerASTNodePtr node) override;
    int visit(parser::BooleanASTNodePtr node) override;
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
    int visit(parser::AddExpressionASTNodePtr node) override;
    int visit(parser::SubExpressionASTNodePtr node) override;
    int visit(parser::MulExpressionASTNodePtr node) override;
    int visit(parser::DivExpressionASTNodePtr node) override;
    int visit(parser::ModExpressionASTNodePtr node) override;
    int visit(parser::EqualExpressionASTNodePtr node) override;
    int visit(parser::NotEqualExpressionASTNodePtr node) override;
    int visit(parser::GreaterThanExpressionASTNodePtr node) override;
    int visit(parser::GreaterEqualExpressionASTNodePtr node) override;
    int visit(parser::LessThanExpressionASTNodePtr node) override;
    int visit(parser::LessEqualExpressionASTNodePtr node) override;
    int visit(parser::AndExpressionASTNodePtr node) override;
    int visit(parser::OrExpressionASTNodePtr node) override;
    int visit(parser::InvocationExpressionASTNodePtr node) override;
    int visit(parser::EmptyStatementASTNodePtr node) override;
    int visit(parser::IfStatementASTNodePtr node) override;
    int visit(parser::WhileStatementASTNodePtr node) override;
    int visit(parser::ForStatementASTNodePtr node) override;
    int visit(parser::ReturnStatementASTNodePtr node) override;
    int visit(parser::BreakStatementASTNodePtr node) override;
    int visit(parser::ContinueStatementASTNodePtr node) override;
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

private:
    SymbolTablePtr _symbol_table;

    // [Group] var use before init check
    struct SymbolScopeKey {
        StringPtr name;
        std::string scope;

        SymbolScopeKey(StringPtr name, std::string scope)
            : name(std::move(name))
            , scope(std::move(scope))
        {
        }

        bool operator==(const SymbolScopeKey &other) const
        {
            return scope == other.scope && *name == *other.name;
        }
    };

    struct SymbolScopeKeyHashProvider {
        std::size_t operator()(const SymbolScopeKey &obj) const
        {
            std::size_t h1 = std::hash<std::string> {}(*obj.name);
            std::size_t h2 = std::hash<std::string> {}(obj.scope);
            return h1 ^ (h2 << 1);
        }
    };

    using SymbolScopedKeyValueHash = std::unordered_map<SymbolScopeKey, int, SymbolScopeKeyHashProvider>;
    std::unordered_map<SymbolScopeKey, std::stack<int>, SymbolScopeKeyHashProvider> _varinit_record_stacks;
    std::stack<SymbolScopedKeyValueHash> _varinit_record_stack_result;
    int get_varinit_record(const StringPtr &var_name, const parser::ASTNodePtr &node_scope_level);
    void create_varinit_record(const StringPtr &var_name, int is_initialized, const parser::ASTNodePtr &node_scope_level);
    void set_varinit_record(const StringPtr &var_name, int is_initialized, const parser::ASTNodePtr &node_scope_level);
    void set_varinit_record(const SymbolScopeKey &key, int is_initialized, const parser::ASTNodePtr &node_scope_level);
    void enter_scope_varinit_record(const parser::ASTNodePtr &node_scope_level);
    void leave_scope_varinit_record(const parser::ASTNodePtr &node_scope_level);
    void get_child_varinit_records(SymbolScopedKeyValueHash &result, const parser::ASTNodePtr &node_scope_level);
    void set_child_varinit_records(const SymbolScopedKeyValueHash &records, const parser::ASTNodePtr &node_scope_level);

    void log_use_before_initialization_error(const StringPtr &name, const parser::ASTNodePtr &node);
    // [End]

    bool lookup_symbol_with_ancestors(const parser::ASTNodePtr &node, const StringPtr &name, SymbolPtr &out_symbol, std::string &out_def_scope);
    bool lookup_symbol_with_ancestors(const parser::ASTNodePtr &node, const StringPtr &name, SymbolPtr &out_symbol);
};

CLOSE_SEMANALYZER_NAMESPACE

#endif