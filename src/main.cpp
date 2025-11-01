#include <iostream>
#include <fstream>
#include <vector>

#include "Module.h"
#include "Parser.h"
#include "TestRunner.h"

std::vector<uint8_t> load_wasm_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("Failed to open file: " + path);
    return {(std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()};
}

int main(int argc, char* argv[]) {
    // std::string wasm_path = argv[1];
    // std::string wat_path = argv[2];

    std::vector<std::string> wasm_files = {
        "../01_test.wasm",
        "../02_test_prio1.wasm",
        "../03_test_prio2.wasm"
    };
    std::vector<std::string> wat_files = {
        "../01_test.wat",
        "../02_test_prio1.wat",
        "../03_test_prio2.wat",
    };

    bool all_passed = true;

    for (int i = 0; i < wat_files.size(); i++) {
        std::string wasm_path = wasm_files[i];
        std::string wat_path = wat_files[i];

        auto wasm_binary = load_wasm_file(wasm_path);
        // 2. Parse the binary into a Module object
        Module my_module;
        Parser parser(wasm_binary);
        parser.parse_into(my_module);
        std::cout << "Module parsed successfully." << std::endl;

        // 3. Create a TestRunner and run the tests
        TestRunner test_runner(wat_path);
        all_passed &= test_runner.run(my_module);

        // Return an exit code indicating success or failure
    }

    return all_passed ? 0 : 1;
}