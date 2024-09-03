#include <cstdlib>
#include <spdlog/spdlog.h>

#include "ASTNodeForward.h"
#include "ASTNodeGraphvizBuilder.h"
#include "CompilerOptions.h"
#include "FileManager.h"
#include "Formatter.h"
#include "HRLLexer.h"
#include "RecursiveDescentParser.h"
#include "TerminalColor.h"
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

    HRLLexer lexer;
    std::vector<TokenPtr> tokens;

    bool ok = lexer.lex(file, fileManager.get_input_filename(), tokens);
    if (!ok) {
        spdlog::error("Error occured during lexical analysis");
        abort();
    }

    // For now, let's write to stdout if there's no output file available, or output fails to open.
    // We have only lexer stage.
    // When it comes to AST it'll be complicated
    FILE *output = fileManager.open_output_file();
    if (output == nullptr) {
        output = stdout;
    }
    Utilities::write_token_list_to_file(output, tokens);

    fclose(file);

    hrl::parser::RecursiveDescentParser parser(tokens);
    hrl::parser::CompilationUnitNodePtr compilation_unit;
    bool parsed = parser.parse(compilation_unit);
    if (!parsed) {
        spdlog::error("Error occured during parsing");
        abort();
    }
    hrl::parser::ASTNodeGraphvizBuilder graphviz(compilation_unit);
    graphviz.generate_graphviz();

    hrl::parser::ASTNodeFormatterVisitor formatter;
    formatter.format(compilation_unit);

    return 0;
}
