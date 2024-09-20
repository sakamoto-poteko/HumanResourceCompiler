#ifndef TESTS_H
#define TESTS_H

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "HRMByte.h"

namespace fs = std::filesystem;

class GlobalTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override;

    void TearDown() override;
};

struct TestCaseData {
    std::string path;
    std::string filename;
    std::string code;
    bool should_pass;
    std::string testname;
    bool expect_code;
    std::vector<std::string> expected_compiler_outputs;
    std::vector<hrl::interpreter::HRMByte> program_inputs;
    std::vector<hrl::interpreter::HRMByte> expected_program_outputs;

    // Overload the << operator for CustomData
    friend std::ostream &operator<<(std::ostream &os, const TestCaseData &data)
    {
        os << data.filename;
        return os;
    }

    static TestCaseData parse_path(const fs::path &path);

    void parse_hrml_file();

    void print_setup() const;
};

class VerboseTestListener : public ::testing::TestEventListener {
public:
    // Called before a test starts
    void OnTestStart(const ::testing::TestInfo &test_info) override
    {
        std::cout << "Running test: " << test_info.test_case_name()
                  << "." << test_info.name();
        if (test_info.value_param() != nullptr) {
            std::cout << " (param: " << *static_cast<const char *>(test_info.value_param()) << ")";
        }
        std::cout << std::endl;
    }

    // Called after a test ends (whether it passed or failed)
    void OnTestEnd(const ::testing::TestInfo &test_info) override
    {
        if (test_info.result()->Passed()) {
            std::cout << "Test passed: " << test_info.test_case_name()
                      << "." << test_info.name() << std::endl;
        }
    }

    // Empty methods for other event handling (you can omit them if unused)
    void OnTestProgramStart(const ::testing::UnitTest & /*unit_test*/) override { }

    void OnTestProgramEnd(const ::testing::UnitTest & /*unit_test*/) override { }

    void OnTestIterationStart(const ::testing::UnitTest & /*unit_test*/, int /*iteration*/) override { }

    void OnTestIterationEnd(const ::testing::UnitTest & /*unit_test*/, int /*iteration*/) override { }

    void OnTestCaseStart(const ::testing::TestCase & /*test_case*/) override { }

    void OnTestCaseEnd(const ::testing::TestCase & /*test_case*/) override { }

    void OnTestPartResult(const ::testing::TestPartResult & /*test_part_result*/) override { }

    void OnEnvironmentsSetUpStart(const ::testing::UnitTest &) override { }

    void OnEnvironmentsSetUpEnd(const ::testing::UnitTest &) override { }

    void OnEnvironmentsTearDownStart(const ::testing::UnitTest &) override { }

    void OnEnvironmentsTearDownEnd(const ::testing::UnitTest &) override { }
};

extern std::map<std::string, std::vector<TestCaseData>> __test_cases;

#endif