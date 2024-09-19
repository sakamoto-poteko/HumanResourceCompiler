#include "WithSemanticAnalyzed.h"
#include "ClearSymbolTablePass.h"
#include "ConstantFoldingPass.h"
#include "ControlFlowVerificationPass.h"
#include "DeadCodeEliminationPass.h"
#include "ErrorManager.h"
#include "SemanticAnalysisPassManager.h"
#include "StripAttributePass.h"
#include "SymbolAnalysisPass.h"
#include "UnusedSymbolAnalysisPass.h"
#include "UseBeforeInitializationCheckPass.h"
#include "WithParsed.h"

WithSemanticAnalyzed::WithSemanticAnalyzed()
{
}

WithSemanticAnalyzed::~WithSemanticAnalyzed()
{
}

void WithSemanticAnalyzed::setup_semantic_analyze(bool optimize, const TestCaseData &data, bool &result)
{
    WithParsed::setup_parse(data, result);
    ASSERT_TRUE(result) << "Failed in parsing stages";

    hrl::semanalyzer::SemanticAnalysisPassManager sem_passmgr(ast, std::make_shared<std::string>(data.filename));

    // analyze, optimize and clean up
    if (optimize) {
        // won't mutate the node
        auto pre_symtbl_analyzer = sem_passmgr.add_pass<hrl::semanalyzer::SymbolAnalysisPass>("PreliminarySemanticAnalysisSymbolTableAnalyzer");
        auto ubi_preliminary = sem_passmgr.add_pass<hrl::semanalyzer::UseBeforeInitializationCheckPass>("PreliminaryUseBeforeInitializationCheckPass");
        // may mutate the node
        auto constfolder = sem_passmgr.add_pass<hrl::semanalyzer::ConstantFoldingPass>("ConstantFoldingPass");
        auto dce = sem_passmgr.add_pass<hrl::semanalyzer::DeadCodeEliminationPass>("DeadCodeElimination");
        auto unused_var = sem_passmgr.add_pass<hrl::semanalyzer::UnusedSymbolAnalysisPass>("UnusedVariableElimination");
        auto clear_symtbl = sem_passmgr.add_pass<hrl::semanalyzer::ClearSymbolTablePass>("ClearSymbolTablePass");
        auto strip_sym_attr = sem_passmgr.add_pass<hrl::semanalyzer::StripAttributePass>("StripSymbolAttributesPass");
    }

    // reannotate the node with symbol and scope
    auto post_symtbl_analyzer = sem_passmgr.add_pass<hrl::semanalyzer::SymbolAnalysisPass>("FinalSemanticAnalysisSymbolTableAnalyzer");
    auto ubi_final = sem_passmgr.add_pass<hrl::semanalyzer::UseBeforeInitializationCheckPass>("FinalUseBeforeInitializationCheckPass");
    auto cfv = sem_passmgr.add_pass<hrl::semanalyzer::ControlFlowVerificationPass>("ControlFlowVerificationPass");

    int sema_result = sem_passmgr.run(true);
    ErrorManager::instance().print_all();

    ASSERT_EQ(sema_result, 0) << "Expected semantic analysis to pass but it failed";

    symtbl = sem_passmgr.get_symbol_table();
}
