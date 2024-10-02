#ifndef INTERPRETEROPTIONS_H
#define INTERPRETEROPTIONS_H

#include <string>
#include <vector>

#include "HRMByte.h"

enum class CompileTarget {
    AST = 0,
    HIR = 1,
    HIR_SSA = 2,
    LIR_SSA = 3,
};

enum class VerbosityLevel {
    Normal = 0, // Default level
    Info,
    Debug,
    Trace
};

struct InterpreterOptions {
    std::string input_file;
    std::vector<hrl::interpreter::HRMByte> input_data;
    bool enable_opt = true;
    VerbosityLevel verbosity = VerbosityLevel::Normal;
    CompileTarget compile_target;
};

InterpreterOptions parse_arguments(int argc, char **argv);

#endif