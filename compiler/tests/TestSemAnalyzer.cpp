#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <boost/format.hpp>
#include <gtest/gtest.h>
#include <spdlog/sinks/ostream_sink.h>

#include "ClearSymbolTablePass.h"
#include "ConstantFoldingPass.h"
#include "ControlFlowVerificationPass.h"
#include "DeadCodeEliminationPass.h"
#include "ErrorManager.h"
#include "SemanticAnalysisPassManager.h"
#include "StripAttributePass.h"
#include "SymbolAnalysisPass.h"
#include "Tests.h"
#include "UnusedSymbolAnalysisPass.h"
#include "UseBeforeInitializationCheckPass.h"
#include "WithParsed.h"

std::vector<TestCaseData> read_semanalyzer_test_cases()
{
    return __test_cases["semanalyzer"];
}

class SemanticAnalyzerTests : public ::testing::TestWithParam<TestCaseData>, public WithParsed {
};

TEST_P(SemanticAnalyzerTests, SemanticAnalysisTests)
{
    const auto &data = GetParam();
    bool ok;
    setup_parse(data, ok);
    ASSERT_TRUE(ok) << "Failed in pre semantic analysis stages";

    using SemaAttrId = hrl::semanalyzer::SemAnalzyerASTNodeAttributeId;

    // it's for conditional breakpoint
    auto dbg = data.filename == "W3006_pass_var_shadow_global_block.hrml";
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

    auto unused_var = sem_passmgr.add_pass<hrl::semanalyzer::UnusedSymbolAnalysisPass>(
        "UnusedVariableElimination",
        data.filename + "-uvar.dot",
        std::set<int> {
            SemaAttrId::ATTR_SEMANALYZER_SYMBOL,
            SemaAttrId::ATTR_SEMANALYZER_SCOPE_INFO,
        });

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

    for (const auto &exp : data.expected_compiler_outputs) {
        ASSERT_TRUE(captured.find(exp) != std::string::npos) << "Expected output but not found: '" << exp << "'";
    }
}

INSTANTIATE_TEST_SUITE_P(CompilerMessageTests, SemanticAnalyzerTests, ::testing::ValuesIn(read_semanalyzer_test_cases()));
