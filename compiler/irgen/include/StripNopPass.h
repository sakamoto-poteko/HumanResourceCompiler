#ifndef STRIPNOPPASS_H
#define STRIPNOPPASS_H

#include "IROptimizationPass.h"
#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

class StripNopPass : public IROptimizationPass {
public:
    StripNopPass(const ProgramPtr &program) : IROptimizationPass(program) {}
    ~StripNopPass();

protected:
    int run_subroutine(const SubroutinePtr &subroutine, ProgramMetadata &metadata, const ProgramPtr &program) override;

private:
};

CLOSE_IRGEN_NAMESPACE
#endif