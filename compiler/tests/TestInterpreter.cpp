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

    std::set<std::string> enabled_test_group { "semanalyzer", "solutions" };

    for (const auto &[testgruop, cases] : __test_cases) {
        if (!enabled_test_group.contains(testgruop)) {
            continue;
        }

        for (const auto &testcase : cases) {
            if (testcase.should_pass && (!testcase.program_inputs.empty() || !testcase.expected_program_outputs.empty())) {
                result.push_back(testcase);
            }
        }
    }

    return result;
}

class InterpreterTests : public ::testing::TestWithParam<TestCaseData>, private WithSemanticAnalyzed {
protected:
    WithSemanticAnalyzed _test;
    WithSemanticAnalyzed _opt_test;
};

TEST_P(InterpreterTests, InterpreterCorrectnessTests)
{
    const auto &data = GetParam();
    bool ok;

    // begin unoptimized
    _test.setup_semantic_analyze(false, data, ok);
    ASSERT_TRUE(ok) << "Failed in semantic analysis stages";

    hrl::interpreter::MemoryManager memman;
    hrl::interpreter::IOManager ioman;
    hrl::interpreter::Accumulator accumulator(memman, ioman);

    hrl::interpreter::Interpreter interpreter(std::make_shared<std::string>(data.filename), _test.get_ast(), accumulator);
    interpreter.set_symbol_table(_test.get_symtbl());

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

    // begin optimized
    _opt_test.setup_semantic_analyze(true, data, ok);
    ASSERT_TRUE(ok) << "Failed in semantic analysis stages with opt";

    hrl::interpreter::MemoryManager opt_memman;
    hrl::interpreter::IOManager opt_ioman;
    hrl::interpreter::Accumulator opt_accumulator(opt_memman, opt_ioman);

    hrl::interpreter::Interpreter opt_interpreter(std::make_shared<std::string>(data.filename), _opt_test.get_ast(), opt_accumulator);
    opt_interpreter.set_symbol_table(_opt_test.get_symtbl());

    for (hrl::interpreter::HRMByte input : data.program_inputs) {
        opt_ioman.push_input(input);
    }

    std::vector<hrl::interpreter::HRMByte> opt_inputs, opt_outputs;

    opt_ioman.set_on_input_popped([&](hrl::interpreter::HRMByte val) {
        opt_inputs.push_back(val);
    });
    opt_ioman.set_on_output_pushed([&](hrl::interpreter::HRMByte val) {
        opt_outputs.push_back(val);
    });

    int rc = 0;
    try {
        rc = interpreter.run();
        ASSERT_EQ(rc, 0) << "Interpreter did not return a success code";
    } catch (const hrl::interpreter::InterpreterException &ex) {
        ASSERT_EQ(ex.get_error_type(), hrl::interpreter::InterpreterException::ErrorType::EndOfInput)
            << "Interpreter reported an error: " << ex.what();
    }

    try {
        rc = opt_interpreter.run();
        ASSERT_EQ(rc, 0) << "Interpreter (opt) did not return a success code";
    } catch (const hrl::interpreter::InterpreterException &ex) {
        ASSERT_EQ(ex.get_error_type(), hrl::interpreter::InterpreterException::ErrorType::EndOfInput)
            << "Interpreter (opt) reported an error: " << ex.what();
    }

    EXPECT_EQ(outputs, data.expected_program_outputs)
        << "The program output is incorrect";

    EXPECT_EQ(outputs, opt_outputs)
        << "Optimized program yields different output than unoptimized";
}

INSTANTIATE_TEST_SUITE_P(InterpreterOptTests, InterpreterTests, ::testing::ValuesIn(read_interpreter_test_cases()));
