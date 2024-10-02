#ifndef COMPILEROPTIONS_H
#define COMPILEROPTIONS_H

#include <optional>
#include <string>
#include <vector>

#include "hrc_global.h"

OPEN_HRC_NAMESPACE

enum class VerbosityLevel {
    Normal = 0, // Default level
    Info,
    Debug,
    Trace
};

struct CompilerOptions {
    std::string input_file;
    std::optional<std::string> output_file;
    int optimization_level = 0; // Default is -O0
    std::vector<std::string> include_paths;
    VerbosityLevel verbosity = VerbosityLevel::Normal;
    bool show_version = false;
    bool show_help = false;
    std::string stage;
};

CompilerOptions parse_arguments(int argc, char **argv);

CLOSE_HRC_NAMESPACE

#endif