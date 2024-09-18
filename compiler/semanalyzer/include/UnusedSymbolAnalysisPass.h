#ifndef UNUSEDSYMBOLANALYSISPASS_H
#define UNUSEDSYMBOLANALYSISPASS_H

#include "ASTNodeForward.h"
#include "SemanticAnalysisPass.h"
#include "Symbol.h"
#include "WithSymbolTable.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

class UnusedSymbolAnalysisPass : public hrl::semanalyzer::SemanticAnalysisPass, public WithSymbolTable {
public:
    UnusedSymbolAnalysisPass(StringPtr filename, parser::CompilationUnitASTNodePtr root)
        : SemanticAnalysisPass(std::move(filename), std::move(root))
    {
    }

    ~UnusedSymbolAnalysisPass() = default;

    int run() override;
    int visit(const parser::VariableAccessASTNodePtr &node) override;
    int visit(const parser::IncrementExpressionASTNodePtr &node) override;
    int visit(const parser::DecrementExpressionASTNodePtr &node) override;
    int visit(const parser::VariableDeclarationASTNodePtr &node) override;
    int visit(const parser::VariableAssignmentASTNodePtr &node) override;
    // Don't strip out unused function calls. It can be exported
    // int visit(const parser::SubprocDefinitionASTNodePtr &node) override;
    // int visit(const parser::FunctionDefinitionASTNodePtr &node) override;
    // int visit(const parser::InvocationExpressionASTNodePtr &node) override;
    int visit(const parser::CompilationUnitASTNodePtr &node) override;

protected:
    void enter_node(const parser::ASTNodePtr &node) override;

private:
    enum class VisitState {
        CollectionPass,
        MutationPass,
    };
    VisitState _visit_state = VisitState::CollectionPass;

    // Pass 1: collect all symbols
    // Pass 2: remove the node
    std::set<SymbolPtr> _unused_symbols;

    int log_unused_variable(const parser::VariableDeclarationASTNodePtr &node);
    int visit_using_variable(const parser::ASTNodePtr &node);
};

CLOSE_SEMANALYZER_NAMESPACE

#endif