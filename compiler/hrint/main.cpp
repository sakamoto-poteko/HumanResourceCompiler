#include <cstdlib>
#include <string>

#include <spdlog/spdlog.h>

#include "ASTNodeForward.h"
#include "Accumulator.h"
#include "CompileAST.h"
#include "IOManager.h"
#include "Interpreter.h"
#include "InterpreterExceptions.h"
#include "InterpreterOptions.h"
#include "MemoryManager.h"
#include "SymbolTable.h"
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
    hrl::semanalyzer::SymbolTablePtr symtbl;
    int rc = compile_to_ast(options, ast, symtbl);
    if (rc) {
        exit(EXIT_FAILURE);
    }

    MemoryManager memman;
    IOManager ioman;
    Accumulator accumulator(memman, ioman);

    Interpreter interpreter(std::make_shared<std::string>(options.input_file), ast, accumulator);
    interpreter.set_symbol_table(symtbl);

    ioman.set_on_input_popped([](int val) {
        spdlog::info("Input: {}", val);
    });
    ioman.set_on_output_pushed([](int val) {
        spdlog::info("Output: {}", val);
    });

    ioman.push_input(1);
    ioman.push_input(2);
    ioman.push_input(3);
    ioman.push_input(4);
    ioman.push_input(5);
    ioman.push_input(6);
    ioman.push_input(7);
    ioman.push_input(8);
    ioman.push_input(9);
    ioman.push_input(10);

    try {
        int rc = interpreter.run();
    } catch (InterpreterException ex) {
        if (ex.get_error_type() == InterpreterException::ErrorType::EndOfInput) {
            spdlog::info("End of input reached");
        } else {
            spdlog::error(ex.what());
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
