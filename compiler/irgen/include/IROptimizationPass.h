#ifndef IROPTIMIZATIONPASS_H
#define IROPTIMIZATIONPASS_H

#include <memory>
#include <string>

#include "IRProgramStructure.h"
#include "hrl_global.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class IROptimizationPass {
public:
    virtual ~IROptimizationPass() = default;

    virtual int run();

    virtual std::string get_additional_metadata_text(unsigned int task_index, const std::string &path)
    {
        UNUSED(task_index);
        UNUSED(path);

        return std::string();
    }

protected:
    IROptimizationPass(const ProgramPtr &program)
        : _program(program)
    {
    }

    ProgramPtr _program;

    virtual int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) = 0;

private:
};

using IROptimizationPassPtr = std::shared_ptr<IROptimizationPass>;

CLOSE_IRGEN_NAMESPACE

#endif