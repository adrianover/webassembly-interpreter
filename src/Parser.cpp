#include "Parser.h"

Parser::Parser(const std::vector<uint8_t>& binary) : binary(binary), offset(0) {}

void Parser::parse_into(Module& module) {

    validate_header();
    offset = 8; // Move past the header to the first section

    while (offset < binary.size()) {
        uint8_t section_id = read_byte();
        uint32_t section_size = decode_leb128_u();
        size_t section_end = offset + section_size;

        switch (section_id) {
            case 1: // Type Section
                std::cout << "Parsing Type Section (ID 1)..." << std::endl;
                parse_type_section(module);
                break;
            case 3: // Function Section
                std::cout << "Parsing Function Section (ID 3)..." << std::endl;
                parse_function_section(module);
                break;
            case 5: // Memory Section
                std::cout << "Parsing Memory Section (ID 5)..." << std::endl;
                parse_memory_section(module);
                break;
            case 6: // Global Section
                std::cout << "Parsing Global Section (ID 6)..." << std::endl;
                parse_global_section(module);
                break;
            case 7: // Export Section
                std::cout << "Parsing Export Section (ID 7)..." << std::endl;
                parse_export_section(module);
                break;
            case 10: // Code Section
                std::cout << "Parsing Code Section (ID 10)..." << std::endl;
                parse_code_section(module);
                break;
            default:
                std::cout << "Skipping unhandled Section ID: " << (int)section_id << std::endl;
                break;
        }

        offset = section_end;
    }
}

uint8_t Parser::read_byte() {
    return binary[offset++];
}

uint32_t Parser::decode_leb128_u() {
    uint32_t result = 0;
    int shift = 0;
    while (true) {
        uint8_t byte = read_byte();
        result |= (byte & 0x7f) << shift;
        if ((byte & 0x80) == 0) {
            break;
        }
        shift += 7;
    }
    return result;
}

void Parser::validate_header() {
    if (binary.size() < 8) {
        throw std::runtime_error("File is too small to be a wasm module.");
    }

    if (!std::equal(magic_number.begin(), magic_number.end(), binary.begin())) {
        throw std::runtime_error("Invalid wasm magic number.");
    }
    if (!std::equal(version.begin(), version.end(), binary.begin() + 4)) {
        throw std::runtime_error("Unsupported wasm version.");
    }
    std::cout << "Wasm header validated." << std::endl;
}


void Parser::parse_type_section(Module& module) {
    uint32_t num_types = decode_leb128_u();
    module.types.reserve(num_types);
    for (uint32_t i = 0; i < num_types; ++i) {
        if (read_byte() != 0x60) { // Form must be 0x60 for 'func'
            throw std::runtime_error("Expected function type form 0x60");
        }
        FunctionType ftype;
        uint32_t num_params = decode_leb128_u();
        for (uint32_t p = 0; p < num_params; ++p) {
            ftype.params.push_back(static_cast<ValueType>(read_byte()));
        }
        uint32_t num_results = decode_leb128_u();
        for (uint32_t r = 0; r < num_results; ++r) {
            ftype.results.push_back(static_cast<ValueType>(read_byte()));
        }
        module.types.push_back(ftype);
    }
}

void Parser::parse_function_section(Module& module) {
    uint32_t num_functions = decode_leb128_u();
    module.function_type_indices.reserve(num_functions);
    for (uint32_t i = 0; i < num_functions; ++i) {
        module.function_type_indices.push_back(decode_leb128_u());
    }
}

void Parser::parse_memory_section(Module& module) {
    uint32_t num_memories = decode_leb128_u();
    if (num_memories > 0) {
        uint8_t flags = read_byte();
        module.memory_initial_pages = decode_leb128_u();
        if (flags == 0x01) {
            decode_leb128_u();
        }
    }
}

void Parser::parse_global_section(Module& module) {
    uint32_t num_globals = decode_leb128_u();
    for (uint32_t i = 0; i < num_globals; ++i) {
        GlobalType gtype;
        gtype.type = static_cast<ValueType>(read_byte());
        gtype.is_mutable = (read_byte() == 0x01);

        while(read_byte() != 0x0b);

        module.globals.push_back(gtype);
    }
}

void Parser::parse_export_section(Module& module) {
    uint32_t num_exports = decode_leb128_u();
    for (uint32_t i = 0; i < num_exports; ++i) {
        Export ex;
        uint32_t name_len = decode_leb128_u();
        ex.name = std::string(binary.begin() + offset, binary.begin() + offset + name_len);
        offset += name_len;

        ex.kind = read_byte();
        ex.index = decode_leb128_u();
        module.exports.push_back(ex);
    }
}

void Parser::parse_code_section(Module& module) {
    uint32_t num_functions = decode_leb128_u();
    if (num_functions != module.function_type_indices.size()) {
        throw std::runtime_error("Function and Code section counts mismatch.");
    }

    module.functions.reserve(num_functions);
    for (uint32_t i = 0; i < num_functions; ++i) {
        uint32_t body_size = decode_leb128_u();
        size_t body_end = offset + body_size;

        Function func;
        func.type_index = module.function_type_indices[i];

        uint32_t num_local_entries = decode_leb128_u();
        for (uint32_t j = 0; j < num_local_entries; ++j) {
            uint32_t count = decode_leb128_u();

            ValueType type = static_cast<ValueType>(read_byte());

            for (uint32_t k = 0; k < count; ++k) {
                func.locals.push_back(type);
            }
        }

        func.code.assign(binary.begin() + offset, binary.begin() + body_end -1);

        module.functions.push_back(func);
        offset = body_end;
    }
}