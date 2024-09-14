#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <spdlog/sinks/ostream_sink.h>

#include "ErrorManager.h"
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

        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::invalid_argument("Failed to open file: " + filename);
        }
        std::string line;
        std::vector<std::string> expected_sentences;

        // Process each line in the file
        while (std::getline(file, line)) {
            // Remove leading whitespace
            std::string trimmed_line = boost::algorithm::trim_left_copy(line);

            const std::string expect_prefix = "// Expect: ";
            if (boost::algorithm::starts_with(trimmed_line, expect_prefix)) {
                // Extract the part after "// Expect: "
                std::string expected_sentence = trimmed_line.substr(expect_prefix.size());
                expected_sentences.push_back(expected_sentence);
            }
        }

        file.close(); // Close the file when done

        return {
            .path = path.string(),
            .filename = filename,
            .code = code,
            .should_pass = should_pass,
            .testname = testname,
            .expect_code = code.find("X") == std::string::npos,
            .expected_outputs = expected_sentences,
        };
    }

    throw std::invalid_argument("Filename does not match expected pattern: " + filename);
}

void TestCaseData::print_setup() const
{
    std::cout << "Setting up '" << filename << "'" << std::endl
              << "  >Expect '" << code << "' (" << (should_pass ? "PASS" : "FAILURE") << "): " << testname << std::endl;
    if (!expected_outputs.empty()) {
        std::cout << "  >Expect output with [" << boost::join(expected_outputs, ", ") << "]" << std::endl;
    }
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