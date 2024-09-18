#include <cstdlib>
#include <iostream>

#include <boost/program_options.hpp>

#include "InterpreterOptions.h"
#include "Versioning.h"

namespace po = boost::program_options;

InterpreterOptions parse_arguments(int argc, char **argv)
{
    InterpreterOptions options;

    po::options_description desc("Allowed options");
    desc.add_options() //
        ("help,h", "Display help message") //
        ("version,v", "Show version information") //
        ("input,i", po::value<std::string>(&options.input_file)->required(), "Input source file") //
        ("no-opt", po::bool_switch()->default_value(false), "Disable optimizations") //
        ("verbose", po::bool_switch(&options.verbose), "Enable verbose output");

    po::positional_options_description pos_desc;
    pos_desc.add("input", 1);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv)
                      .options(desc)
                      .positional(pos_desc)
                      .run(),
            vm);

        po::notify(vm);
    } catch (const po::error &e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cerr << "Usage: " << argv[0] << " [options] [input_file]\n";
        std::cerr << desc << std::endl;
        exit(EXIT_FAILURE);
    }

    if (vm.count("help")) {
        std::cout << "Usage: " << argv[0] << " [options] [input_file]\n";
        std::cout << desc << std::endl;
        exit(EXIT_SUCCESS);
    }

    if (vm.count("version")) {
        std::cout << "hrint interpreter " << git_tag() << std::endl;
        std::cout << "built with " << compiler_version() << " (" << build_type() << ") on " << build_timestamp() << std::endl;
        std::cout << "    Report bugs in https://github.com/sakamoto-poteko/HumanResourceCompiler" << std::endl;
        exit(EXIT_SUCCESS);
    }

    options.enable_opt = !vm["no-opt"].as<bool>();

    return options;
}
