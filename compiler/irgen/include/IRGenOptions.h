#ifndef IRGENOPTIONS_H
#define IRGENOPTIONS_H

#include "irgen_global.h"

OPEN_IRGEN_NAMESPACE

enum class IROptimizationFor {
    NoOpt,
    OptForSpeed,
    OptForCodeSize,
};

struct IRGenOptions {
    IROptimizationFor MultiplyLowering = IROptimizationFor::NoOpt;
    IROptimizationFor DivisionLowering = IROptimizationFor::NoOpt;
    IROptimizationFor ModulusLowering = IROptimizationFor::NoOpt;

    static IRGenOptions ForSpeed()
    {
        return IRGenOptions {
            .MultiplyLowering = IROptimizationFor::OptForSpeed,
            .DivisionLowering = IROptimizationFor::OptForSpeed,
            .ModulusLowering = IROptimizationFor::OptForSpeed,
        };
    }

    static IRGenOptions ForCodeSize()
    {
        return IRGenOptions {
            .MultiplyLowering = IROptimizationFor::OptForCodeSize,
            .DivisionLowering = IROptimizationFor::OptForCodeSize,
            .ModulusLowering = IROptimizationFor::OptForCodeSize,
        };
    }
};

CLOSE_IRGEN_NAMESPACE

#endif