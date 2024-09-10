#define BOOST_TEST_MODULE SemAnalyzeTest

#include <filesystem>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

#include <spdlog/sinks/ostream_sink.h>

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
    std::string path;
    std::string filename;
    std::string code;
    bool should_pass;
    std::string testname;
};

// Function to parse filenames like E3001_fail_1.hrml
TestCaseData parse_path(const fs::path &path)
{
    std::regex pattern(R"(([EWNX]\d*)_(fail|pass)_(\w+)\.hrml)");
    std::smatch matches;

    auto filename = path.filename().string();
    if (std::regex_match(filename, matches, pattern)) {
        std::string code = matches[1];
        bool should_pass = matches[2] == "pass";
        std::string testname = matches[3];
        return {
            .path = path.string(),
            .filename = filename,
            .code = code,
            .should_pass = should_pass,
            .testname = testname,
        };
    }

    throw std::invalid_argument("Filename does not match expected pattern: " + filename);
}

// Function to simulate file content reading and result checking
void run_test(const TestCaseData &test_case)
{
    // Redirect spdlog output
    std::ostringstream captured_cout;
    std::ostringstream captured_cerr;
    auto cout_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(captured_cout);
    auto cerr_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(captured_cerr);

    ErrorManager &errmgr = ErrorManager::instance();
    errmgr.add_common_filters();

    // Open HRML
    FILE *file = std::fopen(test_case.path.c_str(), "r");
    BOOST_REQUIRE_MESSAGE(file != nullptr, boost::format("Failed to open HRML %1%") % test_case.path);

    // Lexing
    hrl::lexer::HRLLexer lexer;
    std::vector<hrl::lexer::TokenPtr> tokens;

    bool ok = lexer.lex(file, test_case.path, tokens);
    BOOST_REQUIRE_MESSAGE(ok, "Lexical analysis failed");
    fclose(file);

    // Parsing
    hrl::parser::RecursiveDescentParser parser(test_case.path, tokens);
    hrl::parser::CompilationUnitPTNodePtr compilation_unit;
    bool parsed = parser.parse(compilation_unit);
    BOOST_REQUIRE_MESSAGE(parsed, "Parsing failed");
    BOOST_REQUIRE_MESSAGE(!errmgr.has_errors(), "Parsing has errors");
    // hrl::parser::ParseTreeNodeGraphvizBuilder graphviz(compilation_unit);
    // graphviz.generate_graphviz("build/pt.dot");

    // Building AST
    hrl::parser::CompilationUnitASTNodePtr ast;
    hrl::parser::ASTBuilder builder(compilation_unit);
    bool built = builder.build(ast);
    BOOST_REQUIRE_MESSAGE(built, "Failed to build AST");
    BOOST_REQUIRE_MESSAGE(!errmgr.has_errors(), "Building AST has errors");

    // hrl::parser::ASTNodeGraphvizBuilder graphviz_ast(ast);
    // graphviz_ast.generate_graphviz("build/ast.dot");

    // Semantic Analysis
    hrl::semanalyzer::SemanticAnalysisPassManager sem_passmgr(ast, std::make_shared<std::string>(test_case.filename));
    auto symtbl_analyzer = sem_passmgr.add_pass<hrl::semanalyzer::SymbolAnalysisPass>("SymbolTableAnalyzer");
    auto constfolder = sem_passmgr.add_pass<hrl::semanalyzer::ConstantFoldingPass>("ConstantFoldingPass");
    int sema_result = sem_passmgr.run(true);

    bool cout_has = captured_cout.str().find(test_case.code) != std::string::npos;
    bool cerr_has = captured_cerr.str().find(test_case.code) != std::string::npos;
    if (test_case.should_pass) {
        BOOST_REQUIRE_MESSAGE(sema_result == 0, "Expect semantic analysis to pass but failed");
        if (!test_case.code.starts_with("X")) {
            BOOST_REQUIRE_MESSAGE(cout_has || cerr_has, boost::format("Expect '%1%' but not found in neither stdout nor stderr") % test_case.code);
        }
    } else {
        BOOST_REQUIRE_MESSAGE(sema_result == 0, "Expect semantic analysis to fail but passed");
        BOOST_REQUIRE_MESSAGE(cout_has || cerr_has, boost::format("Expect '%1%' but not found in neither stdout nor stderr") % test_case.code);
    }

    BOOST_CHECK_MESSAGE(true, "Test passed: " + test_case.code);
}

// Function to collect all .hrml files in the directory
std::vector<TestCaseData> collect_test_cases(const std::string &directory_path)
{
    std::vector<TestCaseData> test_cases;

    for (const auto &entry : fs::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".hrml") {
            try {
                auto parsed = parse_path(entry.path());
                BOOST_TEST_MESSAGE(boost::format("Adding file %1%: test '%4%', expect '%2%' and %3%")
                    % parsed.filename % parsed.code % (parsed.should_pass ? "PASS" : "FAILURE") % parsed.testname);
                test_cases.push_back(std::move(parsed));
            } catch (const std::invalid_argument &e) {
                std::cerr << e.what() << std::endl; // Ignore files that don't match the pattern
            }
        }
    }

    return test_cases;
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
