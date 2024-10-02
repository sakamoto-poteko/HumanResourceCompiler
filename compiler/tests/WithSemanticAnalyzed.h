#ifndef WITHSEMANTICANALYZED_H
#define WITHSEMANTICANALYZED_H

#include "IRProgramStructure.h"
#include "SymbolTable.h"
#include "WithParsed.h"

class WithSemanticAnalyzed : public WithParsed {
public:
    WithSemanticAnalyzed();
    virtual ~WithSemanticAnalyzed();
    virtual void setup_semantic_analyze(bool optimize, const TestCaseData &data, bool &result);

    hrl::semanalyzer::SymbolTablePtr get_symtbl() { return symtbl; }

    hrl::irgen::ProgramPtr get_program() { return program; }

protected:
    hrl::semanalyzer::SymbolTablePtr symtbl;
    hrl::irgen::ProgramPtr program;
};

#endif