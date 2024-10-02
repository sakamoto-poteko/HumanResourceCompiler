#include <iostream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "CompilerOptions.h"
#include "Versioning.h"

OPEN_HRC_NAMESPACE

namespace po = boost::program_options;

static VerbosityLevel string_to_verbosity_level(const std::string &level_str)
{
    if (level_str == "normal") {
        return VerbosityLevel::Normal;
    } else if (level_str == "info") {
        return VerbosityLevel::Info;
    } else if (level_str == "debug") {
        return VerbosityLevel::Debug;
    } else if (level_str == "trace") {
        return VerbosityLevel::Trace;
    } else {
        throw po::invalid_option_value("LEVEL must be one of normal, info, debug, trace");
    }
}

CompilerOptions parse_arguments(int argc, char **argv)
{
    CompilerOptions options;
    po::options_description desc("Allowed options");

    desc.add_options() //
        ("input,i", po::value<std::string>(), "Input file to compile") //
        ("output,o", po::value<std::string>(), "Output file name") //
        ("optimization,O", po::value<std::string>(), "Optimization level (0-2 or 's' for size)") //
        ("include,I", po::value<std::vector<std::string>>()->multitoken(), "Include paths for header files") //
        ("stage,s", po::value<std::string>()->implicit_value(""), "Run a specific compilation stage (lexer, parser, etc.)") //
        ("help,h", "Show help message") //
        ("version,V", "Show version information") //
        ("verbose,v",
            po::value<std::string>()
                ->implicit_value("debug") // Default level if no argument is provided
                ->value_name("LEVEL")
                ->notifier([&options](const std::string &level_str) {
                    options.verbosity = string_to_verbosity_level(level_str);
                }),
            "Enable verbose output with optional LEVEL (normal/info/debug/trace)");

    po::positional_options_description pos_desc;
    pos_desc.add("input", 1); // Allow input file as a positional argument

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv)
                      .options(desc)
                      .positional(pos_desc)
                      .run(),
            vm);

        if (vm.count("help")) {
            std::cout << "Usage: " << argv[0] << " [options] [input_file]\n";
            std::cout << desc
                      << "\nVerbose Levels:\n"
                      << "  normal   - Standard output\n"
                      << "  info     - Informational messages\n"
                      << "  debug    - Debugging messages (default if -v is used without LEVEL)\n"
                      << "  trace    - Detailed trace messages\n"
                      << std::endl;
            exit(EXIT_SUCCESS);
        }

        if (vm.count("version")) {
            std::cout << "hrc compiler " << git_tag() << std::endl;
            std::cout << "built with " << compiler_version() << " (" << build_type() << ") on " << build_timestamp() << std::endl;
            std::cout << "    Report bugs at https://github.com/sakamoto-poteko/HumanResourceCompiler" << std::endl;
            exit(EXIT_SUCCESS);
        }

        po::notify(vm);

        // Set default verbosity if not provided
        if (!vm.count("verbose")) {
            options.verbosity = VerbosityLevel::Normal; // Default verbosity level
        }

    } catch (const po::error &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        std::cerr << "Usage: " << argv[0] << " [options] [input_file]\n";
        std::cerr << desc
                  << "\nVerbose Levels:\n"
                  << "  normal   - Standard output\n"
                  << "  info     - Informational messages\n"
                  << "  debug    - Debugging messages (default if -v is used without LEVEL)\n"
                  << "  trace    - Detailed trace messages\n"
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    // Handle input file
    if (vm.count("input")) {
        options.input_file = vm["input"].as<std::string>();
    } else if (argc > 1) {
        options.input_file = argv[argc - 1]; // Positional argument for input file
    }

    // Handle output file
    if (vm.count("output")) {
        options.output_file = vm["output"].as<std::string>();
    }

    // Handle optimization level
    if (vm.count("optimization")) {
        std::string opt_level = vm["optimization"].as<std::string>();
        if (opt_level == "0") {
            options.optimization_level = 0;
        } else if (opt_level == "1") {
            options.optimization_level = 1;
        } else if (opt_level == "2") {
            options.optimization_level = 2;
        } else if (opt_level == "s") {
            options.optimization_level = -1; // Assuming -1 represents size optimization
        } else {
            std::cerr << "Invalid optimization level: " << opt_level << "\n";
            exit(EXIT_FAILURE);
        }
    }

    // Handle include paths
    if (vm.count("include")) {
        options.include_paths = vm["include"].as<std::vector<std::string>>();
    }

    // Handle stage option
    if (vm.count("stage")) {
        options.stage = vm["stage"].as<std::string>();
    }

    // Validate input file
    if (options.input_file.empty()) {
        std::cerr << "No input file specified.\n";
        exit(EXIT_FAILURE);
    }

    return options;
}

CLOSE_HRC_NAMESPACE