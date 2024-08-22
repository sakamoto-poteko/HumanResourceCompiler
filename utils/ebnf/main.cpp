#include <cstdio>

#include <memory>

#include <boost/format.hpp>

#include "ASTNodeVisitor.h"
#include "DependencyGraphBuilder.h"
#include "DependencyGraphSanityChecker.h"

int yyparse(void);
extern SyntaxNodePtr root;

const char *path = "../../design/hrl.ebnf";
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
    }
    else {
        printf("Parsing failed.\n");
        return EXIT_FAILURE;
    }
    fclose(yyin);

    ASTPrintVisitor visitor;
    visitor.accept(root);

    auto tokens = std::set<std::string>(
        {"ADD",     "ADDADD", "AND",        "BOOLEAN",  "DIV",    "EE",
         "ELSE",    "EQ",     "FLOOR",      "FLOORMAX", "FOR",    "FUNCTION",
         "GE",      "GT",     "IDENTIFIER", "IF",       "IMPORT", "INIT",
         "INTEGER", "LE",     "LET",        "LT",       "MOD",    "MUL",
         "NE",      "NOT",    "OR",         "RETURN",   "SUB",    "SUBSUB",
         "SUBWORD", "WHILE"});
    std::string start = "compilation_unit";

    DependencyGraphBuilder builder(root, tokens, start);
    boost::directed_graph<ASTNodePtr> dependency_graph;
    builder.build();
    builder.write_graphviz("build/o.dot");
    builder.get_dependency_graph(dependency_graph);

    DependencyGraphSanityChecker checker(dependency_graph, tokens, start);
    checker.check();

    std::vector<InfoWithLoc<std::pair<ProductionNodePtr, IdentifierNodePtr>>>
        circular;
    std::vector<InfoWithLoc<ProductionNodePtr>> unreachable;
    checker.get_cicrular_dependency(circular);
    checker.get_unreachable(unreachable);

    for (auto c : circular) {
        std::cout << boost::format(
                         "Circular dependency detected between the rule on "
                         "line %1% and '%2%' on line %3%.") %
                         c.row % c.info.first->id % c.info.first->lineno()
                  << std::endl;
    }

    for (auto u : unreachable) {
        std::cout << boost::format("Rule '%1%' (line %2%) is unreachable.") %
                         u.info->id % u.info->lineno()
                  << std::endl;
    }

    

    return 0;
}
