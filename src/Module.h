#ifndef MODULE_H
#define MODULE_H

#include <vector>
#include <cstdint>
#include <string>

/**
 * @brief Represents the value types in WebAssembly.
 */
enum class ValueType : uint8_t {
    I32 = 0x7f,
    I64 = 0x7e,
    F32 = 0x7d,
    F64 = 0x7c,
};

/**
 * @brief Represents the values that variables can have.
 */
union Value {
    int32_t i32;
    int64_t i64;
    float   f32;
    double  f64;
};

/**
 * @brief Represents a function signature (parameter types and result types).
 * This corresponds to an entry in the Type Section (ID 1).
 */
struct FunctionType {
    std::vector<ValueType> params;
    std::vector<ValueType> results;
};

/**
 * @brief Represents the definition of a global variable.
 * This corresponds to an entry in the Global Section (ID 6).
 */
struct GlobalType {
    ValueType type;
    bool is_mutable;
    Value initial_value;
};

/**
 * @brief Represents a single, complete function, combining its signature
 * and its actual executable code.
 * This is populated from the Function Section (ID 3) and the Code Section (ID 10).
 */
struct Function {
    uint32_t type_index; // An index into the Module's 'types' vector.
    std::vector<ValueType> locals; // A list of local variables.
    std::vector<uint8_t> code; // The raw bytecode of the function body.
};

/**
 * @brief Represents an export from the module.
 * This corresponds to an entry in the Export Section (ID 7).
 */
struct Export {
    std::string name;
    uint8_t kind; // The kind of export: 0=func, 1=table, 2=mem, 3=global
    uint32_t index; // The index into the corresponding space (e.g., function index).
};


/**
 * @class Module
 * @brief A static, in-memory representation of a parsed .wasm file.
 *
 * This class acts as a blueprint, holding all the definitions that are read from the binary file.
 */
class Module {
public:
    std::vector<FunctionType> types;

    std::vector<uint32_t> function_type_indices;

    uint32_t memory_initial_pages = 0;

    std::vector<GlobalType> globals;

    std::vector<Export> exports;

    std::vector<Function> functions;
};

#endif //MODULE_H