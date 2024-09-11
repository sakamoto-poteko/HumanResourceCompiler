// #define BOOST_TEST_ALTERNATIVE_INIT_API
// #define BOOST_TEST_MODULE SemAnalyzeTest

#include <filesystem>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <vector>

#include <boost/test/data/test_case.hpp>
#include <boost/test/included/unit_test.hpp>
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
namespace bdata = boost::unit_test::data;

struct TestCaseData {
    std::string path;
    std::string filename;
    std::string code;
    bool should_pass;
    std::string testname;

    // Overload the << operator for CustomData
    friend std::ostream &operator<<(std::ostream &os, const TestCaseData &data)
    {
        os << data.filename;
        return os;
    }
};

// E3001_fail_1.hrml
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

std::vector<TestCaseData> collect_test_cases(const std::string &directory_path)
{
    std::vector<TestCaseData> test_cases;

    for (const auto &entry : fs::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".hrml") {
            try {
                auto parsed = parse_path(entry.path());

                test_cases.push_back(std::move(parsed));
            } catch (const std::invalid_argument &e) {
                std::cerr << e.what() << std::endl; // Ignore files that don't match the pattern
            }
        }
    }

    return test_cases;
}

struct TestFixture {
    TestFixture() { }

    ~TestFixture() { }

    void setup_with_data(const TestCaseData &test_case)
    {
        data = test_case;
        BOOST_TEST_MESSAGE(boost::format("Setting up %1%: test '%4%', expect '%2%' and %3%")
            % data.filename
            % data.code
            % (data.should_pass ? "PASS" : "FAILURE")
            % data.testname);

        ErrorManager &errmgr = ErrorManager::instance();

        // Redirect spdlog output
        auto captured_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(captured_outstream);
        captured_sink->set_level(spdlog::level::trace);
        spdlog::logger logger(data.filename, { captured_sink });
        spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));
        spdlog::flush_on(spdlog::level::trace);

        // Open HRML
        FILE *file = std::fopen(data.path.c_str(), "r");
        BOOST_REQUIRE_MESSAGE(file != nullptr, boost::format("Failed to open HRML %1%") % data.path);

        // Lexing
        hrl::lexer::HRLLexer lexer;
        std::vector<hrl::lexer::TokenPtr> tokens;

        bool ok = lexer.lex(file, data.path, tokens);
        BOOST_REQUIRE_MESSAGE(ok, "Lexical analysis failed");
        fclose(file);

        // Parsing
        hrl::parser::RecursiveDescentParser parser(data.path, tokens);
        hrl::parser::CompilationUnitPTNodePtr compilation_unit;
        bool parsed = parser.parse(compilation_unit);
        BOOST_REQUIRE_MESSAGE(parsed, "Parsing failed");
        BOOST_REQUIRE_MESSAGE(!errmgr.has_errors(), "Parsing has errors");

        // Building AST
        hrl::parser::CompilationUnitASTNodePtr ast;
        hrl::parser::ASTBuilder builder(compilation_unit);
        bool built = builder.build(ast);
        BOOST_REQUIRE_MESSAGE(built, "Failed to build AST");
        BOOST_REQUIRE_MESSAGE(!errmgr.has_errors(), "Building AST has errors");
    }

    TestCaseData data;
    std::ostringstream captured_outstream;
    hrl::parser::CompilationUnitASTNodePtr ast;
};

std::vector<TestCaseData> test_case_data;

BOOST_DATA_TEST_CASE_F(TestFixture, SemanticAnalysisTest, bdata::make(test_case_data))
{
    setup_with_data(sample);

    hrl::semanalyzer::SemanticAnalysisPassManager sem_passmgr(ast, std::make_shared<std::string>(data.filename));
    auto symtbl_analyzer = sem_passmgr.add_pass<hrl::semanalyzer::SymbolAnalysisPass>("SymbolTableAnalyzer");
    auto constfolder = sem_passmgr.add_pass<hrl::semanalyzer::ConstantFoldingPass>("ConstantFoldingPass");
    int sema_result = sem_passmgr.run(true);
    ErrorManager::instance().print_all();

    bool out_has_code = captured_outstream.str().find("[" + data.code + "]") != std::string::npos;
    if (data.should_pass) {
        BOOST_REQUIRE_MESSAGE(sema_result == 0, "Expect semantic analysis to pass but failed");
        if (!data.code.starts_with("X")) {
            BOOST_REQUIRE_MESSAGE(out_has_code, boost::format("Expect '%1%' but not found in neither stdout nor stderr") % data.code);
        }
    } else {
        BOOST_REQUIRE_MESSAGE(sema_result != 0, "Expect semantic analysis to fail but passed");
        BOOST_REQUIRE_MESSAGE(out_has_code, boost::format("Expect '%1%' but not found in neither stdout nor stderr") % data.code);
    }

    BOOST_CHECK_MESSAGE(true, "Test passed: " + data.code);
}

bool init_unit_test()
{
    // Get command-line arguments from Boost.Test
    auto &master_test_suite = boost::unit_test::framework::master_test_suite();

    if (master_test_suite.argc < 2) {
        std::cerr << "Missing test HRML files path" << std::endl;
        return false;
    }

    const char *directory_path = master_test_suite.argv[1];
    BOOST_TEST_MESSAGE(boost::format("Loading test data from: %1%") % directory_path);
    test_case_data = collect_test_cases(directory_path);

    ErrorManager::instance().add_common_filters();

    return true;
}

boost::unit_test::test_suite* init_unit_test_suite(int /*argc*/, char* /*argv*/[])
{
  std::cout << "using obsolete init" << std::endl;
  return 0;
}