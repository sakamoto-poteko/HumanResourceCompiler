#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fstream>
#include <iostream>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>

#include <getopt.h>

#include <spdlog/spdlog.h>

#include "ASTNode.h"
#include "ASTNodeVisitor.h"
#include "DependencyGraphAnalyzer.h"
#include "DependencyGraphBuilder.h"

int yyparse(void);
extern SyntaxNodePtr root;

struct Arguments {
    std::string input_file;
    std::string start_symbol;
    std::set<std::string> tokens;
    std::string token_file;
    std::string graphviz_output;
    bool graphviz_requested = false;
    bool check_left_recursion = false;
    bool check_non_left_circular = false;
    bool check_unreachable = false;
    bool calculate_first_set = false;
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
        { "left-recursion", no_argument, nullptr, 0 },
        { "non-left-circular", no_argument, nullptr, 0 },
        { "unreachable", no_argument, nullptr, 0 },
        { "first-set", no_argument, nullptr, 0 },
        { "help", no_argument, nullptr, 'h' },
        { "version", no_argument, nullptr, 'v' },
        { nullptr, 0, nullptr, 0 }
    };

    while ((opt = getopt_long(argc, argv, "i:s:t:f:g:hv", long_options, nullptr)) != -1) {
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
        case 0:
            if (strcmp("left-recursion", long_options[optind - 1].name) == 0) {
                args.check_left_recursion = true;
            } else if (strcmp("non-left-circular", long_options[optind - 1].name) == 0) {
                args.check_non_left_circular = true;
            } else if (strcmp("unreachable", long_options[optind - 1].name) == 0) {
                args.check_unreachable = true;
            } else if (strcmp("first-set", long_options[optind - 1].name) == 0) {
                args.calculate_first_set = true;
                args.check_left_recursion = true; // Enforce left recursion check
            }
            break;
        case 'h':
            std::cout << "Usage: program [options] [input_file]\n";
            std::cout << "Options:\n";
            std::cout << "  -i, --input        Input file\n";
            std::cout << "  -s, --start        Start symbol (default: compilation_unit)\n";
            std::cout << "  -t, --token        Specify tokens (can be used multiple times)\n";
            std::cout << "  -f, --token-file   File containing tokens, one per line\n";
            std::cout << "  -g, --graph        Output Graphviz DOT file\n";
            std::cout << "  --left-recursion   Enable left recursion check\n";
            std::cout << "  --non-left-circular Enable non-left circular dependency check\n";
            std::cout << "  --unreachable      Enable unreachable rules check\n";
            std::cout << "  --first-set        Enable FIRST set calculation (enforces left recursion check)\n";
            std::cout << "  -h, --help         Show this help message\n";
            std::cout << "  -v, --version      Show program version\n";
            std::exit(EXIT_SUCCESS);
        case 'v':
            std::cout << "Program version 1.0\n";
            std::exit(EXIT_SUCCESS);

        default:
            std::cerr << "Unknown option. Use -h or --help for help.\n";
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

    ASTPrintVisitor visitor;
    visitor.accept(root);

    DependencyGraphBuilder builder(root, args.tokens, args.start_symbol);
    boost::directed_graph<ASTNodePtr> dependency_graph;
    builder.build();
    builder.get_dependency_graph(dependency_graph);

    if (args.graphviz_requested) {
        builder.write_graphviz(args.graphviz_output);
    }

    DependencyGraphAnalyzer checker(dependency_graph, args.tokens, args.start_symbol);
    checker.analyze();

    if (args.check_left_recursion) {
        std::vector<InfoWithLoc<std::pair<ProductionNodePtr, std::vector<std::string>>>> left_recursion;
        checker.get_left_recursion(left_recursion);

        for (auto c : left_recursion) {
            std::cout << boost::format("Left recursion detected on rule '%2%' (line %4%) -> '%1%' (line %3%). Path: %5%")
                    % c.id % c.id2 % c.row % c.row2 % boost::algorithm::join(c.info.second, "->")
                      << std::endl;
        }
    }

    if (args.check_non_left_circular) {
        std::vector<InfoWithLoc<std::pair<ProductionNodePtr, ProductionNodePtr>>> nonleft_circular;
        checker.get_non_left_cicrular_dependency(nonleft_circular);

        for (auto c : nonleft_circular) {
            std::cout << boost::format("Non-left circular dependency detected between the rule '%2%' (line %4%) and '%1%' (line %3%).")
                    % c.id % c.id2 % c.row % c.row2
                      << std::endl;
        }
    }

    if (args.check_unreachable) {
        std::vector<InfoWithLoc<ProductionNodePtr>> unreachable;
        checker.get_unreachable(unreachable);

        for (auto u : unreachable) {
            std::cout << boost::format("Rule '%1%' (line %2%) is unreachable.") % u.id % u.row << std::endl;
        }
    }

    // topological order
    if (false) {
        std::vector<ProductionNodePtr> topoorder;
        checker.get_topological_rule_order(topoorder);

        std::cout << "Soft topological order of rules:" << std::endl;
        for (const auto &p : topoorder) {
            std::cout << p->id << ", ";
        }
        std::cout << std::endl;
    }

    if (args.calculate_first_set) {
        std::map<std::string, std::set<FirstSetElement>> first_set;
        checker.get_first_set(first_set);

        std::cout << "FIRST set:" << std::endl;
        for (const auto &entry : first_set) {
            std::cout << entry.first << ": ";
            for (const auto &f : entry.second) {
                switch (f.type) {
                case FirstSetElement::Literal:
                    std::cout << f.value;
                    break;
                case FirstSetElement::Epsilon:
                    std::cout << "(epsilon)";
                    break;
                case FirstSetElement::Token:
                    std::cout << f.value;
                    if (!f.produced_by.empty()) {
                        std::cout << "(" << f.produced_by << ")";
                    }
                    break;
                case FirstSetElement::Reference:
                    std::cout << "REF(" << f.value << ")";
                    break;
                }

                std::cout << " ";
            }
            std::cout << std::endl;
        }
    }

    return 0;
}
