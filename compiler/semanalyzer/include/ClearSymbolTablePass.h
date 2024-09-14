#ifndef CLEARSYMBOLTABLEPASS_H
#define CLEARSYMBOLTABLEPASS_H

#include "SemanticAnalysisPass.h"
#include "WithSymbolTable.h"
#include "semanalyzer_global.h"

OPEN_SEMANALYZER_NAMESPACE

class ClearSymbolTablePass : public SemanticAnalysisPass, public WithSymbolTable {
public:
    ClearSymbolTablePass(StringPtr filename, parser::CompilationUnitASTNodePtr root);
    ~ClearSymbolTablePass() = default;

    int run() override;

private:
};

CLOSE_SEMANALYZER_NAMESPACE

#endif