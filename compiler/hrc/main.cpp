#include <cstdlib>
#include <spdlog/spdlog.h>

#include "ASTBuilder.h"
#include "ASTNodeGraphvizBuilder.h"
#include "CompilerOptions.h"
#include "FileManager.h"
#include "Formatter.h"
#include "HRLLexer.h"
#include "ParseTreeNodeForward.h"
#include "ParseTreeNodeGraphvizBuilder.h"
#include "RecursiveDescentParser.h"
#include "SymbolTableAnalyzer.h"
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

    HRLLexer lexer;
    std::vector<TokenPtr> tokens;

    bool ok = lexer.lex(file, fileManager.get_input_filename(), tokens);
    if (!ok) {
        spdlog::error("Error occured during lexical analysis");
        abort();
    }

    // For now, let's write to stdout if there's no output file available, or output fails to open.
    // We have only lexer stage.
    // When it comes to ParseTree it'll be complicated
    FILE *output = fileManager.open_output_file();
    if (output == nullptr) {
        output = stdout;
    }
    Utilities::write_token_list_to_file(output, tokens);

    fclose(file);

    hrl::parser::RecursiveDescentParser parser(tokens);
    hrl::parser::CompilationUnitPTNodePtr compilation_unit;
    bool parsed = parser.parse(compilation_unit);
    if (!parsed) {
        spdlog::error("Error occured during parsing");
        abort();
    }
    hrl::parser::ParseTreeNodeGraphvizBuilder graphviz(compilation_unit);
    graphviz.generate_graphviz("build/pt.dot");

    // hrl::parser::ParseTreeNodeFormatterVisitor formatter;
    // formatter.format(compilation_unit); // Format not yet supported

    hrl::parser::CompilationUnitASTNodePtr ast;
    hrl::parser::ASTBuilder builder(compilation_unit);
    if (!builder.build(ast)) {
        spdlog::error("Error occured during AST construction");
        abort();
    }

    using SemaAttrId = hrl::semanalyzer::SemAnalzyerASTNodeAttributeId;

    hrl::parser::ASTNodeGraphvizBuilder graphviz_ast(ast);
    graphviz_ast.generate_graphviz(
        "build/ast.dot");

    hrl::semanalyzer::SymbolTablePtr symbol_table;

    hrl::semanalyzer::SymbolTableAnalyzer symbol_table_builder(ast);
    if (!symbol_table_builder.build(symbol_table)) {
        spdlog::error("Error occured building symbol table");
        abort();
    }

    graphviz_ast.generate_graphviz("build/symtbl.dot",
        std::set<int>(
            {
                SemaAttrId::ATTR_SEMANALYZER_SYMBOL,
                SemaAttrId::ATTR_SEMANALYZER_SCOPE_INFO,
            }));

    return 0;
}
