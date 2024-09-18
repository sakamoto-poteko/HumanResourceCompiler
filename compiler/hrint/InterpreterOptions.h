#ifndef INTERPRETEROPTIONS_H
#define INTERPRETEROPTIONS_H

#include <string>

struct InterpreterOptions {
    std::string input_file;
    bool enable_opt = true;
    bool verbose = false;
};

InterpreterOptions parse_arguments(int argc, char **argv);

#endif