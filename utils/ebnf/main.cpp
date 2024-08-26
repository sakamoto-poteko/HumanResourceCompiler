#include <cstdio>

#include <iostream>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>

#include <spdlog/spdlog.h>

#include "ASTNode.h"
#include "ASTNodeVisitor.h"
#include "DependencyGraphAnalyzer.h"
#include "DependencyGraphBuilder.h"

int yyparse(void);
extern SyntaxNodePtr root;

int main(int argc, char **argv)
{
    // spdlog::set_pattern("[%l] %v");

    const char *path = "../../design/hrl.ebnf";
    // const char *path = "./sample.ebnf";
    // const char *path = "build/test.ebnf";

    FILE *file = std::fopen(path, "r");
    if (!file) {
        std::perror(path);
        return 1;
    }
    // Redirect yyin to read from the file instead of stdin
    extern FILE *yyin;
    yyin = file;

    if (yyparse() == 0) {
        printf("Parsing completed successfully.\n");
    } else {
        printf("Parsing failed.\n");
        return EXIT_FAILURE;
    }
    fclose(yyin);

    ASTPrintVisitor visitor;
    visitor.accept(root);

    auto tokens = std::set<std::string>(
        { "ADD", "ADDADD", "AND", "BOOLEAN", "DIV", "EE",
            "ELSE", "EQ", "FLOOR", "FLOORMAX", "FOR", "FUNCTION",
            "GE", "GT", "IDENTIFIER", "IF", "IMPORT", "INIT",
            "INTEGER", "LE", "LET", "LT", "MOD", "MUL",
            "NE", "NOT", "OR", "RETURN", "SUB", "SUBSUB",
            "SUBWORD", "WHILE", "T", "OPEN_BRACKET", "CLOSE_BRACKET", "OPEN_PAREN",
            "CLOSE_PAREN", "OPEN_BRACE", "CLOSE_BRACE", "COMMA" });
    std::string start = "compilation_unit";
    // std::string start = "function_body";

    DependencyGraphBuilder builder(root, tokens, start);
    // DependencyGraphBuilder builder(root, tokens, "S");
    boost::directed_graph<ASTNodePtr> dependency_graph;
    builder.build();
    builder.write_graphviz("build/deps.dot");
    builder.get_dependency_graph(dependency_graph);

    DependencyGraphAnalyzer checker(dependency_graph, tokens, start);
    // DependencyGraphAnalyzer checker(dependency_graph, tokens, "S");
    checker.analyze();

    std::vector<InfoWithLoc<std::pair<ProductionNodePtr, ProductionNodePtr>>> nonleft_circular;
    std::vector<InfoWithLoc<std::pair<ProductionNodePtr, std::vector<std::string>>>> left_recursion;
    std::vector<InfoWithLoc<ProductionNodePtr>> unreachable;
    std::vector<ProductionNodePtr> topoorder;
    std::map<std::string, std::set<FirstSetElement>> first_set;

    checker.get_left_recursion(left_recursion);
    checker.get_non_left_cicrular_dependency(nonleft_circular);
    checker.get_unreachable(unreachable);
    checker.get_topological_rule_order(topoorder);
    checker.get_first_set(first_set);

    for (auto c : nonleft_circular) {
        std::cout << boost::format("Non-left circular dependency detected between the rule '%2%' (line %4%) and '%1%' (line %3%)")
                % c.id % c.id2 % c.row % c.row2
                  << std::endl;
    }

    for (auto c : left_recursion) {
        std::cout << boost::format("Left recursion detected on rule '%2%' (line %4%) -> '%1%' (line %3%). Path: %5%")
                % c.id % c.id2 % c.row % c.row2 % boost::algorithm::join(c.info.second, "->")
                  << std::endl;
    }

    for (auto u : unreachable) {
        std::cout << boost::format("Rule '%1%' (line %2%) is unreachable.") % u.id % u.row << std::endl;
    }

    std::cout << "Soft topological order of rules:" << std::endl;
    for (const auto &p : topoorder) {
        std::cout << p->id << ", ";
    }
    std::cout << std::endl;

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

    return 0;
}
