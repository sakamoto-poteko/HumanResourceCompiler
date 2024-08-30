
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
#include "DependencyGraphAnalyzer.h"

#include "Tasks.h"

void calculate_first_follow_set(DependencyGraphAnalyzer &checker, bool check_conflicts)
{
    std::map<std::string, std::set<FirstSetElement>> first_set;
    std::map<std::string, std::set<std::string>> conflicts;
    checker.get_first_set(first_set);
    checker.get_first_first_conflicts(conflicts);

    std::cout << "FIRST set:" << std::endl;
    for (const auto &entry : first_set) {
        std::cout << entry.first << ": ";
        for (const auto &f : entry.second) {
            switch (f.type) {
            case FirstSetElement::Literal:
                std::cout << f.value;
                break;
            case FirstSetElement::Epsilon:
                std::cout << "$EPSILON";
                break;
            case FirstSetElement::Token:
                std::cout << f.value;
                if (!f.produced_by.empty()) {
                    std::cout << "(" << f.produced_by << ")";
                }
                break;
            case FirstSetElement::Reference:
                std::cout << "$REF[" << f.value << "]";
                break;
            }

            std::cout << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    if (check_conflicts) {
        std::cout << "FIRST/FIRST conflict(s):" << std::endl;
        for (const auto &first : conflicts) {
            std::cout
                << boost::format("FIRST/FIRST conflict: %1% appeared in rule '%2%'") % first.first % boost::join(first.second, "', '")
                << std::endl;
        }
    }
}

void check_unreachable(DependencyGraphAnalyzer &checker)
{
    std::vector<InfoWithLoc<ProductionNodePtr>> unreachable;
    checker.get_unreachable(unreachable);

    std::cout << "Unreachable:" << std::endl;
    for (auto u : unreachable) {
        std::cout << boost::format("Rule '%1%' (line %2%) is unreachable.") % u.id % u.row << std::endl;
    }
    std::cout << std::endl;
}

void check_non_left_circular(DependencyGraphAnalyzer &checker)
{
    std::vector<InfoWithLoc<std::pair<ProductionNodePtr, ProductionNodePtr>>> nonleft_circular;
    checker.get_non_left_cicrular_dependency(nonleft_circular);

    std::cout << "Circular dependency (excl. left recursion):" << std::endl;
    for (auto c : nonleft_circular) {
        std::cout << boost::format("Non-left circular dependency detected between the rule '%2%' (line %4%) and '%1%' (line %3%).")
                % c.id % c.id2 % c.row % c.row2
                  << std::endl;
    }
    std::cout << std::endl;
}

void check_left_recursion(DependencyGraphAnalyzer &checker)
{
    std::vector<InfoWithLoc<std::pair<ProductionNodePtr, std::vector<std::string>>>> left_recursion;
    checker.get_left_recursion(left_recursion);

    std::cout << "Left recursion:" << std::endl;
    for (auto c : left_recursion) {
        std::cout << boost::format("Left recursion detected on rule '%2%' (line %4%) -> '%1%' (line %3%). Path: %5%")
                % c.id % c.id2 % c.row % c.row2 % boost::algorithm::join(c.info.second, "->")
                  << std::endl;
    }
    std::cout << std::endl;
}
