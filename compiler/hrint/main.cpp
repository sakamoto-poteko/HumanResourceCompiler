#include <cstdlib>
#include <string>

#include <spdlog/spdlog.h>

#include "ASTNodeForward.h"
#include "CompileAST.h"
#include "IntAccumulator.h"
#include "IntIOManager.h"
#include "IntMemoryManager.h"
#include "Interpreter.h"
#include "InterpreterExceptions.h"
#include "InterpreterOptions.h"
#include "SymbolTable.h"
#include "TerminalColor.h"

using namespace hrl::hrint;
using namespace hrl::interpreter;

int main(int argc, char **argv)
{
    spdlog::set_pattern("%^[%l]%$ %v");
    __tc.reset();

    InterpreterOptions options = parse_arguments(argc, argv);
    if (options.verbose) {
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::info);
    }

    int rc = 0;

    hrl::parser::CompilationUnitASTNodePtr ast;
    hrl::semanalyzer::SymbolTablePtr symtbl;
    rc = compile_to_ast(options, ast, symtbl);
    if (rc) {
        exit(EXIT_FAILURE);
    }

    MemoryManager memman;
    IOManager ioman;
    Accumulator accumulator(memman, ioman);

    Interpreter interpreter(std::make_shared<std::string>(options.input_file), ast, accumulator);
    interpreter.set_symbol_table(symtbl);

    ioman.set_on_input_popped([](HRMByte val) {
        spdlog::info("<< {}", val);
    });
    ioman.set_on_output_pushed([](HRMByte val) {
        spdlog::info("=> {}", val);
    });

    for (HRMByte input : options.input_data) {
        ioman.push_input(input);
    }

    try {
        rc = interpreter.run();
        if (rc != 0) {
            spdlog::error("The interpreter returned a non-success code: {}", rc);
            exit(EXIT_FAILURE);
        }
    } catch (const InterpreterException &ex) {
        if (ex.get_error_type() == InterpreterException::ErrorType::EndOfInput) {
            spdlog::info("End of input reached");
        } else {
            spdlog::error(ex.what());
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
