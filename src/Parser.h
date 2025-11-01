#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <iostream>
#include <algorithm>

#include "Module.h"

const std::vector<uint8_t> magic_number = {0x00, 0x61, 0x73, 0x6d};
const std::vector<uint8_t> version = {0x01, 0x00, 0x00, 0x00};

class Parser {
public:
    /**
     * @brief Constructs a Parser with the binary data of a .wasm file.
     * @param binary The raw bytes of the wasm file.
     */
    explicit Parser(const std::vector<uint8_t>& binary);

    /**
     * @brief Parses the binary data and populates a Module object.
     * @param module The Module object to fill with parsed data.
     */
    void parse_into(Module& module);

private:

    const std::vector<uint8_t>& binary;
    size_t offset;

    uint8_t read_byte();

    uint32_t decode_leb128_u();

    void validate_header();

    void parse_type_section(Module& module);

    void parse_function_section(Module& module);

    void parse_memory_section(Module& module);

    void parse_global_section(Module& module);

    void parse_export_section(Module& module);

    void parse_code_section(Module& module);
};

#endif //PARSER_H