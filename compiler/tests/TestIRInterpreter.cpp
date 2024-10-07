#include <string>
#include <vector>

#include <boost/format.hpp>
#include <gtest/gtest.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

#include "IRInterpreter.h"
#include "InterpreterExceptions.h"
#include "Tests.h"
#include "WithIR.h"

static std::vector<TestCaseData> read_ir_interpreter_test_cases()
{
    std::vector<TestCaseData> result;

    std::set<std::string> enabled_test_group { "math", "semanalyzer", "solutions" };

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

class IRInterpreterTests : public ::testing::TestWithParam<TestCaseData>, private WithIR {
protected:
    WithIR _test;
    WithIR _opt_speed_test;
};

TEST_P(IRInterpreterTests, IRCorrectnessTests)
{
    const auto &data = GetParam();
    bool ok;

    // begin unoptimized
    _test.setup_ir(0, data, ok);
    ASSERT_TRUE(ok) << "Failed in IR optimization stages";

    hrl::interpreter::InterpreterMemoryManager memman;
    hrl::interpreter::InterpreterIOManager ioman;

    hrl::interpreter::IRInterpreter interpreter(ioman, memman, _test.get_program(), true);

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

    // begin optimized for speed
    _opt_speed_test.setup_ir(1, data, ok);
    ASSERT_TRUE(ok) << "Failed in semantic analysis stages with opt";

    hrl::interpreter::InterpreterMemoryManager opt_memman;
    hrl::interpreter::InterpreterIOManager opt_ioman;

    hrl::interpreter::IRInterpreter opt_interpreter(opt_ioman, opt_memman, _opt_speed_test.get_program(), true);

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
        rc = interpreter.exec();
        ASSERT_EQ(rc, 0) << "IR interpreter did not return a success code";
    } catch (const hrl::interpreter::InterpreterException &ex) {
        ASSERT_EQ(ex.get_error_type(), hrl::interpreter::InterpreterException::ErrorType::EndOfInput)
            << "IR interpreter reported an error: " << ex.what();
    }

    try {
        rc = opt_interpreter.exec();
        ASSERT_EQ(rc, 0) << "IR interpreter (opt) did not return a success code";
    } catch (const hrl::interpreter::InterpreterException &ex) {
        ASSERT_EQ(ex.get_error_type(), hrl::interpreter::InterpreterException::ErrorType::EndOfInput)
            << "IR interpreter (opt) reported an error: " << ex.what();
    }

    EXPECT_EQ(outputs, data.expected_program_outputs)
        << "IR interpreter: The program output is incorrect";

    EXPECT_EQ(outputs, opt_outputs)
        << "IR interpreter: Optimized program yields different output than unoptimized";
}

INSTANTIATE_TEST_SUITE_P(IRInterpreterOptTests, IRInterpreterTests, ::testing::ValuesIn(read_ir_interpreter_test_cases()));
