#ifndef WITHIR_H
#define WITHIR_H

#include "WithSemanticAnalyzed.h"

class WithIR : public WithSemanticAnalyzed {
public:
    WithIR() = default;
    virtual ~WithIR() = default;

    /**
     * @brief Set the up IR to test
     *
     * @param optimize Positive means for speed, negative for code size, 0 no opt
     * @param data
     * @param result
     */
    virtual void setup_ir(int optimize, const TestCaseData &data, bool &result);

private:
};

#endif