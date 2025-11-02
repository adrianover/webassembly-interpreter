#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <iomanip>
#include <fstream>

#include "../src/Parser.h"
#include "../src/Module.h"
#include "../src/Interpreter.h"
#include "test_01.cpp"
#include "test_02.cpp"
#include "test_03.cpp"
#include "test_04.cpp"
#include "test_05.cpp"
#include "test_06.cpp"
#include "test_07.cpp"
#include "test_08.cpp"
#include "test_09.cpp"

const std::vector all_suites_to_run = {
    test_01,
    test_02,
    test_03,
    test_04,
    test_05,
    test_06,
    test_07,
    test_08,
    test_09,
};

std::vector<uint8_t> load_wasm_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("Failed to open file: " + path);
    return {(std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()};
}

int main() {
    int total_passed = 0;
    int total_ran = 0;

    for (const auto& suite : all_suites_to_run) {
        std::cout << "\n=================================================" << std::endl;
        std::cout << "  RUNNING SUITE: " << suite.name << std::endl;
        std::cout << "=================================================\n" << std::endl;
        int suite_passed_count = 0;

        try {
            auto wasm_binary = load_wasm_file(suite.wasm_path);
            Module my_module;
            Parser parser(wasm_binary);
            parser.parse_into(my_module);
            Interpreter interpreter(my_module);

            for (const auto& test : suite.tests) {
                std::cout << "Running: " << test.name << std::endl;
                try {
                    interpreter.invoke(test.function_index_to_run);
                    if (test.verify(interpreter)) {
                        std::cout << "SUCCESS" << std::endl;
                        suite_passed_count++;
                    } else {
                        std::cout << "FAILURE" << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cout << "ERROR: " << e.what() << std::endl;
                }
                std::cout << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "FATAL ERROR loading suite '" << suite.name << "': " << e.what() << std::endl;
        }

        total_passed += suite_passed_count;
        total_ran += suite.tests.size();
        std::cout << "Suite Summary: " << suite_passed_count << " / " << suite.tests.size() << " passed." << std::endl;
    }

    std::cout << "=================================================" << std::endl;
    std::cout << "  OVERALL SUMMARY: " << total_passed << " / " << total_ran << " passed." << std::endl;
    std::cout << "=================================================\n" << std::endl;

    return (total_passed == total_ran) ? 0 : 1;
}