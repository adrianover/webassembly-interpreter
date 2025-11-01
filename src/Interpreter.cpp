#include "Interpreter.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>

Interpreter::Interpreter(const Module& module) : module(module) {
    // Allocate Memory
    if (module.memory_initial_pages > 0) {
        memory.resize(module.memory_initial_pages * PAGE_SIZE);
    }

    // Initialize Globals
    for (const auto& global_def : module.globals) {
        globals.push_back({.i32 = 0});
    }
}

void Interpreter::invoke(uint32_t function_index) {
    if (function_index >= module.functions.size()) {
        throw std::runtime_error("Function index out of bounds");
    }

    std::cout << "Invoking function with index " << function_index << std::endl;

    const Function& func = module.functions.at(function_index);
    const FunctionType& type = module.types.at(func.type_index);

    // Create the first stack frame
    StackFrame frame;
    frame.func = &func;
    frame.pc = 0;
    frame.locals = std::vector<Value>(type.params.size() + func.locals.size());

    // Pop arguments from the stack and place them in the frame
    for (int i = type.params.size() - 1; i >= 0; --i) {
        frame.locals[i] = stack.back();
        stack.pop_back();
    }

    call_stack.push_back(frame);
    execute();
}

void Interpreter::execute_unary_op(const std::function<int32_t(int32_t)>& op) {
    int32_t a = pop_i32();
    push_i32(op(a));
}

void Interpreter::execute_binary_op(const std::function<int32_t(int32_t, int32_t)>& op) {
    int32_t b = pop_i32();
    int32_t a = pop_i32();
    push_i32(op(a, b));
}

void Interpreter::execute() {
    while (!call_stack.empty()) {
        StackFrame& frame = call_stack.back();
        const Function& func = *frame.func;
        size_t& pc = frame.pc;

        if (pc >= func.code.size()) {
            // Reached the end of the function naturally
            call_stack.pop_back();
            continue;
        }

        uint8_t opcode = func.code[pc++];

        switch (opcode) {
            case 0x10: { // call
                // 1. Read the function index to call
                uint32_t func_idx_to_call = decode_leb128_u(func.code, pc);

                // 2. Get the target function and its type
                const Function& target_func = module.functions.at(func_idx_to_call);
                const FunctionType& target_type = module.types.at(target_func.type_index);

                // 3. Create a new stack frame for the call
                StackFrame new_frame;
                new_frame.func = &target_func;
                new_frame.pc = 0;
                new_frame.locals.resize(target_type.params.size() + target_func.locals.size(), {.i32 = 0});

                // 4. Pop args from operand stack and set them as locals in the new frame
                for (int i = target_type.params.size() - 1; i >= 0; --i) {
                    new_frame.locals[i] = stack.back();
                    stack.pop_back();
                }

                // 5. Push the new frame onto the call stack. The loop will now execute it.
                call_stack.push_back(new_frame);
                break;
            }
            case 0x01: // noop
            case 0x02: // block
            case 0x03: // loop
                break;
            case 0x0A:
                break;
            case 0x20: { // local.get
                uint32_t index = decode_leb128_u(func.code, pc);
                const Value& value = frame.locals.at(index);
                stack.push_back(value);
                break;
            }
            case 0x36: { // i32.store
                int32_t value = pop_i32();
                int32_t address = pop_i32();
                if (address + 4 > memory.size()) {
                    throw std::runtime_error("Memory access out of bounds");
                }

                store_i32(address, value);
                break;
            }
            case 0x41: { // i32.const
                int32_t value = decode_leb128_s(func.code, pc);
                push_i32(value);
                break;
            }
            case 0x6A: execute_binary_op([](int32_t a, int32_t b) { return a + b; }); break; // i32.add
            case 0x6B: execute_binary_op([](int32_t a, int32_t b) { return a - b; }); break; // i32.sub
            case 0x6C: execute_binary_op([](int32_t a, int32_t b) { return a * b; }); break; // i32.mul
            case 0x71: execute_binary_op([](int32_t a, int32_t b) { return a & b; }); break; // i32.and
            case 0x72: execute_binary_op([](int32_t a, int32_t b) { return a | b; }); break; // i32.or
            case 0x73: execute_binary_op([](int32_t a, int32_t b) { return a ^ b; }); break; // i32.xor
            case 0x74: execute_binary_op([](int32_t a, int32_t b) { return a << b; }); break; // i32.shl
            case 0x75: execute_binary_op([](int32_t a, int32_t b) { return a >> b; }); break; // i32.shr_s
            case 0x76: execute_binary_op([](int32_t a, int32_t b) { return a >> b; }); break; // i32.shr_u // Todo

            // Binary Ops with edge cases
            case 0x6D: { // i32.div_s
                int32_t b = pop_i32();
                int32_t a = pop_i32();
                if (b == 0) throw std::runtime_error("integer divide by zero");
                if (a == INT32_MIN && b == -1) throw std::runtime_error("integer overflow");
                push_i32(a / b);
                break;
            }
            case 0x6E: { // i32.div_u
                uint32_t b = pop_i32();
                uint32_t a = pop_i32();
                if (b == 0) throw std::runtime_error("integer divide by zero");
                push_i32(a / b);
                break;
            }
            case 0x6F: { // i32.rem_s
                int32_t b = pop_i32();
                int32_t a = pop_i32();
                if (b == 0) throw std::runtime_error("integer divide by zero");
                push_i32(a % b);
                break;
            }

            case 0x00:
            case 0x0B: { // end
                if (pc == func.code.size()) { // End of a function
                    std::cout << "Execution of function finished." << std::endl;
                    return;
                }
                // End of a block
                break;
            }
            default:
                std::stringstream error_stream;
                error_stream << "Unknown or unimplemented opcode: 0x"
                         << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                         << (int)opcode;
                throw std::runtime_error(error_stream.str());
        }
    }
}


void Interpreter::push_i32(int32_t value) {
    stack.push_back({.i32 = value});
}

int32_t Interpreter::pop_i32() {
    if (stack.empty()) {
        throw std::runtime_error("Stack underflow");
    }
    Value val = stack.back();
    stack.pop_back();
    return val.i32;
}

int32_t Interpreter::get_memory_i32(uint32_t address) const {
    if (address + 4 > memory.size()) {
        throw std::runtime_error("Memory read out of bounds");
    }
    return (int32_t)(memory[address] | (memory[address+1] << 8) | (memory[address+2] << 16) | (memory[address+3] << 24));
}

void Interpreter::store_i32(int32_t address, int32_t value){
    memory[address]     = (value >> 0) & 0xFF;
    memory[address + 1] = (value >> 8) & 0xFF;
    memory[address + 2] = (value >> 16) & 0xFF;
    memory[address + 3] = (value >> 24) & 0xFF;
}

int32_t Interpreter::decode_leb128_s(const std::vector<uint8_t>& code, size_t &pc) {
    int32_t result = 0;
    int shift = 0;

    while (true) {
        uint8_t byte = code[pc++];
        result |= (byte & 0x7f) << shift;
        shift += 7;

        if ((byte & 0x80) == 0) {
            // Sign extension if the sign bit of the last byte is set
            if ((shift < 32) && (byte & 0x40)) {
                result |= (~0 << shift);
            }
            break;
        }
    }

    return result;
}


uint32_t Interpreter::decode_leb128_u(const std::vector<uint8_t> code, size_t& pc) {
    uint32_t result = 0;
    int shift = 0;
    while (true) {
        uint8_t byte = code[pc++];
        result |= (byte & 0x7f) << shift;
        if ((byte & 0x80) == 0) {
            break;
        }
        shift += 7;
    }
    return result;
}