#ifndef CONTROLFLOWVERIFICATIONPASS_H
#define CONTROLFLOWVERIFICATIONPASS_H

#include "SemanticAnalysisPass.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

/**
 * @brief
 - [ ] Validate that all control paths in non-void functions return a value.
 - [ ] Verify correct usage of `break`, `continue`, and `return` statements.
 - [ ] Ensure that all code paths in a function lead to a valid return statement.
 - [ ] Check that all functions with a return statement actually return a value
 *
 */
class ControlFlowVerificationPass : public SemanticAnalysisPass {
public:
    ControlFlowVerificationPass(StringPtr filename, parser::CompilationUnitASTNodePtr root)
        : SemanticAnalysisPass(std::move(filename), std::move(root))
    {
    }

    ~ControlFlowVerificationPass() = default;

private:
};

CLOSE_SEMANALYZER_NAMESPACE

#endif