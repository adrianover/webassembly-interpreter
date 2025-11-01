#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "Module.h"
#include <vector>
#include <functional>


union Value {
    int32_t i32;
    int64_t i64;
    float   f32;
    double  f64;
};

struct StackFrame {
    const Function* func;     // Pointer to the function being executed
    size_t pc;                // Program counter for that function
    std::vector<Value> locals;  // The values of the locals for this call
};

static constexpr size_t PAGE_SIZE = 65536; // Todo

class Interpreter {
public:
    explicit Interpreter(const Module& module);

    void invoke(uint32_t function_index);

    int32_t get_memory_i32(uint32_t address) const;


private:
    void execute();
    int32_t decode_leb128_s(const std::vector<uint8_t>& code, size_t &pc);
    uint32_t decode_leb128_u(const std::vector<uint8_t> code, size_t& pc);

    const Module& module;
    std::vector<Value> stack;
    std::vector<uint8_t> memory;
    std::vector<Value> globals;
    std::vector<StackFrame> call_stack;

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

    template <typename DestT, typename SourceT>
    DestT load(int32_t address) const {
        constexpr size_t num_bytes = sizeof(SourceT);

        uint64_t raw_bits = 0;
        for (size_t i = 0; i < num_bytes; ++i) {
            raw_bits |= (static_cast<uint64_t>(memory[address + i]) << (i * 8));
        }
        return static_cast<DestT>(static_cast<SourceT>(raw_bits));
    }

    template <typename T>
    void store(int32_t address, T value) {
        constexpr size_t num_bytes = sizeof(T);
        if (address + num_bytes > memory.size()) {
            throw std::runtime_error("Memory access out of bounds");
        }

        uint64_t bits = 0;
        if constexpr (num_bytes == 1) { // For int8_t
            bits = *reinterpret_cast<const uint8_t*>(&value);
        } else if constexpr (num_bytes == 2) { // For int16_t
            bits = *reinterpret_cast<const uint16_t*>(&value);
        } else if constexpr (num_bytes == 4) { // For int32_t float
            bits = *reinterpret_cast<const uint32_t*>(&value);
        } else if constexpr (num_bytes == 8) { // For int64_t, double
            bits = *reinterpret_cast<const uint64_t*>(&value);
        }

        for (size_t i = 0; i < num_bytes; ++i) {
            memory[address + i] = (bits >> (i * 8)) & 0xFF;
        }
    }

    template <typename DestT, typename SourceT>
    void execute_load_op() {
        int32_t address = pop<int32_t>();

        if (address + 4 > memory.size()) {
            throw std::runtime_error("Memory access out of bounds");
        }
        DestT value = load<DestT, SourceT>(address);
        push<DestT>(value);
    }

    template <typename DestT, typename SourceT>
    void execute_store_op() {
        SourceT value = pop<SourceT>();
        int32_t address = pop<int32_t>();

        if (address + 4 > memory.size()) {
            throw std::runtime_error("Memory access out of bounds");
        }

        store<DestT>(address, static_cast<DestT>(value));
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
};

#endif //INTERPRETER_H