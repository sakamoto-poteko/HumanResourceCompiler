#include <iostream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "CompilerOptions.h"
#include "git.h"

OPEN_HRC_NAMESPACE

namespace po = boost::program_options;

CompilerOptions parse_arguments(int argc, char **argv)
{
    CompilerOptions options;
    po::options_description desc("Allowed options");

    desc.add_options() //
        ("input,i", po::value<std::string>(), "Input file to compile") //
        ("output,o", po::value<std::string>(), "Output file name") //
        ("optimization,O", po::value<std::string>(), "Optimization level (0-2 or 's' for size)") //
        ("include,I", po::value<std::vector<std::string>>()->multitoken(), "Include paths for header files") //
        ("verbose,v", "Enable verbose output") //
        ("version", "Show version information") //
        ("help", "Show help message") //
        ("stage,s", po::value<std::string>()->implicit_value(""), "Run a specific compilation stage (lexer, parser, etc.)");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (const po::error &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        std::cerr << desc << std::endl;
        exit(EXIT_FAILURE);
    }

    if (vm.count("help")) {
        std::cout << "Usage: " << argv[0] << " [options] [input_file]\n";
        std::cout << desc << std::endl;
        exit(EXIT_SUCCESS);
    }

    if (vm.count("version")) {
        std::cout << "hrc compiler " << git_Describe() << std::endl;
        std::cout << "    Report bugs in https://github.com/sakamoto-poteko/HumanResourceCompiler" << std::endl;
        exit(EXIT_SUCCESS);
    }

    if (vm.count("input")) {
        options.input_file = vm["input"].as<std::string>();
    } else if (argc > 1) {
        options.input_file = argv[argc - 1]; // Positional argument for input file
    }

    if (vm.count("output")) {
        options.output_file = vm["output"].as<std::string>();
    }

    if (vm.count("optimization")) {
        std::string opt_level = vm["optimization"].as<std::string>();
        if (opt_level == "0") {
            options.optimization_level = 0;
        } else if (opt_level == "1") {
            options.optimization_level = 1;
        } else if (opt_level == "2") {
            options.optimization_level = 2;
        } else if (opt_level == "s") {
            options.optimization_level = -1;
        }
    }

    if (vm.count("include")) {
        options.include_paths = vm["include"].as<std::vector<std::string>>();
    }

    if (vm.count("verbose")) {
        options.verbose = true;
    }

    if (vm.count("stage")) {
        options.stage = vm["stage"].as<std::string>();
    }

    if (options.input_file.empty()) {
        std::cerr << "No input file specified.\n";
        exit(EXIT_FAILURE);
    }

    return options;
}

CLOSE_HRC_NAMESPACE