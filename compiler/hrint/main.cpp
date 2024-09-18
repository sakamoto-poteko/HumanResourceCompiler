#include <cstdlib>
#include <string>

#include <spdlog/spdlog.h>

#include "ASTNodeForward.h"
#include "CompileAST.h"
#include "Interpreter.h"
#include "InterpreterOptions.h"
#include "TerminalColor.h"

using namespace hrl::hrint;

int main(int argc, char **argv)
{
    spdlog::set_pattern("%^[%l]%$ %v");
    __tc.reset();

    InterpreterOptions options = parse_arguments(argc, argv);
    if (options.verbose) {
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::warn);
    }

    hrl::parser::CompilationUnitASTNodePtr ast;
    int rc = compile_to_ast(options, ast);
    if (rc) {
        exit(EXIT_FAILURE);
    }

    Interpreter interpreter(std::make_shared<std::string>(options.input_file), ast);
    auto &ioman = interpreter.get_io_manager();
    ioman.push_input(1);
    ioman.push_input(1);
    ioman.push_input(1);
    ioman.push_input(1);
    ioman.push_input(1);
    ioman.push_input(1);
    ioman.push_input(1);
    ioman.push_input(1);
    ioman.push_input(1);
    
    interpreter.run();

    return 0;
}
