#ifndef PROPAGATECOPYPASS_H
#define PROPAGATECOPYPASS_H

#include <map>

#include "IRGenOptions.h"
#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class PropagateCopyPass : public IROptimizationPass {
public:
    PropagateCopyPass(const ProgramPtr &program, const IRGenOptions &options)
        : IROptimizationPass(program, options)
    {
    }

    ~PropagateCopyPass();

    int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) override;

private:
    void visit_dominance_node(const BBGraphVertex vertex, const BBGraph &dom_tree, std::map<unsigned int, std::tuple<unsigned int, BasicBlockPtr>> &prev_copied_variable);
};

CLOSE_IRGEN_NAMESPACE

#endif