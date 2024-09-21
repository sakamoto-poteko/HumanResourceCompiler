#ifndef WITHPARSED_H
#define WITHPARSED_H

#include "ASTNodeForward.h"
#include "Tests.h"

class WithParsed {
public:
    WithParsed() = default;
    virtual ~WithParsed() = default;
    virtual void setup_parse(const TestCaseData &data, bool &result);
    virtual void print_test_info(const TestCaseData &data);

    const hrl::parser::CompilationUnitASTNodePtr &get_ast() const { return ast; }

    const std::ostringstream &get_captured_outstream() const { return captured_outstream; }

protected:
    std::ostringstream captured_outstream;
    hrl::parser::CompilationUnitASTNodePtr ast;

private:
};

#endif