#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/format.hpp>
#include <spdlog/spdlog.h>

#include "ASTInterpreter.h"
#include "ASTNodeForward.h"
#include "Compile.h"
#include "IRInterpreter.h"
#include "IRProgramStructure.h"
#include "IntAccumulator.h"
#include "IntIOManager.h"
#include "IntMemoryManager.h"
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
    switch (options.verbosity) {

    case VerbosityLevel::Normal:
        spdlog::set_level(spdlog::level::warn);
        break;
    case VerbosityLevel::Info:
        spdlog::set_level(spdlog::level::info);
        break;
    case VerbosityLevel::Debug:
        spdlog::set_level(spdlog::level::debug);
        break;
    case VerbosityLevel::Trace:
        spdlog::set_level(spdlog::level::trace);
        break;
    }

    int rc = 0;

    hrl::parser::CompilationUnitASTNodePtr ast;
    hrl::irgen::ProgramPtr program;
    hrl::semanalyzer::SymbolTablePtr symtbl;
    rc = compile_to_ast_and_hir(options, ast, program, symtbl);
    if (rc) {
        exit(EXIT_FAILURE);
    }
    rc = transform_hir(options, program);
    if (rc) {
        exit(EXIT_FAILURE);
    }

    MemoryManager memman;
    IOManager ioman;

    AbstractInterpreter *interpreter = nullptr;

    bool enforce_ssa = false;
    switch (options.compile_target) {
    case CompileTarget::AST:
        interpreter = new ASTInterpreter(std::make_shared<std::string>(options.input_file), ast, symtbl, ioman, memman);
        break;
    case CompileTarget::HIR_SSA:
        enforce_ssa = true;
        [[fallthrough]];
    case CompileTarget::HIR:
        interpreter = new IRInterpreter(ioman, memman, program, enforce_ssa);
        break;
    case CompileTarget::LIR_SSA:
        spdlog::error("Not yet implemented for LIR SSA");
        throw;
    default:
        spdlog::error("Unknown compile target. {}", __PRETTY_FUNCTION__);
        throw;
    }

    ioman.set_on_input_popped([](HRMByte val) {
        std::cout << __tc.C_BOLD << __tc.C_DARK_CYAN << boost::format("<< %1%") % val << __tc.C_RESET << std::endl;
    });
    ioman.set_on_output_pushed([](HRMByte val) {
        std::cout << __tc.C_BOLD << __tc.C_DARK_GREEN << boost::format("=> %1%") % val << __tc.C_RESET << std::endl;
    });

    for (HRMByte input : options.input_data) {
        ioman.push_input(input);
    }

    try {
        rc = interpreter->exec();
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

    delete interpreter;
    return 0;
}
