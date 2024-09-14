#ifndef DEADCODEELIMINATIONPASS_H
#define DEADCODEELIMINATIONPASS_H

#include "ASTNodeForward.h"
#include "ASTNodeVisitor.h"
#include "SemanticAnalysisPass.h"
#include "semanalyzer_global.h"
#include <set>

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
    int visit(const parser::StatementBlockASTNodePtr &node) override;

private:
    std::set<parser::ASTNodePtr> _while_true_statements;

    enum DeadCodeReason {
        EndOfFlow,
        ConstantFalse,
        ConstantTrue,
        AfterInfiniteLoop,
    };

    static const char *dead_code_reason_to_str(DeadCodeReason reason);

    void report_dead_code(const parser::ASTNodePtr &begin_node, const parser::ASTNodePtr &end_node, const parser::ASTNodePtr &reason_node, DeadCodeReason reason);
    void report_dead_code(const parser::ASTNodePtr &single_node, const parser::ASTNodePtr &reason_node, DeadCodeReason reason);
};

CLOSE_SEMANALYZER_NAMESPACE

#endif