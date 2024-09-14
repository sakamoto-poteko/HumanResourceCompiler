#include <algorithm>

#include "ClearSymbolTablePass.h"
#include "SemanticAnalysisPass.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

ClearSymbolTablePass::ClearSymbolTablePass(StringPtr filename, parser::CompilationUnitASTNodePtr root)
    : SemanticAnalysisPass(std::move(filename), std::move(root))
{
}

int ClearSymbolTablePass::run()
{
    _symbol_table->clear_symbols();
    return 0;
}

CLOSE_SEMANALYZER_NAMESPACE