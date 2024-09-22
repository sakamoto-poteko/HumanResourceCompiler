#include <cstdlib>

#include <spdlog/spdlog.h>

#include "ASTBuilder.h"
#include "ASTNodeGraphvizBuilder.h"
#include "ClearSymbolTablePass.h"
#include "CompilerOptions.h"
#include "ConstantFoldingPass.h"
#include "ControlFlowVerificationPass.h"
#include "DeadCodeEliminationPass.h"
#include "ErrorManager.h"
#include "FileManager.h"
#include "HRLLexer.h"
#include "ParseTreeNodeForward.h"
#include "ParseTreeNodeGraphvizBuilder.h"
#include "RecursiveDescentParser.h"
#include "SemanticAnalysisPassManager.h"
#include "StripAttributePass.h"
#include "SymbolAnalysisPass.h"
#include "TACGen.h"
#include "TerminalColor.h"
#include "UnusedSymbolAnalysisPass.h"
#include "UseBeforeInitializationCheckPass.h"
#include "Utilities.h"

using namespace hrl::lexer;
using namespace hrl::hrc;

int main(int argc, char **argv)
{
    spdlog::set_pattern("%^[%l]%$ %v");
    __tc.reset();

    CompilerOptions options = parse_arguments(argc, argv);
    if (options.verbose) {
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::warn);
    }

    FileManager fileManager(options.input_file, options.output_file, options.include_paths);

    FILE *file = fileManager.open_input_file();
    if (file == nullptr) {
        spdlog::error("Cannot open input file");
        exit(EXIT_FAILURE);
    }

    ErrorManager &errmgr = ErrorManager::instance();
    errmgr.add_common_filters();

    HRLLexer lexer;
    std::vector<TokenPtr> tokens;

    bool ok = lexer.lex(file, fileManager.get_input_file_path(), tokens);
    if (!ok) {
        errmgr.print_all();
        spdlog::error("Error occured during lexical analysis");
        abort();
    }

    // bullshit
    FILE *output = fileManager.open_output_file();
    if (output == nullptr) {
        output = stdout;
    } else {
        Utilities::write_token_list_to_file(output, tokens);
    }

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

    hrl::semanalyzer::SemanticAnalysisPassManager sem_passmgr(ast, std::make_shared<std::string>(options.input_file));

    // analyze, optimize and clean up
    if (true) {
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

    if (sem_passmgr.run(true) != 0) {
        errmgr.print_all();
        spdlog::error("Error occured running semantic analysis passes");
        abort();
    }

    tacgen->print();

    // Sem analysis finished. Collecting data
    hrl::semanalyzer::SymbolTablePtr symbol_table = sem_passmgr.get_symbol_table();

    return 0;
}
