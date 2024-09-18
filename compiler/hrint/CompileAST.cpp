#include "CompileAST.h"

#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#include "ASTBuilder.h"
#include "ASTNodeForward.h"
#include "ClearSymbolTablePass.h"
#include "ConstantFoldingPass.h"
#include "ControlFlowVerificationPass.h"
#include "DeadCodeEliminationPass.h"
#include "ErrorManager.h"
#include "HRLLexer.h"
#include "InterpreterOptions.h"
#include "RecursiveDescentParser.h"
#include "SemanticAnalysisPassManager.h"
#include "StripAttributePass.h"
#include "SymbolAnalysisPass.h"
#include "SymbolTable.h"
#include "UnusedSymbolAnalysisPass.h"
#include "UseBeforeInitializationCheckPass.h"
#include "hrint_global.h"

OPEN_HRINT_NAMESPACE

#define CHECK_OK(ok)                      \
    ErrorManager::instance().print_all(); \
    if (!ok) {                            \
        return 1;                         \
    }

int compile_to_ast(const InterpreterOptions &options, hrl::parser::CompilationUnitASTNodePtr &ast, semanalyzer::SymbolTablePtr &symtbl)
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

    int sema_result = sem_passmgr.run(true);
    ErrorManager::instance().print_all();
    if (sema_result != 0) {
        return sema_result;
    }

    symtbl = sem_passmgr.get_symbol_table();
    return 0;
}

CLOSE_HRINT_NAMESPACE
// end