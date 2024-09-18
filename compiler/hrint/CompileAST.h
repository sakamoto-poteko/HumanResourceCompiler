#ifndef COMPILEAST_H
#define COMPILEAST_H

#include "ASTNodeForward.h"
#include "InterpreterOptions.h"
#include "hrint_global.h"

OPEN_HRINT_NAMESPACE

int compile_to_ast(const InterpreterOptions &options, hrl::parser::CompilationUnitASTNodePtr &ast);

CLOSE_HRINT_NAMESPACE

#endif