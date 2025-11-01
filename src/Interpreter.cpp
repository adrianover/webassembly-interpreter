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
            // case 0x03: // loop
                break;
            // case 0x04: // if
            case 0x0A:
                break;
            case 0x1B: {
                const Value& a = stack.back();
                stack.pop_back();
                const Value& b = stack.back();
                stack.pop_back();
                uint32_t c = pop<int32_t>();

                if (c == 0){
                    stack.push_back(a);
                } else {
                    stack.push_back(b);
                }
                break;
            }
            case 0x20: { // local.get
                uint32_t index = decode_leb128_u(func.code, pc);
                const Value& value = frame.locals.at(index);
                stack.push_back(value);
                break;
            }
            case 0x21: { // local.set
                uint32_t index = decode_leb128_u(func.code, pc);
                const Value& value = stack.back();
                stack.pop_back();
                frame.locals.at(index) = value;
                break;
            }
            case 0x22: { // local.tee
                uint32_t index = decode_leb128_u(func.code, pc);
                const Value& value = stack.back();
                frame.locals.at(index) = value;
                break;
            }
            case 0x23: { // global.set
                uint32_t index = decode_leb128_u(func.code, pc);
                const Value& value = globals.at(index);
                stack.push_back(value);
                break;
            }
            case 0x24: { // global.get
                uint32_t index = decode_leb128_u(func.code, pc);
                globals.at(index) = stack.back();
                stack.pop_back();
                break;
            }

            // === LOAD ===
            case 0x28: execute_load_op<int32_t, int32_t>(); break;   // i32.load
            case 0x29: execute_load_op<int64_t, int64_t>(); break;   // i64.load
            case 0x2A: execute_load_op<float, float>(); break;       // f32.load
            case 0x2B: execute_load_op<double, double>(); break;      // f64.load
            case 0x2C: execute_load_op<int32_t, int8_t>(); break;    // i32.load8_s
            case 0x2D: execute_load_op<int32_t, uint8_t>(); break;   // i32.load8_u
            case 0x2E: execute_load_op<int32_t, int16_t>(); break;   // i32.load16_s
            case 0x2F: execute_load_op<int32_t, uint16_t>(); break;  // i32.load16_u

            case 0x30: execute_load_op<int64_t, int8_t>(); break;    // i64.load8_s
            case 0x31: execute_load_op<int64_t, uint8_t>(); break;   // i64.load8_u
            case 0x32: execute_load_op<int64_t, int16_t>(); break;   // i64.load16_s
            case 0x33: execute_load_op<int64_t, uint16_t>(); break;  // i64.load16_u
            case 0x34: execute_load_op<int64_t, int32_t>(); break;   // i64.load32_s
            case 0x35: execute_load_op<int64_t, uint32_t>(); break;  // i64.load32_u

            // === STORE ===
            case 0x36: execute_store_op<int32_t, int32_t>(); break;   // i32.store
            case 0x37: execute_store_op<int64_t, int64_t>(); break;   // i64.store
            case 0x38: execute_store_op<float, float>(); break;       // f32.store
            case 0x39: execute_store_op<double, double>(); break;      // f64.store
            case 0x3A: execute_store_op<int8_t, int32_t>(); break;    // i32.store8
            case 0x3B: execute_store_op<int16_t, int32_t>(); break;   // i32.store16
            case 0x3C: execute_store_op<int32_t, int64_t>(); break;   // i64.store32
            case 0x3D: execute_store_op<int8_t, int64_t>(); break;    // i64.store8
            case 0x3E: execute_store_op<int16_t, int64_t>(); break;   // i64.store16

            case 0x3F: { // memory.size
                push<int32_t>(memory.size() / PAGE_SIZE);
                break;
            }
            case 0x40: { // grow
                std::cout << func.code[pc] << std::endl;
                if (func.code[pc++] != 0x00) {
                    throw std::runtime_error("Invalid memory.grow instruction format");
                }
                int32_t delta_pages = pop<int32_t>();
                int32_t old_size_pages = memory.size() / PAGE_SIZE;

                // We assume that we always have enough memory
                size_t new_size_bytes = (old_size_pages + delta_pages) * PAGE_SIZE;
                memory.resize(new_size_bytes);
                break;
            }

            case 0x41: { // i32.const
                int32_t value = decode_leb128_s(func.code, pc);
                push<int32_t>(value);
                break;
            }

            case 0x43: { // f32.const
                int32_t value = decode_leb128_s(func.code, pc);
                push<float>(value);
                break;
            }

            case 0x45: execute_unary_op<int32_t>([](int32_t a) { return a == 0; }); break; // i32.eqz
            case 0x46: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a == b; }); break; // i32.eq
            case 0x47: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a != b; }); break; // i32.ne

            case 0x48: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a < b; }); break; // i32.lt_s
            case 0x49: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a < b; }); break; // i32.lt_u // Todo
            case 0x4A: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a > b; }); break; // i32.gt_s
            case 0x4B: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a > b; }); break; // i32.lt_u // Todo
            case 0x4C: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a <= b; }); break; // i32.le_s
            case 0x4D: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a <= b; }); break; // i32.le_u // Todo
            case 0x4E: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a >= b; }); break; // i32.ge_s
            case 0x4F: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a >= b; }); break; // i32.ge_u // Todo


            case 0x60: execute_binary_op<float>([](float a, float b) { return a >= b; }); break; // f32.ge
            case 0x67: execute_unary_op<int32_t>([](int32_t a) { return a ? __builtin_clz(a) : 32; }); break; // i32.clz
            case 0x68: execute_unary_op<int32_t>([](int32_t a) { return a ? __builtin_ctz(a) : 32; }); break; // i32.ctz
            case 0x69: execute_unary_op<int32_t>([](int32_t a) { return std::popcount(static_cast<uint32_t>(a)); }); break; // i32.popcnt
            case 0x77: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return std::rotl(static_cast<uint32_t>(a), b); }); break; // i32.rotl
            case 0x78: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return std::rotr(static_cast<uint32_t>(a), b); }); break; // i32.rotr

            case 0x6A: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a + b; }); break; // i32.add
            case 0x6B: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a - b; }); break; // i32.sub
            case 0x6C: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a * b; }); break; // i32.mul
            case 0x71: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a & b; }); break; // i32.and
            case 0x72: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a | b; }); break; // i32.or
            case 0x73: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a ^ b; }); break; // i32.xor
            case 0x74: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a << b; }); break; // i32.shl
            case 0x75: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a >> b; }); break; // i32.shr_s
            case 0x76: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a >> b; }); break; // i32.shr_u // Todo

            // Binary Ops with edge cases
            case 0x6D: { // i32.div_s
                int32_t b = pop<int32_t>();
                int32_t a = pop<int32_t>();
                if (b == 0) throw std::runtime_error("integer divide by zero");
                if (a == INT32_MIN && b == -1) throw std::runtime_error("integer overflow");
                push<int32_t>(a / b);
                break;
            }
            case 0x6E: { // i32.div_u
                uint32_t b = pop<int32_t>();
                uint32_t a = pop<int32_t>();
                if (b == 0) throw std::runtime_error("integer divide by zero");
                push<int32_t>(a / b);
                break;
            }
            case 0x6F: { // i32.rem_s
                int32_t b = pop<int32_t>();
                int32_t a = pop<int32_t>();
                if (b == 0) throw std::runtime_error("integer divide by zero");
                push<int32_t>(a % b);
                break;
            }
            case 0x7F: { // i64.div_s
                int64_t b = pop<int64_t>();
                int64_t a = pop<int64_t>();
                if (b == 0) throw std::runtime_error("integer divide by zero");
                if (a == INT64_MIN && b == -1) throw std::runtime_error("integer overflow");
                push<int64_t>(a / b);
                break;
            }
            case 0x80: { // i64.div_u
                int64_t b = pop<int64_t>();
                int64_t a = pop<int64_t>();
                if (b == 0) throw std::runtime_error("integer divide by zero");
                push<int64_t>(a / b);
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


int32_t Interpreter::get_memory_i32(uint32_t address) const {
    if (address + 4 > memory.size()) {
        throw std::runtime_error("Memory read out of bounds");
    }
    return (int32_t)(memory[address] | (memory[address+1] << 8) | (memory[address+2] << 16) | (memory[address+3] << 24));
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