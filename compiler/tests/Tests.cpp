#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#include <spdlog/sinks/ostream_sink.h>

#include "ErrorManager.h"
#include "TerminalColor.h"
#include "Tests.h"

std::map<std::string, std::vector<TestCaseData>> __test_cases;

void load_test_cases(const std::string &directory_path)
{
    std::map<std::string, std::vector<TestCaseData>> test_cases;

    std::cout << "Loading test data from: " << directory_path << std::endl;
    for (const auto &dir : fs::directory_iterator(directory_path)) {
        if (dir.is_directory()) {
            const auto &test_group = dir.path().filename().string();
            auto &vec = test_cases[test_group];
            for (const auto &entry : fs::directory_iterator(dir.path())) {
                if (entry.path().extension() == ".hrml") {
                    try {
                        auto parsed = TestCaseData::parse_path(entry.path());
                        vec.push_back(std::move(parsed));
                    } catch (const std::invalid_argument &e) {
                        std::cerr << e.what() << std::endl; // Ignore files that don't match the pattern
                    }
                }
            }
        }
    }

    __test_cases.swap(test_cases);
}

void GlobalTestEnvironment::SetUp()
{
    ErrorManager::instance().add_common_filters();
}

void GlobalTestEnvironment::TearDown()
{
}

// E3001_fail_1.hrml
TestCaseData TestCaseData::parse_path(const fs::path &path)
{
    std::regex pattern(R"(([EWNX]\d*X?)_(fail|pass)_([^\.]+)\.hrml)");
    std::smatch matches;

    auto filename = path.filename().string();
    if (std::regex_match(filename, matches, pattern)) {
        std::string code = matches[1];
        bool should_pass = matches[2] == "pass";
        std::string testname = matches[3];
        return {
            .path = path.string(),
            .filename = filename,
            .code = code,
            .should_pass = should_pass,
            .testname = testname,
            .expect_code = code.find("X") == std::string::npos,
        };
    }

    throw std::invalid_argument("Filename does not match expected pattern: " + filename);
}

void TestCaseData::print_setup() const
{
    std::cout << "Setting up '" << filename << "'" << std::endl
              << "  >Expect [" << (should_pass ? "PASS" : "FAILURE") << "] " << __tc.C_DARK_BLUE << code << __tc.C_RESET << ": " << testname << std::endl;
}

int main(int argc, char **argv)
{
    const char *directory = nullptr;

    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]).starts_with("--gtest")) {
            continue;
        } else {
            directory = argv[i];
            break;
        }
    }

    if (directory == nullptr) {
        directory = FALLBACK_HRML_DIR;
    }

    load_test_cases(directory);

    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new GlobalTestEnvironment()); // Register the global fixture

    // ::testing::TestEventListeners &listeners = ::testing::UnitTest::GetInstance()->listeners();
    // listeners.Append(new VerboseTestListener());

    return RUN_ALL_TESTS();
}