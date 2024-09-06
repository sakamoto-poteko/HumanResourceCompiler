#ifndef SYMBOLTABLEBUILDER_H
#define SYMBOLTABLEBUILDER_H

#include "ASTNode.h"
#include "ASTNodeVisitor.h"
#include "ScopeManager.h"
#include "SymbolTable.h"
#include "hrl_global.h"
#include "semanalyzer_global.h"
#include <string>

OPEN_SEMANALYZER_NAMESPACE

using namespace parser;

/**
 * @brief This builder traverses the AST to construct the symbol table, annotating the scope id of each node.
 * It also verifies function calls and variable usage against the symbol table
 * to ensure correct usage of predefined symbols.
 */
class SymbolTableBuilder : ASTNodeVisitor {
public:
    /**
     * @brief Construct a new Symbol Table Builder object
     *
     * @param root The root node of AST
     * @param symbol_table The existing table. This can be useful when the program has imports.
     */
    SymbolTableBuilder(CompilationUnitASTNodePtr root, SymbolTablePtr symbol_table = nullptr)
        : _root(std::move(root))
        , _symbol_table(std::move(symbol_table))
    {
    }

    ~SymbolTableBuilder() override = default;

    bool build(SymbolTablePtr &symbol_table);

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

protected:
    CompilationUnitASTNodePtr _root;
    SymbolTablePtr _symbol_table;
    ScopeManager _scope_manager;

    int visit_subroutine(AbstractSubroutineASTNodePtr node);
    int lookup_and_attach_symbol_to_node_or_report_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node);

    void log_redefinition_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node);
    void log_undefined_error(const StringPtr &name, SymbolType type, const ASTNodePtr &node);

    template <typename Container>
        requires std::ranges::range<Container> && parser::convertible_to_ASTNodePtr<std::ranges::range_value_t<Container>>
    int traverse(const Container &nodes)
    {
        int result = 0;
        for (const auto &node : nodes) {
            int rc = node->accept(this);
            if (rc != 0) {
                result = rc;
            }
        }
        return result;
    }

    template <typename T>
        requires convertible_to_ASTNodePtr<T>
    int traverse(const T &node)
    {
        if (node) {
            return node->accept(this);
        } else {
            return 0;
        }
    }

    bool lookup_symbol(const StringPtr &name, SymbolPtr &out_symbol);

    // std::string current_scope() const { return _scope_manager.get_current_scope_id(); }

private:
};

CLOSE_SEMANALYZER_NAMESPACE

#endif