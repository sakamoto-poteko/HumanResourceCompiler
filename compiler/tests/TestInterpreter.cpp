#include <memory>
#include <string>
#include <vector>

#include <boost/format.hpp>
#include <gtest/gtest.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

#include "IntAccumulator.h"
#include "IntIOManager.h"
#include "IntMemoryManager.h"
#include "Interpreter.h"
#include "InterpreterExceptions.h"
#include "Tests.h"
#include "WithSemanticAnalyzed.h"

std::vector<TestCaseData> read_interpreter_test_cases()
{
    std::vector<TestCaseData> result;

    for (const auto &[_, cases] : __test_cases) {
        for (const auto &testcase : cases) {
            if (testcase.should_pass && (!testcase.program_inputs.empty() || !testcase.expected_program_outputs.empty())) {
                result.push_back(testcase);
            }
        }
    }

    return result;
}

class InterpreterTests : public ::testing::TestWithParam<TestCaseData>, public WithSemanticAnalyzed {
protected:
};

TEST_P(InterpreterTests, InterpreterCorrectnessTests)
{
    const auto &data = GetParam();
    bool ok;
    setup_semantic_analyze(false, data, ok);
    ASSERT_TRUE(ok) << "Failed in semantic analysis stages";

    hrl::interpreter::MemoryManager memman;
    hrl::interpreter::IOManager ioman;
    hrl::interpreter::Accumulator accumulator(memman, ioman);

    hrl::interpreter::Interpreter interpreter(std::make_shared<std::string>(data.filename), ast, accumulator);
    interpreter.set_symbol_table(symtbl);

    for (hrl::interpreter::HRMByte input : data.program_inputs) {
        ioman.push_input(input);
    }

    std::vector<hrl::interpreter::HRMByte> inputs, outputs;
    ioman.set_on_input_popped([&](hrl::interpreter::HRMByte val) {
        inputs.push_back(val);
    });
    ioman.set_on_output_pushed([&](hrl::interpreter::HRMByte val) {
        outputs.push_back(val);
    });

    int rc = 0;
    try {
        rc = interpreter.run();
        ASSERT_EQ(rc, 0) << "Interpreter did not return a success code";
    } catch (const hrl::interpreter::InterpreterException &ex) {
        ASSERT_EQ(ex.get_error_type(), hrl::interpreter::InterpreterException::ErrorType::EndOfInput)
            << "Interpreter reported an error: " << ex.what();
    }

    EXPECT_EQ(outputs, data.expected_program_outputs)
        << "The program output is incorrect";
}

INSTANTIATE_TEST_SUITE_P(InterpreterTests, InterpreterTests, ::testing::ValuesIn(read_interpreter_test_cases()));
