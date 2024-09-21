#ifndef WITHSEMANTICANALYZED_H
#define WITHSEMANTICANALYZED_H

#include "SymbolTable.h"
#include "WithParsed.h"

class WithSemanticAnalyzed : public WithParsed {
public:
    WithSemanticAnalyzed();
    virtual ~WithSemanticAnalyzed();
    virtual void setup_semantic_analyze(bool optimize, const TestCaseData &data, bool &result);

    hrl::semanalyzer::SymbolTablePtr get_symtbl() { return symtbl; }

protected:
    hrl::semanalyzer::SymbolTablePtr symtbl;
};

#endif