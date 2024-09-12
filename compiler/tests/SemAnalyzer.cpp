#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <spdlog/sinks/ostream_sink.h>

#include "ASTBuilder.h"
#include "ConstantFoldingPass.h"
#include "ErrorManager.h"
#include "HRLLexer.h"
#include "ParseTreeNodeForward.h"
#include "RecursiveDescentParser.h"
#include "SemanticAnalysisPassManager.h"
#include "SymbolAnalysisPass.h"
#include "SymbolTable.h"
#include "Tests.h"
#include "UseBeforeInitializationCheckPass.h"

#define RETURN_FALSE_IF(x) \
    if (x) {               \
        return false;      \
    }

std::vector<TestCaseData> read_semanalyzer_test_cases()
{
    return __test_cases["semanalyzer"];
}

class SemanticAnalyzerTests : public ::testing::TestWithParam<TestCaseData> {
protected:
    std::ostringstream captured_outstream;
    hrl::parser::CompilationUnitASTNodePtr ast;

    void setup(const TestCaseData &data, bool &result)
    {
        result = false;

        data.print_setup();

        ErrorManager &errmgr = ErrorManager::instance();
        errmgr.clear();

        // Redirect spdlog output
        auto captured_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(captured_outstream);
        captured_sink->set_level(spdlog::level::trace);
        spdlog::set_default_logger(std::make_shared<spdlog::logger>(data.filename, captured_sink));
        spdlog::flush_on(spdlog::level::trace);

        // Open HRML file
        FILE *file = std::fopen(data.path.c_str(), "r");
        ASSERT_NE(file, nullptr) << "Failed to open HRML " << data.path;

        // Lexing
        hrl::lexer::HRLLexer lexer;
        std::vector<hrl::lexer::TokenPtr> tokens;
        bool ok = lexer.lex(file, data.path, tokens);
        ASSERT_TRUE(ok) << "Lexical analysis failed";
        fclose(file);

        // Parsing
        hrl::parser::RecursiveDescentParser parser(data.path, tokens);
        hrl::parser::CompilationUnitPTNodePtr compilation_unit;
        bool parsed = parser.parse(compilation_unit);
        ASSERT_TRUE(parsed) << "Parsing failed";
        ASSERT_FALSE(errmgr.has_errors()) << "Parsing has errors";

        // Building AST
        hrl::parser::ASTBuilder builder(compilation_unit);
        bool built = builder.build(ast);
        ASSERT_TRUE(built) << "Failed to build AST";
        ASSERT_FALSE(errmgr.has_errors()) << "Building AST has errors";

        result = true;
    }
};

TEST_P(SemanticAnalyzerTests, SemanticAnalysisTests)
{
    const auto &data = GetParam();
    bool ok;
    setup(data, ok);
    ASSERT_TRUE(ok) << "Failed in pre semantic analysis stages";

    hrl::semanalyzer::SemanticAnalysisPassManager sem_passmgr(ast, std::make_shared<std::string>(data.filename));
    auto symtbl = std::make_shared<hrl::semanalyzer::SymbolTable>();
    // non-node-mutational
    auto symtbl_analyzer = sem_passmgr.add_pass<hrl::semanalyzer::SymbolAnalysisPass>("SymbolTableAnalyzer");
    auto ubi1 = sem_passmgr.add_pass<hrl::semanalyzer::UseBeforeInitializationCheckPass>("UseBeforeInitializationCheckPass1");
    // node-mutated
    auto constfolder = sem_passmgr.add_pass<hrl::semanalyzer::ConstantFoldingPass>("ConstantFoldingPass");

    symtbl_analyzer->set_symbol_table(symtbl);
    ubi1->set_symbol_table(symtbl);

    int sema_result = sem_passmgr.run(true);
    ErrorManager::instance().print_all();

    bool out_has_code = captured_outstream.str().find("[" + data.code + "]") != std::string::npos;

    if (data.should_pass) {
        ASSERT_EQ(sema_result, 0) << "Expected semantic analysis to pass but it failed";
    } else {
        ASSERT_NE(sema_result, 0) << "Expected semantic analysis to fail but it passed";
    }

    if (data.expect_code) {
        ASSERT_TRUE(out_has_code) << "Expected '" << data.code << "' but not found in output";
    }
}

INSTANTIATE_TEST_SUITE_P(CompilerMessageTests, SemanticAnalyzerTests, ::testing::ValuesIn(read_semanalyzer_test_cases()));
