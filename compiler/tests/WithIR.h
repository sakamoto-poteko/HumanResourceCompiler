#ifndef WITHIR_H
#define WITHIR_H

#include "WithSemanticAnalyzed.h"

class WithIR : public WithSemanticAnalyzed {
public:
    WithIR() = default;
    virtual ~WithIR() = default;

    virtual void setup_ir(int optimize, const TestCaseData &data, bool &result);

private:
};

#endif