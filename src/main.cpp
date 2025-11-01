#include <iostream>
#include <fstream>
#include <vector>

#include "module.h"
#include "Parser.h"

std::vector<uint8_t> load_wasm_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("Failed to open file: " + path);
    return {(std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()};
}

int main(int argc, char* argv[]) {

    auto wasm_binary = load_wasm_file(argv[1]);
    Module module;
    Parser parser(wasm_binary);
    parser.parse_into(module);
    std::cout << "Module parsed successfully.\n" << std::endl;

    return 0;
}