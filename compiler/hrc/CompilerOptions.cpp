#include <iostream>

#include "getopt.h"

#include "CompilerOptions.h"
#include "git.h"

OPEN_HRC_NAMESPACE

CompilerOptions __compiler_options;

void parse_arguments(int argc, char **argv)
{
    static struct option long_options[] = {
        { "input", required_argument, 0, 'i' },
        { "output", required_argument, 0, 'o' },
        { "optimization", required_argument, 0, 'O' },
        { "include", required_argument, 0, 'I' },
        { "verbose", no_argument, 0, 'v' },
        { "version", no_argument, 0, 0 },
        { "help", no_argument, 0, 0 },
        { "stage", optional_argument, 0, 's' },
        { 0, 0, 0, 0 }
    };

    int option_index = 0;
    int opt;

    while ((opt = getopt_long(argc, argv, "i:o:O:I:vs::", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'i':
            __compiler_options.input_file = optarg;
            break;
        case 'o':
            __compiler_options.output_file = optarg;
            break;
        case 'O':
            if (std::string(optarg) == "0") {
                __compiler_options.optimization_level = 0;
            } else if (std::string(optarg) == "1") {
                __compiler_options.optimization_level = 1;
            } else if (std::string(optarg) == "2") {
                __compiler_options.optimization_level = 2;
            } else if (std::string(optarg) == "s") {
                __compiler_options.optimization_level = -1;
            }
            break;
        case 'I':
            __compiler_options.include_paths.push_back(optarg);
            break;
        case 'v':
            __compiler_options.verbose = true;
            break;
        case 's':
            __compiler_options.stage = optarg ? optarg : "";
            break;
        case 0:
            if (std::string(long_options[option_index].name) == "version") {
                __compiler_options.show_version = true;
            } else if (std::string(long_options[option_index].name) == "help") {
                __compiler_options.show_help = true;
            }
            break;
        default:
            std::cerr << "Usage: " << argv[0] << " [options]\n";
            exit(EXIT_FAILURE);
        }
    }

    // If an input file wasn't specified with -i, check for a positional argument
    if (optind < argc && __compiler_options.input_file.empty()) {
        __compiler_options.input_file = argv[optind];
    }

    // Process __compiler_options
    if (__compiler_options.show_help) {
        std::cout << "Usage: " << argv[0] << " [options] [input_file]\n";
        std::cout << "  -i, --input FILE          Input file to compile\n";
        std::cout << "  -o, --output FILE         Output file name\n";
        std::cout << "  -O, --optimization LEVEL  Optimization level (0-2 or 's' for size)\n";
        std::cout << "  -I, --include PATH        Include paths for header files (can be used multiple times)\n";
        std::cout << "  -v, --verbose             Enable verbose output\n";
        std::cout << "  -s, --stage [STAGE]       Run a specific compilation stage (lexer, parser, etc.)\n";
        std::cout << "      --version             Show version information\n";
        std::cout << "      --help                Show this help message\n";
        std::cout << "  input_file                Positional argument for the input file (alternative to -i)\n";

        exit(EXIT_SUCCESS);
    }

    if (__compiler_options.show_version) {
        std::cout << "hrc compiler " << git_Describe() << std::endl;
        std::cout << "    Report bugs in https://github.com/sakamoto-poteko/HumanResourceCompiler" << std::endl;
        exit(EXIT_SUCCESS);
    }

    if (__compiler_options.input_file.empty()) {
        std::cerr << "No input file specified.\n";
        exit(EXIT_FAILURE);
    }
}

CLOSE_HRC_NAMESPACE