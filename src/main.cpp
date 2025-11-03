#include <fstream>
#include <vector>

#include "Module.h"
#include "Parser.h"

std::vector<uint8_t> load_wasm_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("Failed to open file: " + path);
    return {(std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()};
}

int main(int argc, char* argv[]) {
    std::string wasm_path = argv[1];

    auto wasm_binary = load_wasm_file(wasm_path);
    Module my_module;
    Parser parser(wasm_binary);
    parser.parse_into(my_module);

    return 0;
}