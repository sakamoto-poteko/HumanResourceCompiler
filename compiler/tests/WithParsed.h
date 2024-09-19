#ifndef WITHPARSED_H
#define WITHPARSED_H

#include "ASTNodeForward.h"
#include "Tests.h"

class WithParsed {
public:
    WithParsed() = default;
    virtual ~WithParsed() = default;

protected:
    std::ostringstream captured_outstream;
    hrl::parser::CompilationUnitASTNodePtr ast;

    virtual void print_test_info(const TestCaseData &data);
    virtual void setup_parse(const TestCaseData &data, bool &result);

private:
};

#endif