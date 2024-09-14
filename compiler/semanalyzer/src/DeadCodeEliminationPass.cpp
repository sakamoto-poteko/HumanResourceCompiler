#include "DeadCodeEliminationPass.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

#define BEGIN_VISIT() \
    enter_node(node); \
    int rc = 0;       \
    UNUSED(rc);

#define END_VISIT() \
    leave_node();   \
    return 0;

#define RETURN_IF_FAIL_IN_VISIT(rc) \
    if (rc != 0) {                  \
        leave_node();               \
        return rc;                  \
    }

int DeadCodeEliminationPass::run()
{
    return 0;
}

int DeadCodeEliminationPass::visit(const parser::EmptyStatementASTNodePtr &node)
{
    BEGIN_VISIT();
    END_VISIT();
}

int DeadCodeEliminationPass::visit(const parser::IfStatementASTNodePtr &node)
{
    BEGIN_VISIT();
    END_VISIT();
}

int DeadCodeEliminationPass::visit(const parser::WhileStatementASTNodePtr &node)
{
    BEGIN_VISIT();
    END_VISIT();
}

int DeadCodeEliminationPass::visit(const parser::ForStatementASTNodePtr &node)
{
    BEGIN_VISIT();
    END_VISIT();
}

int DeadCodeEliminationPass::visit(const parser::ReturnStatementASTNodePtr &node)
{
    BEGIN_VISIT();
    END_VISIT();
}

int DeadCodeEliminationPass::visit(const parser::BreakStatementASTNodePtr &node)
{
    BEGIN_VISIT();
    END_VISIT();
}

int DeadCodeEliminationPass::visit(const parser::ContinueStatementASTNodePtr &node)
{
    BEGIN_VISIT();
    END_VISIT();
}

int DeadCodeEliminationPass::visit(const parser::StatementBlockASTNodePtr &node)
{
    BEGIN_VISIT();
    END_VISIT();
}

CLOSE_SEMANALYZER_NAMESPACE
// end
