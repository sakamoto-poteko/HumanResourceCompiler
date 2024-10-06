#ifndef REMOVE_DEAD_INSTRUCTION_PASS_H
#define REMOVE_DEAD_INSTRUCTION_PASS_H

#include "IROptimizationPass.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class RemoveDeadInstructionsPass : public IROptimizationPass {
public:
    RemoveDeadInstructionsPass(const ProgramPtr &program, const IRGenOptions &options)
        : IROptimizationPass(program, options)
    {
    }

    ~RemoveDeadInstructionsPass() = default;

protected:
    int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) override;
    void dce_dom_tree_visit(const BBGraphVertex vertex, const BBGraph &dom_tree, std::set<unsigned int> &live_variables);

private:
};

CLOSE_IRGEN_NAMESPACE
#endif