#ifndef DEADCODEELIMINATIONPASS_H
#define DEADCODEELIMINATIONPASS_H

#include "SemanticAnalysisPass.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

class DeadCodeEliminationPass : public SemanticAnalysisPass {
public:
    DeadCodeEliminationPass(StringPtr filename, parser::CompilationUnitASTNodePtr root)
        : SemanticAnalysisPass(std::move(filename), std::move(root))
    {
    }

    ~DeadCodeEliminationPass() override = default;

    int run() override;

    // For all visit, the return value of 0 indicate success.
    int visit(const parser::EmptyStatementASTNodePtr &node) override;
    int visit(const parser::IfStatementASTNodePtr &node) override;
    int visit(const parser::WhileStatementASTNodePtr &node) override;
    int visit(const parser::ForStatementASTNodePtr &node) override;
    int visit(const parser::ReturnStatementASTNodePtr &node) override;
    int visit(const parser::BreakStatementASTNodePtr &node) override;
    int visit(const parser::ContinueStatementASTNodePtr &node) override;
    int visit(const parser::StatementBlockASTNodePtr &node) override;

private:
};

CLOSE_SEMANALYZER_NAMESPACE

#endif