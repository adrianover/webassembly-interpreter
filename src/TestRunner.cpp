#include "TestRunner.h"
#include <fstream>
#include <iostream>
#include <optional>

TestRunner::TestRunner(const std::string& wat_path) {
    parse_tests_from_wat(wat_path);
}

void TestRunner::parse_tests_from_wat(const std::string& wat_path) {
    std::ifstream file(wat_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open .wat file for parsing tests: " + wat_path);
    }

    std::string line;

    std::string pending_description;
    std::optional<int32_t> pending_expected_result;

    while (std::getline(file, line)) {
        // Find a new test based on the Test label
        size_t test_pos = line.find(";; Test:");
        if (test_pos != std::string::npos) {
            pending_description = line.substr(test_pos + 9);
            pending_expected_result.reset();
        }

        // Find the expected result
        size_t expected_pos = line.find(";; Expected result at address");
        if (expected_pos != std::string::npos) {
            std::string result_str = line.substr(expected_pos + 31);
            size_t digit_pos = result_str.find_first_of("-0123456789");
            if (digit_pos != std::string::npos) {
                pending_expected_result = std::stoi(result_str.substr(digit_pos));
            }
        }

        // Look for the function index line
        size_t func_pos = line.find("(func (;");
        if (func_pos != std::string::npos) {

            if (!pending_description.empty() && pending_expected_result.has_value()) {
                TestCase test;
                test.description = pending_description;
                test.expected_result = *pending_expected_result;

                size_t end_pos = line.find(";) (type");
                std::string index_str = line.substr(func_pos + 8, end_pos - (func_pos + 8));
                test.function_index = std::stoi(index_str);

                test_cases.push_back(test);

                pending_description.clear();
                pending_expected_result.reset();
            }
        }
    }
    std::cout << "Parsed " << test_cases.size() << " test cases from " << wat_path << "." << std::endl;
}

bool TestRunner::run(const Module& module) {
    int passed_count = 0;
    Interpreter interpreter(module);

    for (const auto& test : test_cases) {
        std::cout << "\n--- Running Test: " << test.description << " (func " << test.function_index << ") ---" << std::endl;

        try {
            interpreter.invoke(test.function_index);
            int32_t result = interpreter.get_memory_i32(0);

            std::cout << "  Result at mem[0]: " << result << std::endl;
            std::cout << "  Expected result:  " << test.expected_result << std::endl;

            if (result == test.expected_result) {
                std::cout << "  SUCCESS" << std::endl;
                passed_count++;
            } else {
                std::cout << "  FAILURE" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "  ERROR during execution: " << e.what() << std::endl;
            std::cout << "  FAILURE" << std::endl;
        }
    }

    std::cout << "\n===== Test Summary =====\n";
    std::cout << "Passed: " << passed_count << "/" << test_cases.size() << std::endl;
    std::cout << "========================\n";

    return passed_count == test_cases.size();
}