#include <cstdio>

#include <iostream>

#include <boost/format.hpp>
#include <vector>

#include "ASTNode.h"
#include "ASTNodeVisitor.h"
#include "DependencyGraphBuilder.h"
#include "DependencyGraphAnalyzer.h"

int yyparse(void);
extern SyntaxNodePtr root;

const char *path = "../../design/hrl.ebnf";
// const char *path = "./sample.ebnf";
// const char *path = "build/test.ebnf";

int main(int argc, char **argv)
{
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

    DependencyGraphBuilder builder(root, tokens, start);
    // DependencyGraphBuilder builder(root, tokens, "S");
    boost::directed_graph<ASTNodePtr> dependency_graph;
    builder.build();
    builder.write_graphviz("build/deps.dot");
    builder.get_dependency_graph(dependency_graph);

    DependencyGraphAnalyzer checker(dependency_graph, tokens, start);
    // DependencyGraphAnalyzer checker(dependency_graph, tokens, "S");
    checker.analyze();

    std::vector<InfoWithLoc<std::pair<ProductionNodePtr, IdentifierNodePtr>>>
        circular;
    std::vector<InfoWithLoc<ProductionNodePtr>> unreachable;
    std::vector<ProductionNodePtr> topoorder;
    checker.get_cicrular_dependency(circular);
    checker.get_unreachable(unreachable);
    checker.get_topological_rule_order(topoorder);

    for (auto c : circular) {
        std::cout << boost::format(
                         "Circular dependency detected between the rule on "
                         "line %1% and '%2%' on line %3%.")
                % c.row % c.info.first->id % c.info.first->lineno()
                  << std::endl;
    }

    for (auto u : unreachable) {
        std::cout << boost::format("Rule '%1%' (line %2%) is unreachable.") % u.info->id % u.info->lineno()
                  << std::endl;
    }

    std::cout << "Soft topological order of rules:" << std::endl;
    for (const auto &p : topoorder) {
        std::cout << p->id << ", ";
    }
    std::cout << std::endl;

    return 0;
}
