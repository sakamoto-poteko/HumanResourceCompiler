#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fstream>
#include <iostream>
#include <string_view>
#include <unistd.h>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>

#include <getopt.h>

#include <spdlog/spdlog.h>

#include "ASTNode.h"
#include "ASTNodeVisitor.h"
#include "DependencyGraphAnalyzer.h"
#include "DependencyGraphBuilder.h"
#include "Tasks.h"

int yyparse(void);
extern SyntaxNodePtr root;

struct Arguments {
    std::string input_file;
    std::string start_symbol;
    std::set<std::string> tokens;
    std::string token_file;
    std::string graphviz_output;
    std::string first_follow_output;
    bool graphviz_requested = false;
    bool check_left_recursion = false;
    bool check_non_left_circular = false;
    bool check_unreachable = false;
    bool calculate_first_set = false;
    bool reprint_ebnf = false;
    bool check_conflicts = false;
};

void read_tokens_from_file(const std::string &file_path, std::set<std::string> &tokens)
{
    std::ifstream file(file_path);

    if (!file.is_open()) {
        spdlog::error("Failed to open token file: {}", file_path);
        std::exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(file, line)) {
        // Remove leading and trailing whitespace from the line
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

        if (!line.empty()) {
            tokens.insert(line);
        }
    }

    if (tokens.empty()) {
        spdlog::warn("No tokens found in the file: {}", file_path);
    }

    file.close();
}

void print_help()
{
    constexpr std::string_view help_message = R"(
Usage: program_name [OPTIONS] -i <input_file> -s <start_symbol>

Options:
  -i, --input <file>          Specify the input file containing the grammar.
  -s, --start <symbol>        Specify the start symbol for the grammar.
  -t, --token <token>         Specify a token. This option can be used multiple times to add tokens.
  -f, --token-file <file>     Specify a file containing tokens, one per line.
  -g, --graph <file>          Generate a Graphviz dependency graph and save it to the specified file.
  -L, --left-recursion        Check for left recursion in the grammar.
  -C, --non-left-circular     Check for non-left-circular productions in the grammar.
  -U, --unreachable           Check for unreachable symbols in the grammar.
  -F, --first-follow-set [file] Calculate the first and follow sets, also enforcing a left recursion check.
                              Optionally, write the C++ FIRST and FOLLOW maps. (FOLLOW is not yet supported)
  -N, --conflicts             Check for conflicts in the grammar.
  -R, --reprint-ebnf          Reprint the grammar in Extended Backus-Naur Form (EBNF).
  -h, --help                  Display this help message and exit.
  -v, --version               Display the program version and exit.

Examples:
  program_name -i grammar.ebnf -s start_symbol
  program_name -i grammar.ebnf -s start_symbol -t token1 -t token2 -g output.dot

Notes:
  - The input file (-i) and start symbol (-s) are mandatory.
  - If a token file is specified with -f, tokens from that file will be added to the list of tokens.
  - The -F option automatically enables the left recursion check.
)";

    std::cout << help_message;
}

