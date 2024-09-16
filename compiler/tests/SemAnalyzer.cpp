#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <boost/format.hpp>
#include <gtest/gtest.h>
#include <spdlog/sinks/ostream_sink.h>

#include "ASTBuilder.h"
#include "ASTNodeGraphvizBuilder.h"
#include "ClearSymbolTablePass.h"
#include "ConstantFoldingPass.h"
#include "ControlFlowVerificationPass.h"
#include "DeadCodeEliminationPass.h"
#include "ErrorManager.h"
#include "HRLLexer.h"
#include "ParseTreeNodeForward.h"
#include "ParseTreeNodeGraphvizBuilder.h"
#include "RecursiveDescentParser.h"
#include "SemanticAnalysisPassManager.h"
#include "StripAttributePass.h"
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
        errmgr.print_all();
        ASSERT_TRUE(ok) << "Lexical analysis failed\n"
                        << captured_outstream.str();
        ASSERT_FALSE(errmgr.has_errors()) << "Lexing has errors:\n"
                                          << captured_outstream.str();
        fclose(file);

        // Parsing
        hrl::parser::RecursiveDescentParser parser(data.path, tokens);
        hrl::parser::CompilationUnitPTNodePtr compilation_unit;
        bool parsed = parser.parse(compilation_unit);
        errmgr.print_all();
        ASSERT_TRUE(parsed) << "Parsing failed\n"
                            << captured_outstream.str();
        ASSERT_FALSE(errmgr.has_errors()) << "Parsing has errors:\n"
                                          << captured_outstream.str();
        hrl::parser::ParseTreeNodeGraphvizBuilder graphviz(compilation_unit);
        graphviz.generate_graphviz(data.filename + "-pt.dot");

        // Building AST
        hrl::parser::ASTBuilder builder(compilation_unit);
        bool built = builder.build(ast);
        errmgr.print_all();
        ASSERT_TRUE(built) << "Failed to build AST\n"
                           << captured_outstream.str();
        ASSERT_FALSE(errmgr.has_errors()) << "Building AST has errors\n"
                                          << captured_outstream.str();
        hrl::parser::ASTNodeGraphvizBuilder graphviz_ast(ast);
        graphviz_ast.generate_graphviz(data.filename + "-ast.dot");

        result = true;
    }
};

TEST_P(SemanticAnalyzerTests, SemanticAnalysisTests)
{
    const auto &data = GetParam();
    bool ok;
    setup(data, ok);
    ASSERT_TRUE(ok) << "Failed in pre semantic analysis stages";

    using SemaAttrId = hrl::semanalyzer::SemAnalzyerASTNodeAttributeId;

    // it's for conditional breakpoint
    auto dbg = data.filename == "E3011X_pass_if_then.hrml";
    UNUSED(dbg);

    hrl::semanalyzer::SemanticAnalysisPassManager sem_passmgr(ast, std::make_shared<std::string>(data.filename));

    // won't mutate the node
    auto pre_symtbl_analyzer = sem_passmgr.add_pass<hrl::semanalyzer::SymbolAnalysisPass>(
        "1stSemanticAnalysisSymbolTableAnalyzer",
        data.filename + "-symtbl.pre.dot",
        std::set<int> {
            SemaAttrId::ATTR_SEMANALYZER_SYMBOL,
            SemaAttrId::ATTR_SEMANALYZER_SCOPE_INFO,
        });
    auto ubi_preliminary = sem_passmgr.add_pass<hrl::semanalyzer::UseBeforeInitializationCheckPass>(
        "1stUseBeforeInitializationCheckPass1",
        data.filename + "-ubi1.dot",
        std::set<int> {
            SemaAttrId::ATTR_SEMANALYZER_SYMBOL,
            SemaAttrId::ATTR_SEMANALYZER_SCOPE_INFO,
        });

    // may mutate the node
    auto constfolder = sem_passmgr.add_pass<hrl::semanalyzer::ConstantFoldingPass>(
        "ConstantFoldingPass",
        data.filename + "-constfold.dot",
        std::set<int> {
            SemaAttrId::ATTR_SEMANALYZER_CONST_FOLDING_VALUE,
        });

    // reannotate the node with symbol and scope
    auto dce = sem_passmgr.add_pass<hrl::semanalyzer::DeadCodeEliminationPass>(
        "DeadCodeElimination",
        data.filename + "-dce.dot",
        std::set<int> {});

    auto clear_symtbl = sem_passmgr.add_pass<hrl::semanalyzer::ClearSymbolTablePass>("ClearSymbolTablePass");
    auto strip_sym_attr = sem_passmgr.add_pass<hrl::semanalyzer::StripAttributePass>("StripSymbolAttributesPass");

    // reannotate the node with symbol and scope
    auto post_symtbl_analyzer = sem_passmgr.add_pass<hrl::semanalyzer::SymbolAnalysisPass>(
        "2ndSemanticAnalysisSymbolTableAnalyzer",
        data.filename + "-symtbl.post.dot",
        std::set<int> {
            SemaAttrId::ATTR_SEMANALYZER_SYMBOL,
            SemaAttrId::ATTR_SEMANALYZER_SCOPE_INFO,
        });
    auto ubi_final = sem_passmgr.add_pass<hrl::semanalyzer::UseBeforeInitializationCheckPass>(
        "2ndUseBeforeInitializationCheckPass",
        data.filename + "-ubi2.dot",
        std::set<int> {
            SemaAttrId::ATTR_SEMANALYZER_SYMBOL,
            SemaAttrId::ATTR_SEMANALYZER_SCOPE_INFO,
        });
    auto cfv = sem_passmgr.add_pass<hrl::semanalyzer::ControlFlowVerificationPass>(
        "ControlFlowVerificationPass",
        data.filename + "-cfv.dot",
        std::set<int> {
            SemaAttrId::ATTR_SEMANALYZER_CONTROL_CONTEXT_INFO,
        });

    strip_sym_attr->add_attribute(SemaAttrId::ATTR_SEMANALYZER_SCOPE_INFO);
    strip_sym_attr->add_attribute(SemaAttrId::ATTR_SEMANALYZER_SYMBOL);

    int sema_result = sem_passmgr.run(true);
    ErrorManager::instance().print_all();
    cfv->generate_return_graph(data.filename + "-cfv-ret.dot");

    std::string captured = captured_outstream.str();
    bool out_has_code = captured.find("[" + data.code + "]") != std::string::npos;

    if (data.should_pass) {
        if (sema_result != 0 && !captured.empty()) {
            std::cerr << captured << std::endl;
        }
        ASSERT_EQ(sema_result, 0) << "Expected semantic analysis to pass but it failed";
    } else {
        std::string result_code = boost::str(boost::format("%04d") % sema_result);

        bool correctly_failed_with_code = data.code.find(result_code) != std::string::npos;
        if (!correctly_failed_with_code) {
            std::cerr << captured << std::endl;
        }
        ASSERT_TRUE(correctly_failed_with_code)
            << "Expected semantic analysis to fail with " << data.code << " but got " << sema_result;
    }

    if (data.expect_code) {
        ASSERT_TRUE(out_has_code) << "Expected '" << data.code << "' but not found in output, result code " << sema_result;
    }

    for (const auto &exp : data.expected_outputs) {
        ASSERT_TRUE(captured.find(exp) != std::string::npos) << "Expected output but not found: '" << exp << "'";
    }
}

INSTANTIATE_TEST_SUITE_P(CompilerMessageTests, SemanticAnalyzerTests, ::testing::ValuesIn(read_semanalyzer_test_cases()));
