#include <charconv>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include "HRMByte.h"
#include "InterpreterOptions.h"
#include "Versioning.h"

namespace po = boost::program_options;

static std::vector<hrl::interpreter::HRMByte> parse_data(const std::string &data_str)
{
    std::vector<hrl::interpreter::HRMByte> data;
    std::vector<std::string> tokens;
    boost::split(tokens, data_str, boost::is_any_of(","));

    for (auto &token : tokens) {
        boost::algorithm::trim(token); // Trim whitespace
        int value;

        if (token.size() == 1 && std::isalpha(token[0])) {
            char ch = static_cast<char>(std::toupper(token[0]));
            data.push_back(hrl::interpreter::HRMByte(ch));
        } else if (std::from_chars(token.data(), token.data() + token.size(), value).ec == std::errc()) {
            data.push_back(hrl::interpreter::HRMByte(value));
        } else {
            throw std::runtime_error("Invalid input: " + token);
        }
    }

    return data;
}

static std::vector<hrl::interpreter::HRMByte> load_data_from_file(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string line;
    std::getline(file, line);
    return parse_data(line);
}

static CompileTarget string_to_compile_target(const std::string &str)
{
    if (str == "AST") {
        return CompileTarget::AST;
    } else if (str == "HIR") {
        return CompileTarget::HIR;
    } else if (str == "HIR_SSA") {
        return CompileTarget::HIR_SSA;
    } else if (str == "LIR_SSA") {
        return CompileTarget::LIR_SSA;
    } else {
        throw po::validation_error(po::validation_error::invalid_option_value, "compile-target", str);
    }
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
        ("compile-target,c", po::value<std::string>()->default_value("AST"), "Compile target (AST, HIR, HIR_SSA, LIR_SSA)") //
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
            std::cout << "    Report bugs at https://github.com/sakamoto-poteko/HumanResourceCompiler" << std::endl;
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

    if (vm.count("compile-target")) {
        std::string target_str = vm["compile-target"].as<std::string>();
        boost::to_upper(target_str);
        options.compile_target = string_to_compile_target(target_str);
    }

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
