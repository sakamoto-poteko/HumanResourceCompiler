#include <spdlog/spdlog.h>

#include "CompilerOptions.h"
#include "FileManager.h"
#include "HRLLexer.h"
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
    std::vector<GCToken> tokens;

    int result = lexer.lex(file, fileManager.get_input_filename(), tokens);
    if (result != 0) {
        spdlog::error("Error occured during lexical analysis");
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
    return 0;
}
