#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "Module.h"
#include <vector>
#include <functional>
#include <stdexcept>

/**
 * @struct StackFrame
 * @brief Represents the state of a single function call on the call stack.
 *
 * Each time a function is called, a new StackFrame is created and pushed onto
 * the call stack. It contains all the state necessary to resume a parent function
 * after a nested call completes.
 */
struct StackFrame {
    const Function* func;     // Pointer to the function being executed
    size_t pc;                // Program counter for that function
    std::vector<Value> locals;  // The values of the locals for this call
    size_t control_stack_base;  // The base of this frame with reference to the stack
};

/**
 * @struct ControlFrame
 * @brief Represents a structured control flow block like `block`, `loop`, or `if`.
 *
 * These are pushed onto the control stack to manage the targets for branch instructions.
 */
struct ControlFrame {
    uint8_t opcode; // The opcode that created this block (if, block, loop)
    size_t end;     // PC to jump to end the block
    size_t start;   // PC to jump to start the block.
};

static constexpr size_t PAGE_SIZE = 65536; // Todo

/**
 * @class Interpreter
 * @brief Executes the bytecode of a parsed WebAssembly Module.
 *
 * The Interpreter is a stack-based virtual machine that processes Wasm instructions
 * sequentially. It is initialized with a static Module blueprint and manages all
 * runtime state, including the call stack, operand stack, and linear memory.
 */
class Interpreter {
public:
    explicit Interpreter(const Module& module);

    /**
     * @brief Begins execution by invoking a function by its index. This is the main entry point.
     * @param function_index The index of the function to call in the module's function space.
     */
    void invoke(uint32_t function_index);

    /**
     * @brief Retrieves a 32-bit integer from the interpreter's linear memory.
     *
     * @param address The byte address to read from.
     * @return The i32 value at the specified address.
     */
    int32_t get_memory_i32(uint32_t address) const;

    /**
     * @brief Retrieves a 32-bit float from the interpreter's linear memory.
     *
     * @param address The byte address to read from.
     * @return The f32 value at the specified address.
     */
    float get_memory_f32(uint32_t address) const;

private:
    void execute();
    std::pair<size_t, size_t> scan_block_body(size_t start_pc, const std::vector<uint8_t>& code);
    void perform_branch(uint32_t label_index);

    void op_function_call(const Function &func, size_t &pc);
    void op_select();
    void op_loop(const Function &func, size_t &pc, uint8_t opcode);
    void op_if(const Function &func, size_t &pc, uint8_t opcode);
    void op_end(size_t &pc);
    void op_br_if(const Function &func, size_t &pc);
    void op_return();
    void op_mem_size(const Function &func, size_t &pc);
    void op_grow(const Function &func, size_t &pc);

    const Module& module;
    std::vector<Value> stack;
    std::vector<uint8_t> memory;
    std::vector<Value> globals;
    std::vector<StackFrame> call_stack;
    std::vector<ControlFrame> control_stack;

    template <typename T>
    void push(T value) {
        if constexpr (std::is_same_v<T, int32_t>) {
            stack.push_back({.i32 = value});
        } else if constexpr (std::is_same_v<T, int64_t>) {
            stack.push_back({.i64 = value});
        } else if constexpr (std::is_same_v<T, float>) {
            stack.push_back({.f32 = value});
        } else if constexpr (std::is_same_v<T, double>) {
            stack.push_back({.f64 = value});
        }
    }

    template <typename T>
    T pop() {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow");
        }
        Value val = stack.back();
        stack.pop_back();

        if constexpr (std::is_same_v<T, int32_t>) {
            return val.i32;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return val.i64;
        } else if constexpr (std::is_same_v<T, float>) {
            return val.f32;
        } else if constexpr (std::is_same_v<T, double>) {
            return val.f64;
        }
    }

    template <typename T>
    T decode_leb128_s(const std::vector<uint8_t>& code, size_t &pc);

    template <typename T>
    T decode_leb128_u(const std::vector<uint8_t>& code, size_t& pc);

    template <typename T>
    void store(uint32_t address, T value) {
        if (address + sizeof(T) > memory.size()) {
            throw std::runtime_error("Memory access out of bounds: store");
        }
        std::memcpy(&memory[address], &value, sizeof(T));
    }

    template <typename T>
    T load(uint32_t address) const {
        if (address + sizeof(T) > memory.size()) {
            throw std::runtime_error("Memory access out of bounds: load");
        }
        T value;
        std::memcpy(&value, &memory[address], sizeof(T));
        return value;
    }

    template <typename DestT, typename SourceT>
    void execute_reinterpret_op() {
        SourceT source_val = pop<SourceT>();
        DestT dest_val;
        static_assert(sizeof(source_val) == sizeof(dest_val), "Reinterpret types must be the same size");
        std::memcpy(&dest_val, &source_val, sizeof(dest_val));
        push<DestT>(dest_val);
    }

    template <typename DestT, typename SourceT>
    void execute_conversion_op(const std::function<DestT(SourceT)>& op) {
        push<DestT>(op(pop<SourceT>()));
    }

    template <typename T>
    void execute_unary_op(const std::function<T(T)>& op) {
        push<T>(op(pop<T>()));
    }

    template <typename T>
    void execute_binary_op(const std::function<T(T, T)>& op) {
        T b = pop<T>();
        T a = pop<T>();
        push<T>(op(a, b));
    }

    template <typename T>
    void execute_comparison_op(const std::function<bool(T, T)>& op) {
        T b = pop<T>();
        T a = pop<T>();
        push<int32_t>(op(a, b) ? 1 : 0);
    }

    template <typename T>
    T read_immediate(const std::vector<uint8_t>& code, size_t& pc) {
        T value;
        std::memcpy(&value, &code[pc], sizeof(T));
        pc += sizeof(T);
        return value;
    }
};

template <typename T>
T Interpreter::decode_leb128_s(const std::vector<uint8_t>& code, size_t &pc) {
    T result = 0;
    int shift = 0;
    uint8_t byte;
    constexpr int bit_width = sizeof(T) * 8;

    do {
        byte = code[pc++];
        result |= (static_cast<T>(byte & 0x7f) << shift);
        shift += 7;
    } while (byte & 0x80);

    if ((shift < bit_width) && (byte & 0x40)) {
        result |= (~static_cast<T>(0) << shift);
    }

    return result;
}

template <typename T>
T Interpreter::decode_leb128_u(const std::vector<uint8_t>& code, size_t& pc) {
    T result = 0;
    int shift = 0;
    while (true) {
        uint8_t byte = code[pc++];
        result |= (static_cast<T>(byte & 0x7f) << shift);
        if ((byte & 0x80) == 0) {
            break;
        }
        shift += 7;
    }
    return result;
}

#endif //INTERPRETER_H