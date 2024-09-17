#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>

#include <spdlog/spdlog.h>

#include "ASTNode.h"
#include "DependencyGraphAnalyzer.h"

#include "Tasks.h"

static void write_first_follow_file(const std::string &path, const std::map<std::string, std::set<FirstSetElement>> &first_set)
{
    // templates
    const std::string header_template =
        R"(
#ifndef %1%_H
#define %1%_H

#if defined(_MSVC_LANG)
    #define CXX_STD _MSVC_LANG
#else
    #define CXX_STD __cplusplus
#endif

#if CXX_STD < 201103L
    #error "This code requires at least C++11. Please use a compiler that supports C++11 or higher."
#endif


#include <map>
#include <set>
#include <string>

// Map<Production, Set<FIRST Token>>
extern const std::map<std::string, std::set<std::string>> __ebnf_first_set_token;
// Map<Production, Set<FIRST Literal>>
extern const std::map<std::string, std::set<std::string>> __ebnf_first_set_literal;

#endif // %1%_H
)";

    const std::string source_template =
        R"(
#include "%1%.h"

const std::map<std::string, std::set<std::string>> __ebnf_first_set_literal {
%2%
};

const std::map<std::string, std::set<std::string>> __ebnf_first_set_token {
%3%
};
)";

    std::filesystem::path filepath(path);
    std::string filename_base = filepath.filename().string();
    std::string filename_base_capital(filename_base);
    std::transform(filename_base_capital.begin(), filename_base_capital.end(), filename_base_capital.begin(), ::toupper);

    auto first_entry_production = boost::format(
        R"(    {
        "%1%", // production rule '%1%'
        {
%2%
        },
    },)");

    auto first_entry_element_literal = boost::format("            \"%1%\",");
    auto first_entry_element_token = boost::format("            \"%1%\", // from rule '%2%'");
    auto first_entry_element_reference = boost::format("            // $REF(\"%1%\")");

    // build content from templates
    std::vector<std::string> first_literal_map_initializers;
    std::vector<std::string> first_token_map_initializers;

    for (const auto &entry : first_set) {
        const std::string &production_id = entry.first;
        std::vector<std::string> first_literals;
        std::vector<std::string> first_tokens;

        for (const auto &f : entry.second) {
            switch (f.type) {
            case FirstSetElement::Literal:
                first_literals.push_back((first_entry_element_literal % f.value).str());
                break;
            case FirstSetElement::Epsilon:
                first_tokens.push_back("            // EPSILON");
                break;
            case FirstSetElement::Token:
                first_tokens.push_back((first_entry_element_token % f.value % (f.produced_by.empty() ? production_id : f.produced_by)).str());
                break;
            case FirstSetElement::Reference:
                first_tokens.push_back((first_entry_element_reference % f.value).str());
                break;
            }
        }

        first_literal_map_initializers.push_back((first_entry_production % production_id % boost::join(first_literals, "\n")).str());
        first_token_map_initializers.push_back((first_entry_production % production_id % boost::join(first_tokens, "\n")).str());
    }

    // Write the contents
    std::string header_filename = path + ".h";
    std::ofstream header_file(header_filename);
    if (!header_file) {
        spdlog::error("Could not open the file {} for writing.", header_filename);
    }
    header_file << boost::format(header_template) % filename_base_capital;
    if (header_file.fail()) {
        spdlog::error("Failed to write to the file {}.", header_filename);
    }
    header_file.close();

    std::string source_filename = path + ".cpp";
    std::ofstream source_file(source_filename);
    if (!source_file) {
        spdlog::error("Could not open the file {} for writing.", source_filename);
    }
    source_file
        << boost::format(source_template)
            % filename_base
            % boost::join(first_literal_map_initializers, "\n")
            % boost::join(first_token_map_initializers, "\n");
    if (source_file.fail()) {
        spdlog::error("Failed to write to the file {}.", source_filename);
    }
    source_file.close();

    std::cout << "Written to " << header_filename << " and " << source_filename << std::endl;
}

void calculate_first_follow_set(DependencyGraphAnalyzer &checker, bool check_conflicts, const std::string &output_path)
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

    if (!output_path.empty()) {
        write_first_follow_file(output_path, first_set);
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
