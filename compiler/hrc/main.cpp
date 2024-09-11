#include <cstdlib>
#include <spdlog/spdlog.h>

#include "ASTBuilder.h"
#include "ASTNodeGraphvizBuilder.h"
#include "CompilerOptions.h"
#include "ConstantFoldingPass.h"
#include "ErrorManager.h"
#include "FileManager.h"
#include "Formatter.h"
#include "HRLLexer.h"
#include "ParseTreeNodeForward.h"
#include "ParseTreeNodeGraphvizBuilder.h"
#include "RecursiveDescentParser.h"
#include "SemanticAnalysisPassManager.h"
#include "SymbolAnalysisPass.h"
#include "TerminalColor.h"
#include "Utilities.h"
#include "semanalyzer_global.h"

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

    // Sem analysis finished. Collecting data
    hrl::semanalyzer::SymbolTablePtr symbol_table = symtbl_analyzer->get_symbol_table();

    return 0;
}
