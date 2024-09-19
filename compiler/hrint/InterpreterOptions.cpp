#include <cstdlib>
#include <fstream>
#include <iostream>

#include <boost/program_options.hpp>

#include "InterpreterOptions.h"
#include "Versioning.h"

namespace po = boost::program_options;

static std::vector<int> parse_data(const std::string &data_str)
{
    std::vector<int> data;
    std::stringstream ss(data_str);
    std::string item;
    try {
        while (std::getline(ss, item, ',')) {
            data.push_back(std::stoi(item));
        }
    } catch (const std::exception &e) {
        throw std::runtime_error("Invalid input data format: " + data_str);
    }
    return data;
}

static std::vector<int> load_data_from_file(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string line;
    std::getline(file, line);
    return parse_data(line);
}

InterpreterOptions parse_arguments(int argc, char **argv)
{
    InterpreterOptions options;

    po::options_description desc("Allowed options");
    desc.add_options() //
        ("help,h", "Display help message") //
        ("version,V", "Show version information") //
        ("input,i", po::value<std::string>(&options.input_file)->required(), "Input source file") //
        ("input-data,I", po::value<std::string>(), "Input data as comma-separated values or a file") //
        ("no-opt", po::bool_switch()->default_value(false), "Disable optimizations") //
        ("verbose,v", po::bool_switch(&options.verbose), "Enable verbose output");

    po::positional_options_description pos_desc;
    pos_desc.add("input", 1);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv)
                      .options(desc)
                      .positional(pos_desc)
                      .run(),
            vm);

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
        
        po::notify(vm);
    } catch (const po::error &e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cerr << "Usage: " << argv[0] << " [options] [input_file]\n";
        std::cerr << desc << std::endl;
        exit(EXIT_FAILURE);
    }

    options.enable_opt = !vm["no-opt"].as<bool>();

    // Handle input-data option
    if (vm.count("input-data")) {
        std::string input_data = vm["input-data"].as<std::string>();
        bool success = false;

        // Try reading input data from a file first
        if (std::ifstream(input_data)) {
            try {
                options.input_data = load_data_from_file(input_data);
                success = true;
            } catch (const std::exception &e) {
                std::cerr << "Failed to load input data from file: " << e.what() << "\n";
            }
        }

        // If loading from file failed, try parsing as comma-separated values
        if (!success) {
            try {
                options.input_data = parse_data(input_data);
            } catch (const std::exception &e) {
                std::cerr << "Failed to parse input data: " << e.what() << "\n";
                exit(EXIT_FAILURE);
            }
        }
    }

    return options;
}
