#ifndef COMPILEAST_H
#define COMPILEAST_H

#include "ASTNodeForward.h"
#include "IRProgramStructure.h"
#include "InterpreterOptions.h"
#include "SymbolTable.h"
#include "hrint_global.h"

OPEN_HRINT_NAMESPACE

int compile_to_ast_and_hir(const InterpreterOptions &options, hrl::parser::CompilationUnitASTNodePtr &ast, irgen::ProgramPtr &program, semanalyzer::SymbolTablePtr &symtbl);
int transform_hir(const InterpreterOptions &options, const irgen::ProgramPtr &program);

CLOSE_HRINT_NAMESPACE

#endif