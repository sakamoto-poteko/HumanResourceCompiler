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

struct InterpreterOptions {
    std::string input_file;
    std::vector<hrl::interpreter::HRMByte> input_data;
    bool enable_opt = true;
    bool verbose = false;
    CompileTarget compile_target;
};

InterpreterOptions parse_arguments(int argc, char **argv);

#endif