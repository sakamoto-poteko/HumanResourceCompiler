#ifndef INTERPRETEROPTIONS_H
#define INTERPRETEROPTIONS_H

#include <string>
#include <vector>

struct InterpreterOptions {
    std::string input_file;
    std::vector<int> input_data;
    bool enable_opt = true;
    bool verbose = false;
};

InterpreterOptions parse_arguments(int argc, char **argv);

#endif