#include "Compile.h"

#include <spdlog/spdlog.h>
#include <string>
#include <vector>

#include "ASTBuilder.h"
#include "ASTNodeForward.h"
#include "AnalyzeLivenessPass.h"
#include "BuildControlFlowGraphPass.h"
#include "BuildSSAPass.h"
#include "ClearSymbolTablePass.h"
#include "ConstantFoldingPass.h"
#include "ControlFlowVerificationPass.h"
#include "DeadCodeEliminationPass.h"
#include "EliminateDeadBasicBlockPass.h"
#include "ErrorManager.h"
#include "HRLLexer.h"
#include "IROptimizationPassManager.h"
#include "InterpreterOptions.h"
#include "MergeConditionalBranchPass.h"
#include "RecursiveDescentParser.h"
#include "RenumberVariableIdPass.h"
#include "SemanticAnalysisPassManager.h"
#include "StripAttributePass.h"
#include "StripEmptyBasicBlockPass.h"
#include "StripUselessInstructionPass.h"
#include "SymbolAnalysisPass.h"
#include "SymbolTable.h"
#include "TACGen.h"
#include "UnusedSymbolAnalysisPass.h"
#include "UseBeforeInitializationCheckPass.h"
#include "VerifySSAPass.h"
#include "hrint_global.h"

OPEN_HRINT_NAMESPACE

#define CHECK_OK(ok)                      \
    ErrorManager::instance().print_all(); \
    if (!ok) {                            \
        return 1;                         \
    }

int compile_to_ast_and_hir(const InterpreterOptions &options, hrl::parser::CompilationUnitASTNodePtr &ast, irgen::ProgramPtr &program, semanalyzer::SymbolTablePtr &symtbl)
{
    FILE *file = std::fopen(options.input_file.c_str(), "r");

    // Lexing
    hrl::lexer::HRLLexer lexer;
    std::vector<hrl::lexer::TokenPtr> tokens;
    bool ok = lexer.lex(file, options.input_file, tokens);
    fclose(file);
    CHECK_OK(ok);

    // Parsing
    hrl::parser::RecursiveDescentParser parser(options.input_file, tokens);
    hrl::parser::CompilationUnitPTNodePtr compilation_unit;
    bool parsed = parser.parse(compilation_unit);
    CHECK_OK(parsed);

    // Building AST
    hrl::parser::ASTBuilder builder(compilation_unit);
    bool built = builder.build(ast);
    CHECK_OK(built);

    hrl::semanalyzer::SemanticAnalysisPassManager sem_passmgr(ast, std::make_shared<std::string>(options.input_file));

    // analyze, optimize and clean up
    if (options.enable_opt) {
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
    auto tacgen = sem_passmgr.add_pass<hrl::irgen::TACGen>("TACGen");

    int sema_result = sem_passmgr.run(true);
    ErrorManager::instance().print_all();
    if (sema_result != 0) {
        return sema_result;
    }

    symtbl = sem_passmgr.get_symbol_table();
    program = tacgen->get_built_program();
    return 0;
}

int transform_hir(const InterpreterOptions &options, const irgen::ProgramPtr &program)
{
    if (options.compile_target < CompileTarget::HIR) {
        return 0;
    }

    hrl::irgen::IROptimizationPassManager irop_passmgr(program);
    irop_passmgr.add_pass<hrl::irgen::StripUselessInstructionPass>(
        "StripNoOpPass",
        "build/strnop.hrasm",
        "build/strnop.dot");
    irop_passmgr.add_pass<hrl::irgen::StripEmptyBasicBlockPass>(
        "StripEmptyBasicBlockPass",
        "build/strebb.hrasm",
        "build/strebb.dot");
    irop_passmgr.add_pass<hrl::irgen::BuildControlFlowGraphPass>(
        "ControlFlowGraphBuilderPass",
        "build/cfgbuilder.hrasm",
        "build/cfgbuilder.dot");
    irop_passmgr.add_pass<hrl::irgen::MergeConditionalBranchPass>(
        "MergeCondBrPass",
        "build/mgcondbr.hrasm",
        "build/mgcondbr.dot");
    irop_passmgr.add_pass<hrl::irgen::EliminateDeadBasicBlockPass>(
        "EliminateDeadBasicBlockPass",
        "build/edbb.hrasm",
        "build/edbb.dot");
    irop_passmgr.add_pass<hrl::irgen::AnalyzeLivenessPass>(
        "AnalyzeLivenessPassPreSSA",
        "build/liveness.hrasm",
        "build/liveness.dot",
        "build/liveness.yml");

    if (options.compile_target >= CompileTarget::HIR_SSA) {
        irop_passmgr.add_pass<hrl::irgen::BuildSSAPass>(
            "BuildSSAPass",
            "build/ssa.hrasm",
            "build/ssa.dot");
        irop_passmgr.add_pass<hrl::irgen::RenumberVariableIdPass>(
            "SSARenumberVariableId",
            "build/ssa-renum.hrasm",
            "build/ssa-renum.dot");
        irop_passmgr.add_pass<hrl::irgen::VerifySSAPass>("VerifySSA");
        irop_passmgr.add_pass<hrl::irgen::AnalyzeLivenessPass>(
            "AnalyzeLivenessPassPostSSA",
            "build/liveness-ssa.hrasm",
            "build/liveness-ssa.dot",
            "build/liveness-ssa.yml");
        // there's no opt passes yet
    }

    ErrorManager::instance().print_all();
    int irgen_result = irop_passmgr.run(true);
    if (irgen_result != 0) {
        spdlog::error("Error occured running ir optimization passes");
        return irgen_result;
    }
    return 0;
}

CLOSE_HRINT_NAMESPACE
// end