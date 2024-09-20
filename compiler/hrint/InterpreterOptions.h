#ifndef INTERPRETEROPTIONS_H
#define INTERPRETEROPTIONS_H

#include <string>
#include <vector>

#include "HRMByte.h"

struct InterpreterOptions {
    std::string input_file;
    std::vector<hrl::interpreter::HRMByte> input_data;
    bool enable_opt = true;
    bool verbose = false;
};

InterpreterOptions parse_arguments(int argc, char **argv);

#endif