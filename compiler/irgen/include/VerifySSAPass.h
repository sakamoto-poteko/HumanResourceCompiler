#ifndef VERIFYSSAPASS_H
#define VERIFYSSAPASS_H

#include "IROptimizationPass.h"
#include "IRProgramStructure.h"
#include "ThreeAddressCode.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class VerifySSAPass : public IROptimizationPass {
public:
    VerifySSAPass(const ProgramPtr &program, const IRGenOptions &options)
        : IROptimizationPass(program, options)
    {
    }

    ~VerifySSAPass() = default;

protected:
    int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) override;

    template <typename VerifyFunc>
    void visit_basic_block(BBGraphVertex vertex, const BBGraph &cfg, std::set<BBGraphVertex> &visited, VerifyFunc verify_func)
    {
        if (visited.contains(vertex)) {
            return;
        }
        visited.insert(vertex);

        const BasicBlockPtr &basic_block = cfg[vertex];

        verify_func(basic_block);

        for (BBGraphEdge edge : boost::make_iterator_range(boost::out_edges(vertex, cfg))) {
            BBGraphVertex target = boost::target(edge, cfg);
            visit_basic_block(target, cfg, visited, verify_func);
        }
    }

    static void verify_basic_block_assignments_and_uses(const BasicBlockPtr &basic_block, std::map<unsigned int, BasicBlockPtr> &_variable_assigned);
    static void verify_basic_block_phi_incoming_branches(const BasicBlockPtr &basic_block, const std::map<unsigned int, BasicBlockPtr> &_variable_assigned);

private:
};

CLOSE_IRGEN_NAMESPACE

#endif