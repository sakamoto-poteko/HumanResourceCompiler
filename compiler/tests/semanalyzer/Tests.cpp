#define BOOST_TEST_MODULE SemAnalyzeTest

#include <boost/test/included/unit_test.hpp>

#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "ASTBuilder.h"
#include "ASTNodeGraphvizBuilder.h"
#include "ConstantFoldingPass.h"
#include "ErrorManager.h"
#include "Formatter.h"
#include "HRLLexer.h"
#include "ParseTreeNodeForward.h"
#include "ParseTreeNodeGraphvizBuilder.h"
#include "RecursiveDescentParser.h"
#include "SemanticAnalysisPassManager.h"
#include "SymbolAnalysisPass.h"
#include "TerminalColor.h"
#include "Tests.h"
#include "semanalyzer_global.h"

namespace fs = std::filesystem;

// Struct to hold parsed data
struct TestCaseData {
    std::string result;
    bool should_pass;
    std::string filename;
    std::string testname;
};

// Function to parse filenames like E3001_fail_1.hrml
TestCaseData parse_filename(const std::string &filename)
{
    std::regex pattern(R"((E\d+)_(fail|pass)_(\w+)\.hrml)");
    std::smatch matches;

    if (std::regex_match(filename, matches, pattern)) {
        std::string result = matches[1];
        bool should_pass = matches[2] == "pass";
        return {
            result,
            should_pass,
            filename,
            .testname = matches[3],
        };
    }

    throw std::invalid_argument("Filename does not match expected pattern: " + filename);
}

// Function to collect all .hrml files in the directory
std::vector<TestCaseData> collect_test_cases(const std::string &directory_path)
{
    std::vector<TestCaseData> test_cases;

    for (const auto &entry : fs::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".hrml") {
            try {
                test_cases.push_back(parse_filename(entry.path().filename().string()));
            } catch (const std::invalid_argument &e) {
                std::cerr << e.what() << std::endl; // Ignore files that don't match the pattern
            }
        }
    }

    return test_cases;
}

// Function to simulate file content reading and result checking
void run_test(const TestCaseData &test_case)
{
    // Simulate reading the file (for example purposes, we'll just check pass/fail status)
    std::string file_content = "Simulated file content for " + test_case.filename;

    FILE *file = std::fopen(test_case.filename.c_str(), "r");
    if (file == nullptr) {
        spdlog::error("Cannot open input file");
        exit(EXIT_FAILURE);
    }

    ErrorManager &errmgr = ErrorManager::instance();
    errmgr.add_common_filters();

    hrl::lexer::HRLLexer lexer;
    std::vector<hrl::lexer::TokenPtr> tokens;

    bool ok = lexer.lex(file, test_case.filename, tokens);
    BOOST_REQUIRE(ok);
    fclose(file);

    // Parser stage
    hrl::parser::RecursiveDescentParser parser(options.input_file, tokens);
    hrl::parser::CompilationUnitPTNodePtr compilation_unit;
    bool parsed = parser.parse(compilation_unit);
    if (!parsed) {
        errmgr.print_all();
        spdlog::error("Error occured during parsing");
        abort();
    }
    hrl::parser::ParseTreeNodeGraphvizBuilder graphviz(compilation_unit);
    graphviz.generate_graphviz("build/pt.dot");

    hrl::parser::CompilationUnitASTNodePtr ast;
    hrl::parser::ASTBuilder builder(compilation_unit);
    if (!builder.build(ast)) {
        errmgr.print_all();
        spdlog::error("Error occured during AST construction");
        abort();
    }

    hrl::parser::ASTNodeGraphvizBuilder graphviz_ast(ast);
    graphviz_ast.generate_graphviz("build/ast.dot");

    // Sem analysis stage
    using SemaAttrId = hrl::semanalyzer::SemAnalzyerASTNodeAttributeId;

    hrl::semanalyzer::SemanticAnalysisPassManager sem_passmgr(ast, std::make_shared<std::string>(options.input_file));

    auto symtbl_analyzer = sem_passmgr.add_pass<hrl::semanalyzer::SymbolAnalysisPass>(
        "SymbolTableAnalyzer",
        "build/symtbl.dot",
        std::set<int> {
            SemaAttrId::ATTR_SEMANALYZER_SYMBOL,
            SemaAttrId::ATTR_SEMANALYZER_SCOPE_INFO,
        });

    auto constfolder = sem_passmgr.add_pass<hrl::semanalyzer::ConstantFoldingPass>(
        "ConstantFoldingPass",
        "build/constfld.dot",
        std::set<int> {
            SemaAttrId::ATTR_SEMANALYZER_CONST_FOLDING_VALUE,
        });

    if (sem_passmgr.run(true) != 0) {
        errmgr.print_all();
        spdlog::error("Error occured running semantic analysis passes");
        abort();
    }

    errmgr.print_all();

    if (test_case.should_pass) {
        BOOST_CHECK_MESSAGE(true, "Test passed: " + test_case.result);
    } else {
        BOOST_CHECK_MESSAGE(false, "Test failed: " + test_case.result);
    }
}

BOOST_AUTO_TEST_CASE(dynamic_test_cases)
{
    // The directory path will be passed as an argument
    const char *directory_path = boost::unit_test::framework::master_test_suite().argv[1];

    // Collect test cases from the directory
    std::vector<TestCaseData> test_cases = collect_test_cases(directory_path);

    // Run the tests dynamically for each test case
    for (const auto &test_case : test_cases) {
        BOOST_TEST_CONTEXT("Running test for file: " << test_case.filename)
        {
            run_test(test_case);
        }
    }
}
