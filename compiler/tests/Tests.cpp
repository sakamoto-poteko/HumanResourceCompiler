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

static bool parse_integer_line(const std::string &input_part, std::vector<int> &result)
{
    std::vector<std::string> tokens;
    boost::split(tokens, input_part, boost::is_any_of(","));

    for (auto &token : tokens) {
        int value;
        boost::algorithm::trim(token);
        if (std::from_chars(token.data(), token.data() + token.size(), value).ec != std::errc()) {
            return false;
        }
        result.push_back(value);
    }
    return true;
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
        std::vector<int> inputs;
        std::vector<int> outputs;

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

            const std::string input_prefix = "// Input:";
            if (boost::algorithm::starts_with(trimmed_line, input_prefix)) {
                std::string input_part = trimmed_line.substr(input_prefix.size());
                // Extract the part after "// Input: "
                std::vector<int> i;
                if (parse_integer_line(input_part, i)) {
                    i.swap(inputs);
                }
            }
            const std::string output_prefix = "// Output:";
            if (boost::algorithm::starts_with(trimmed_line, output_prefix)) {
                std::string output_part = trimmed_line.substr(output_prefix.size());
                // Extract the part after "// Output: "
                std::vector<int> i;
                if (parse_integer_line(output_part, i)) {
                    i.swap(outputs);
                }
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
            .expected_compiler_outputs = expected_sentences,
            .program_inputs = inputs,
            .expected_program_outputs = outputs,
        };
    }

    throw std::invalid_argument("Filename does not match expected pattern: " + filename);
}

void TestCaseData::print_setup() const
{
    std::cout << "Setting up '" << filename << "'" << std::endl
              << "  >Expect '" << code << "' (" << (should_pass ? "PASS" : "FAILURE") << "): " << testname << std::endl;
    if (!expected_compiler_outputs.empty()) {
        std::cout << "  >Expect output with [" << boost::join(expected_compiler_outputs, ", ") << "]" << std::endl;
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