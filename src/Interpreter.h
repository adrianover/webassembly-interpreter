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
    void push_i32(int32_t value);
    int32_t pop_i32();
    void store_i32(int32_t address, int32_t value);
    int32_t decode_leb128_s(const std::vector<uint8_t>& code, size_t &pc);
    uint32_t decode_leb128_u(const std::vector<uint8_t> code, size_t& pc);

    void execute_unary_op(const std::function<int32_t(int32_t)>& op);
    void execute_binary_op(const std::function<int32_t(int32_t, int32_t)>& op);


    const Module& module;
    std::vector<Value> stack;
    std::vector<uint8_t> memory;
    std::vector<Value> globals;
    std::vector<StackFrame> call_stack;
};

#endif //INTERPRETER_H