Arguments parse_arguments(int argc, char **argv)
{
    Arguments args;
    int opt;

    struct option long_options[] = {
        { "input", required_argument, nullptr, 'i' },
        { "start", required_argument, nullptr, 's' },
        { "token", required_argument, nullptr, 't' },
        { "token-file", required_argument, nullptr, 'f' },
        { "graph", required_argument, nullptr, 'g' },
        { "left-recursion", no_argument, nullptr, 'L' },
        { "non-left-circular", no_argument, nullptr, 'C' },
        { "unreachable", no_argument, nullptr, 'U' },
        { "first-follow-set", optional_argument, nullptr, 'F' },
        { "conflicts", no_argument, nullptr, 'N' },
        { "reprint-ebnf", no_argument, nullptr, 'R' },
        { "help", no_argument, nullptr, 'h' },
        { "version", no_argument, nullptr, 'v' },
        { nullptr, 0, nullptr, 0 }
    };

    while ((opt = getopt_long(argc, argv, "i:s:t:f:g:LCUF::NRhv", long_options, nullptr)) != -1) {
        switch (opt) {
        case 'i':
            args.input_file = optarg;
            break;
        case 's':
            args.start_symbol = optarg;
            break;
        case 't':
            args.tokens.insert(optarg);
            break;
        case 'f':
            args.token_file = optarg;
            break;
        case 'g':
            args.graphviz_output = optarg;
            args.graphviz_requested = true;
            break;
        case 'L': // left-recursion
            args.check_left_recursion = true;
            break;
        case 'C': // non-left-circular
            args.check_non_left_circular = true;
            break;
        case 'U': // unreachable
            args.check_unreachable = true;
            break;
        case 'F': // first-set
            args.calculate_first_set = true;
            args.check_left_recursion = true; // Enforce left recursion check
            if (optarg) {
                args.first_follow_output = optarg;
            }
            break;
        case 'R': // reprint EBNF
            args.reprint_ebnf = true;
            break;
        case 'N':
            args.check_conflicts = true;
            break;
        case 'h':
            print_help();
            std::exit(EXIT_SUCCESS);
        case 'v':
            std::cout << "Program version 1.0\n";
            std::exit(EXIT_SUCCESS);
        default:
            std::cerr << "Unknown option or missing argument. Use -h or --help for help.\n";
            std::exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        args.input_file = argv[optind];
    }

    if (args.input_file.empty()) {
        spdlog::error("Input file is required. Use -h or --help for help.");
        std::exit(EXIT_FAILURE);
    }

    if (args.start_symbol.empty()) {
        spdlog::error("Start symbol is required. Use -h or --help for help.");
        std::exit(EXIT_FAILURE);
    }

    if (!args.token_file.empty()) {
        read_tokens_from_file(args.token_file, args.tokens);
    }

    return args;
}

int main(int argc, char **argv)
{
    spdlog::set_pattern("[%l] %v");

    Arguments args = parse_arguments(argc, argv);

    FILE *file = std::fopen(args.input_file.c_str(), "r");
    if (!file) {
        spdlog::error("Error opening file: {0}", std::strerror(errno));
        std::exit(EXIT_FAILURE);
    }

    // Redirect yyin to read from the file instead of stdin
    extern FILE *yyin;
    yyin = file;

    if (yyparse() != 0) {
        spdlog::error("Parsing failed");
        std::exit(EXIT_FAILURE);
    }
    std::fclose(yyin);

    DependencyGraphBuilder builder(root, args.tokens, args.start_symbol);
    boost::directed_graph<ASTNodePtr> dependency_graph;
    builder.build();
    builder.get_dependency_graph(dependency_graph);

    if (args.reprint_ebnf) {
        ASTPrintVisitor visitor;
        visitor.accept(root);
        std::cout << std::endl;
    }

    if (args.graphviz_requested) {
        builder.write_graphviz(args.graphviz_output);
    }

    DependencyGraphAnalyzer checker(dependency_graph, args.tokens, args.start_symbol);
    checker.analyze();

    if (args.check_left_recursion) {
        check_left_recursion(checker);
    }

    if (args.check_non_left_circular) {
        check_non_left_circular(checker);
    }

    if (args.check_unreachable) {
        check_unreachable(checker);
    }

    // topological order
    /*
    if (false) {
        std::vector<ProductionNodePtr> topoorder;
        checker.get_topological_rule_order(topoorder);

        std::cout << "Soft topological order of rules:" << std::endl;
        for (const auto &p : topoorder) {
            std::cout << p->id << ", ";
        }
        std::cout << std::endl;
    }
    */

    if (args.calculate_first_set) {
        calculate_first_follow_set(checker, args.check_conflicts, args.first_follow_output);
    }

    return 0;
}
