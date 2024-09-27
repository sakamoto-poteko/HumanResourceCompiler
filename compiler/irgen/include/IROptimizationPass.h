#ifndef IROPTIMIZATIONPASS_H
#define IROPTIMIZATIONPASS_H

#include <memory>

#include "IRProgramStructure.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class IROptimizationPass {
public:
    IROptimizationPass(const ProgramPtr &program)
        : _program(program)
    {
    }

    virtual ~IROptimizationPass() = default;

    virtual int run();

protected:
    ProgramPtr _program;

    virtual int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) = 0;

private:
};

using IROptimizationPassPtr = std::shared_ptr<IROptimizationPass>;

CLOSE_IRGEN_NAMESPACE

#endif