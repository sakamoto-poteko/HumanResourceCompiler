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
};

CLOSE_IRGEN_NAMESPACE

#endif