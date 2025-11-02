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
        globals.push_back(global_def.initial_value);

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
    frame.control_stack_base = 0;

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
            control_stack.resize(frame.control_stack_base); // We restore the size of the control stack
            call_stack.pop_back();
            continue;
        }

        uint8_t opcode = func.code[pc++];

        switch (opcode) {
            // === CONTROL FLOW ===
            case 0x00: // unreachable
            case 0x01: // noop
                break;
            case 0x02: // block
            case 0x03: { op_loop(func, pc, opcode); } break; // loop
            case 0x04: { op_if(func, pc, opcode); } break; // if
            // If we encounter an 'else' opcode, it's because the 'if' condition was TRUE and we can skip it.
            case 0x05: { pc = control_stack.back().end; } break; // else
            case 0x0C: { perform_branch(decode_leb128_u<uint32_t>(func.code, pc)); } break; // br
            case 0x0A:
                break;
            case 0x0B: { op_end(pc); } break; // end
            case 0x0D: { op_br_if(func, pc); } break; // br_if
            case 0x0F: { op_return(); } break; // return
            case 0x10: { op_function_call(func, pc); } break; // call
            case 0x1A: { stack.pop_back(); } break; // drop;
            case 0x1B: { op_select(); } break; // select

            // === GLOBALS ===
            case 0x20: { // local.get
                uint32_t index = decode_leb128_u<uint32_t>(func.code, pc);
                const Value& value = frame.locals.at(index);
                stack.push_back(value);
                break;
            }
            case 0x21: { // local.set
                uint32_t index = decode_leb128_u<uint32_t>(func.code, pc);
                const Value& value = stack.back();
                stack.pop_back();
                frame.locals.at(index) = value;
                break;
            }
            case 0x22: { // local.tee
                uint32_t index = decode_leb128_u<uint32_t>(func.code, pc);
                const Value& value = stack.back();
                frame.locals.at(index) = value;
                break;
            }
            case 0x23: { // global.set
                uint32_t index = decode_leb128_u<uint32_t>(func.code, pc);
                const Value& value = globals.at(index);
                stack.push_back(value);
                break;
            }
            case 0x24: { // global.get
                uint32_t index = decode_leb128_u<uint32_t>(func.code, pc);
                globals.at(index) = stack.back();
                stack.pop_back();
                break;
            }

            // === LOAD ===
            case 0x28: { int32_t a = pop<int32_t>(); push<int32_t>(load<int32_t>(a)); break; }   // i32.load
            case 0x29: { int32_t a = pop<int32_t>(); push<int64_t>(load<int64_t>(a)); break; }   // i64.load
            case 0x2A: { int32_t a = pop<int32_t>(); push<float>(load<float>(a)); break; }       // f32.load
            case 0x2B: { int32_t a = pop<int32_t>(); push<double>(load<double>(a)); break; }     // f64.load
            case 0x2C: { int32_t a = pop<int32_t>(); push<int32_t>(static_cast<int32_t>(load<int8_t>(a))); break; }    // i32.load8_s
            case 0x2D: { int32_t a = pop<int32_t>(); push<int32_t>(static_cast<int32_t>(load<uint8_t>(a))); break; }   // i32.load8_u
            case 0x2E: { int32_t a = pop<int32_t>(); push<int32_t>(static_cast<int32_t>(load<int16_t>(a))); break; }  // i32.load16_s
            case 0x2F: { int32_t a = pop<int32_t>(); push<int32_t>(static_cast<int32_t>(load<uint16_t>(a))); break; } // i32.load16_u
            case 0x30: { int32_t a = pop<int32_t>(); push<int64_t>(static_cast<int64_t>(load<int8_t>(a))); break; }    // i64.load8_s
            case 0x31: { int32_t a = pop<int32_t>(); push<int64_t>(static_cast<int64_t>(load<uint8_t>(a))); break; }   // i64.load8_u
            case 0x32: { int32_t a = pop<int32_t>(); push<int64_t>(static_cast<int64_t>(load<int16_t>(a))); break; }  // i64.load16_s
            case 0x33: { int32_t a = pop<int32_t>(); push<int64_t>(static_cast<int64_t>(load<uint16_t>(a))); break; } // i64.load16_u
            case 0x34: { int32_t a = pop<int32_t>(); push<int64_t>(static_cast<int64_t>(load<int32_t>(a))); break; }   // i64.load32_s
            case 0x35: { int32_t a = pop<int32_t>(); push<int64_t>(static_cast<int64_t>(load<uint32_t>(a))); break; }  // i64.load32_u

            // === STORE ===
            case 0x36: { int32_t v = pop<int32_t>(); int32_t a = pop<int32_t>(); store<int32_t>(a, v); break; }     // i32.store
            case 0x37: { int64_t v = pop<int64_t>(); int32_t a = pop<int32_t>(); store<int64_t>(a, v); break; }     // i64.store
            case 0x38: { float   v = pop<float>();   int32_t a = pop<int32_t>(); store<float>(a, v); break; }       // f32.store
            case 0x39: { double  v = pop<double>();  int32_t a = pop<int32_t>(); store<double>(a, v); break; }      // f64.store
            case 0x3A: { int32_t v = pop<int32_t>(); int32_t a = pop<int32_t>(); store<int8_t>(a, static_cast<int8_t>(v)); break; }    // i32.store8
            case 0x3B: { int32_t v = pop<int32_t>(); int32_t a = pop<int32_t>(); store<int16_t>(a, static_cast<int16_t>(v)); break; }   // i32.store16
            case 0x3C: { int64_t v = pop<int64_t>(); int32_t a = pop<int32_t>(); store<int32_t>(a, static_cast<int32_t>(v)); break; }   // i64.store32
            case 0x3D: { int64_t v = pop<int64_t>(); int32_t a = pop<int32_t>(); store<int8_t>(a, static_cast<int8_t>(v)); break; }    // i64.store8
            case 0x3E: { int64_t v = pop<int64_t>(); int32_t a = pop<int32_t>(); store<int16_t>(a, static_cast<int16_t>(v)); break; }   // i64.store16

            // === MEMORY ===
            case 0x3F: { op_mem_size(func, pc); } break; // memory.size
            case 0x40: { op_grow(func, pc); } break; // grow


            // === IMMEDIATES ===

            case 0x41: { // i32.const
                int32_t value = decode_leb128_s<int32_t>(func.code, pc);
                push<int32_t>(value);
                break;
            }

            case 0x42: { // i64.const
                int64_t value = decode_leb128_s<uint64_t>(func.code, pc);
                push<int64_t>(value);
                break;
            }

            case 0x43: { // f32.const
                float value = read_immediate<float>(func.code, pc);
                push<float>(value);
                break;
            }
            case 0x44: { // f64.const
                double value = read_immediate<double>(func.code, pc);
                push<double>(value);
                break;
            }

            // === COMPARISON ===
            case 0x45: push<int32_t>(pop<int32_t>() == 0); break; // i32.eqz
            case 0x46: execute_comparison_op<int32_t>([](int32_t a, int32_t b) { return a == b; }); break; // i32.eq
            case 0x47: execute_comparison_op<int32_t>([](int32_t a, int32_t b) { return a != b; }); break; // i32.ne
            case 0x48: execute_comparison_op<int32_t>([](int32_t a, int32_t b) { return a < b; }); break; // i32.lt_s
            case 0x49: execute_comparison_op<int32_t>([](int32_t a, int32_t b) { return static_cast<uint32_t>(a) < static_cast<uint32_t>(b); }); break; // i32.lt_u
            case 0x4A: execute_comparison_op<int32_t>([](int32_t a, int32_t b) { return a > b; }); break; // i32.gt_s
            case 0x4B: execute_comparison_op<int32_t>([](int32_t a, int32_t b) { return static_cast<uint32_t>(a) > static_cast<uint32_t>(b); }); break; // i32.gt_u
            case 0x4C: execute_comparison_op<int32_t>([](int32_t a, int32_t b) { return a <= b; }); break; // i32.le_s
            case 0x4D: execute_comparison_op<int32_t>([](int32_t a, int32_t b) { return static_cast<uint32_t>(a) <= static_cast<uint32_t>(b); }); break; // i32.le_u
            case 0x4E: execute_comparison_op<int32_t>([](int32_t a, int32_t b) { return a >= b; }); break; // i32.ge_s
            case 0x4F: execute_comparison_op<int32_t>([](int32_t a, int32_t b) { return static_cast<uint32_t>(a) >= static_cast<uint32_t>(b); }); break; // i32.ge_u

            case 0x50: push<int32_t>(pop<int64_t>() == 0); break; // i64.eqz
            case 0x51: execute_comparison_op<int64_t>([](int64_t a, int64_t b) { return a == b; }); break; // i64.eq
            case 0x52: execute_comparison_op<int64_t>([](int64_t a, int64_t b) { return a != b; }); break; // i64.ne
            case 0x53: execute_comparison_op<int64_t>([](int64_t a, int64_t b) { return a < b; }); break; // i64.lt_s
            case 0x54: execute_comparison_op<int64_t>([](int64_t a, int64_t b) { return static_cast<uint64_t>(a) < static_cast<uint64_t>(b); }); break; // i64.lt_u
            case 0x55: execute_comparison_op<int64_t>([](int64_t a, int64_t b) { return a > b; }); break; // i64.gt_s
            case 0x56: execute_comparison_op<int64_t>([](int64_t a, int64_t b) { return static_cast<uint64_t>(a) > static_cast<uint64_t>(b); }); break; // i64.gt_u
            case 0x57: execute_comparison_op<int64_t>([](int64_t a, int64_t b) { return a <= b; }); break; // i64.le_s
            case 0x58: execute_comparison_op<int64_t>([](int64_t a, int64_t b) { return static_cast<uint64_t>(a) <= static_cast<uint64_t>(b); }); break; // i64.le_u
            case 0x59: execute_comparison_op<int64_t>([](int64_t a, int64_t b) { return a >= b; }); break; // i64.ge_s
            case 0x5A: execute_comparison_op<int64_t>([](int64_t a, int64_t b) { return static_cast<uint64_t>(a) >= static_cast<uint64_t>(b); }); break; // i64.ge_u

            case 0x5B: execute_comparison_op<float>([](float a, float b) { return a == b; }); break; // f32.eq
            case 0x5C: execute_comparison_op<float>([](float a, float b) { return a != b; }); break; // f32.ne
            case 0x5D: execute_comparison_op<float>([](float a, float b) { return a < b; }); break;  // f32.lt
            case 0x5E: execute_comparison_op<float>([](float a, float b) { return a > b; }); break;  // f32.gt
            case 0x5F: execute_comparison_op<float>([](float a, float b) { return a <= b; }); break; // f32.le
            case 0x60: execute_comparison_op<float>([](float a, float b) { return a >= b; }); break; // f32.ge

            case 0x61: execute_comparison_op<double>([](double a, double b) { return a == b; }); break; // f64.eq
            case 0x62: execute_comparison_op<double>([](double a, double b) { return a != b; }); break; // f64.ne
            case 0x63: execute_comparison_op<double>([](double a, double b) { return a < b; }); break; // f64.lt
            case 0x64: execute_comparison_op<double>([](double a, double b) { return a > b; }); break; // f64.gt
            case 0x65: execute_comparison_op<double>([](double a, double b) { return a <= b; }); break; // f64.le
            case 0x66: execute_comparison_op<double>([](double a, double b) { return a >= b; }); break; // f64.ge

            // === ARITHMETIC ===
            case 0x67: execute_unary_op<int32_t>([](int32_t a) { return a ? __builtin_clz(a) : 32; }); break; // i32.clz
            case 0x68: execute_unary_op<int32_t>([](int32_t a) { return a ? __builtin_ctz(a) : 32; }); break; // i32.ctz
            case 0x69: execute_unary_op<int32_t>([](int32_t a) { return std::popcount(static_cast<uint32_t>(a)); }); break; // i32.popcnt
            case 0x6A: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a + b; }); break; // i32.add
            case 0x6B: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a - b; }); break; // i32.sub
            case 0x6C: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a * b; }); break; // i32.mul
            case 0x71: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a & b; }); break; // i32.and
            case 0x72: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a | b; }); break; // i32.or
            case 0x73: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a ^ b; }); break; // i32.xor
            case 0x74: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a << b; }); break; // i32.shl
            case 0x75: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a >> b; }); break; // i32.shr_s
            case 0x76: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return a >> b; }); break; // i32.shr_u // Todo
            case 0x77: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return std::rotl(static_cast<uint32_t>(a), b); }); break; // i32.rotl
            case 0x78: execute_binary_op<int32_t>([](int32_t a, int32_t b) { return std::rotr(static_cast<uint32_t>(a), b); }); break; // i32.rotr

            case 0x79: execute_unary_op<int64_t>([](int64_t a) { return a ? __builtin_clzll(a) : 64; }); break; // i64.clz
            case 0x7A: execute_unary_op<int64_t>([](int64_t a) { return a ? __builtin_ctzll(a) : 64; }); break; // i64.ctz
            case 0x7B: execute_unary_op<int64_t>([](int64_t a) { return std::popcount(static_cast<uint64_t>(a)); }); break; // i64.popcnt
            case 0x7C: execute_binary_op<int64_t>([](int64_t a, int64_t b) { return a + b; }); break; // i64.add
            case 0x7D: execute_binary_op<int64_t>([](int64_t a, int64_t b) { return a - b; }); break; // i64.sub
            case 0x7E: execute_binary_op<int64_t>([](int64_t a, int64_t b) { return a * b; }); break; // i64.mul

            case 0x83: execute_binary_op<int64_t>([](int64_t a, int64_t b) { return a & b; }); break; // i64.and
            case 0x84: execute_binary_op<int64_t>([](int64_t a, int64_t b) { return a | b; }); break; // i64.or
            case 0x85: execute_binary_op<int64_t>([](int64_t a, int64_t b) { return a ^ b; }); break; // i64.xor
            case 0x86: execute_binary_op<int64_t>([](int64_t a, int64_t b) { return a << b; }); break; // i64.shl
            case 0x87: execute_binary_op<int64_t>([](int64_t a, int64_t b) { return a >> b; }); break; // i64.shr_s
            case 0x88: execute_binary_op<int64_t>([](int64_t a, int64_t b) { return a >> b; }); break; // i64.shr_u
            case 0x89: execute_binary_op<int64_t>([](int64_t a, int64_t b) { return std::rotl(static_cast<uint64_t>(a), b); }); break; // i64.rotl
            case 0x8A: execute_binary_op<int64_t>([](int64_t a, int64_t b) { return std::rotr(static_cast<uint64_t>(a), b); }); break; // i64.rotr

            case 0x8B: execute_unary_op<float>([](float a) { return std::abs(a); }); break; // f32.abs
            case 0x8C: execute_unary_op<float>([](float a) { return -a; }); break; // f32.neg
            case 0x8D: execute_unary_op<float>([](float a) { return std::ceil(a); }); break; // f32.ceil
            case 0x8E: execute_unary_op<float>([](float a) { return std::floor(a); }); break; // f32.floor
            case 0x8F: execute_unary_op<float>([](float a) { return std::trunc(a); }); break; // f32.trunc
            case 0x90: execute_unary_op<float>([](float a) { return std::nearbyint(a); }); break; // f32.nearest
            case 0x91: execute_unary_op<float>([](float a) { return std::sqrt(a); }); break; // f32.sqrt
            case 0x92: execute_binary_op<float>([](float a, float b) { return a + b; }); break; // f32.add
            case 0x93: execute_binary_op<float>([](float a, float b) { return a - b; }); break; // f32.sub
            case 0x94: execute_binary_op<float>([](float a, float b) { return a * b; }); break; // f32.mul
            case 0x95: execute_binary_op<float>([](float a, float b) { return a / b; }); break; // f32.div // Todo
            case 0x96: execute_binary_op<float>([](float a, float b) { return std::min(a, b); }); break; // f32.min
            case 0x97: execute_binary_op<float>([](float a, float b) { return std::max(a, b); }); break; // f32.max
            case 0x98: execute_binary_op<float>([](float a, float b) { return std::copysign(a, b);}); break; // f32.copysign
            case 0x99: execute_unary_op<double>([](double a) { return std::abs(a); }); break; // f64.abs
            case 0x9A: execute_unary_op<double>([](double a) { return -a; }); break; // f64.neg
            case 0x9B: execute_unary_op<double>([](double a) { return std::ceil(a); }); break; // f64.ceil
            case 0x9C: execute_unary_op<double>([](double a) { return std::floor(a); }); break; // f64.floor
            case 0x9D: execute_unary_op<double>([](double a) { return std::trunc(a); }); break; // f64.trunc
            case 0x9E: execute_unary_op<double>([](double a) { return std::nearbyint(a); }); break; // f64.nearest
            case 0x9F: execute_unary_op<double>([](double a) { return std::sqrt(a); }); break; // f64.sqrt

            case 0xA0: execute_binary_op<double>([](double a, double b) { return a + b; }); break; // f64.add
            case 0xA1: execute_binary_op<double>([](double a, double b) { return a - b; }); break; // f64.sub
            case 0xA2: execute_binary_op<double>([](double a, double b) { return a * b; }); break; // f64.mul
            case 0xA3: execute_binary_op<double>([](double a, double b) { return a / b; }); break; // f64.div // Todo
            case 0xA4: execute_binary_op<double>([](double a, double b) { return std::min(a, b); }); break; // f64.min
            case 0xA5: execute_binary_op<double>([](double a, double b) { return std::max(a, b); }); break; // f64.max
            case 0xA6: execute_binary_op<double>([](double a, double b) { return std::copysign(a, b); }); break; // f64.copysign

            // === Conversion ===
            case 0xA7: execute_conversion_op<int32_t, int64_t>([](int64_t a) { return static_cast<int32_t>(a); }); break; // 32.wrap_i64
            case 0xA8: execute_conversion_op<int32_t, float>([](float a) { return static_cast<int32_t>(a); }); break; // i32.trunc_f32_s
            case 0xA9: execute_conversion_op<int32_t, float>([](float a) { return static_cast<uint32_t>(a); }); break; // i32.trunc_f32_u
            case 0xAA: execute_conversion_op<int32_t, double>([](double a) { return static_cast<int32_t>(a); }); break; // i32.trunc_f64_s
            case 0xAB: execute_conversion_op<int32_t, double>([](double a) { return static_cast<uint32_t>(a); }); break; // i32.trunc_f64_u
            case 0xAC: execute_conversion_op<int64_t, int32_t>([](int32_t a) { return static_cast<int64_t>(a); }); break; // i64.extend_i32_s
            case 0xAD: execute_conversion_op<int64_t, int32_t>([](int32_t a) { return static_cast<uint32_t>(a); }); break; // i64.extend_i32_u
            case 0xAE: execute_conversion_op<int64_t, float>([](float a) { return static_cast<int64_t>(a); }); break; // i64.trunc_f32_s
            case 0xAF: execute_conversion_op<int64_t, float>([](float a) { return static_cast<uint64_t>(a); }); break; // i64.trunc_f32_u
            case 0xB0: execute_conversion_op<int64_t, double>([](double a) { return static_cast<int64_t>(a); }); break; // i64.trunc_f64_s
            case 0xB1: execute_conversion_op<int64_t, double>([](double a) { return static_cast<uint64_t>(a); }); break; // i64.trunc_f64_u
            case 0xB2: execute_conversion_op<float, int32_t>([](int32_t a) { return static_cast<float>(a); }); break; // f32.convert_i32_s
            case 0xB3: execute_conversion_op<float, int32_t>([](int32_t a) { return static_cast<float>(static_cast<uint32_t>(a)); }); break; // f32.convert_i32_u
            case 0xB4: execute_conversion_op<double, int32_t>([](int32_t a) { return static_cast<double>(a); }); break; // f64.convert_i32_s
            case 0xB5: execute_conversion_op<double, int32_t>([](int32_t a) { return static_cast<double>(static_cast<uint32_t>(a)); }); break; // f64.convert_i32_u
            case 0xB6: execute_conversion_op<float, double>([](double a) { return static_cast<float>(a); }); break; // f32.demote_f64
            case 0xB7: execute_conversion_op<double, int32_t>([](int32_t a) { return static_cast<double>(a); }); break; // f64.convert_i32_s
            case 0xB8: execute_conversion_op<double, int32_t>([](int32_t a) { return static_cast<double>(static_cast<uint32_t>(a)); }); break; // f64.convert_i32_u
            case 0xB9: execute_conversion_op<double, int32_t>([](int64_t a) { return static_cast<double>(a); }); break; // f64.convert_i64_s
            case 0xBA: execute_conversion_op<double, int32_t>([](int64_t a) { return static_cast<double>(static_cast<uint64_t>(a)); }); break; // f64.convert_i64_u
            case 0xBB: execute_conversion_op<double, float>([](float a) { return static_cast<double>(a); }); break; // f64.promote_f32
            case 0xBC: execute_reinterpret_op<int32_t, float>(); break;   // i32.reinterpret_f32
            case 0xBD: execute_reinterpret_op<int64_t, double>(); break;  // i64.reinterpret_f64
            case 0xBE: execute_reinterpret_op<float, int32_t>(); break;   // f32.reinterpret_i32
            case 0xBF: execute_reinterpret_op<double, int64_t>(); break;  // f64.reinterpret_i64

            // === ARITHMETIC WITH EDGE CASES ===
            case 0x6D: { // i32.div_s
                int32_t b = pop<int32_t>();
                int32_t a = pop<int32_t>();
                if (b == 0) throw std::runtime_error("integer divide by zero");
                if (a == INT32_MIN && b == -1) throw std::runtime_error("integer overflow");
                push<int32_t>(a / b);
                break;
            }
            case 0x6E: { // i32.div_u
                uint32_t b = static_cast<uint32_t>(pop<int32_t>());
                uint32_t a = static_cast<uint32_t>(pop<int32_t>());
                if (b == 0) throw std::runtime_error("integer divide by zero");
                push<int32_t>(static_cast<int32_t>(a / b));
                break;
            }
            case 0x6F: { // i32.rem_s
                int32_t b = pop<int32_t>();
                int32_t a = pop<int32_t>();
                if (b == 0) throw std::runtime_error("integer divide by zero");
                push<int32_t>(a % b);
                break;
            }
            case 0x70: { // i32.rem_u
                uint32_t b = static_cast<uint32_t>(pop<int32_t>());
                uint32_t a = static_cast<uint32_t>(pop<int32_t>());
                if (b == 0) throw std::runtime_error("integer divide by zero");
                push<int32_t>(static_cast<int32_t>(a % b));
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
                uint64_t b = static_cast<uint64_t>(pop<int64_t>());
                uint64_t a = static_cast<uint64_t>(pop<int64_t>());
                if (b == 0) throw std::runtime_error("integer divide by zero");
                push<int64_t>(static_cast<int64_t>(a / b));
                break;
            }
            case 0x81: { // i64.rem_s
                int64_t b = pop<int64_t>();
                int64_t a = pop<int64_t>();
                if (b == 0) throw std::runtime_error("integer divide by zero");
                push<int64_t>(a % b);
                break;
            }
            case 0x82: { // i64.rem_u
                uint64_t b = static_cast<uint64_t>(pop<int64_t>());
                uint64_t a = static_cast<uint64_t>(pop<int64_t>());
                if (b == 0) throw std::runtime_error("integer divide by zero");
                push<int64_t>(static_cast<int64_t>(a % b));
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

std::pair<size_t, size_t> Interpreter::scan_block_body(size_t start_pc, const std::vector<uint8_t>& code) {
    int nesting_level = 1;
    size_t pc = start_pc;
    size_t else_pc = 0; // Will remain 0 if no 'else' is found

    while (pc < code.size() && nesting_level > 0) {
        uint8_t opcode = code[pc++];
        switch (opcode) {
            case 0x02: // block
            case 0x03: // loop
            case 0x04: // if
                pc = scan_block_body(pc, code).second;
            break;
            case 0x05: // else
                // Found an 'else' that belongs to our current block.
                if (nesting_level == 1) {
                    else_pc = pc;
                }
            break;
            case 0x0B: // end
                nesting_level--;
            break;

        }
    }
    return {else_pc, pc};
}

void Interpreter::op_function_call(const Function &func, size_t &pc) {
    uint32_t func_idx_to_call = decode_leb128_u<uint32_t>(func.code, pc);

    const Function& target_func = module.functions.at(func_idx_to_call);
    const FunctionType& target_type = module.types.at(target_func.type_index);

    StackFrame new_frame;
    new_frame.func = &target_func;
    new_frame.pc = 0;
    new_frame.locals.resize(target_type.params.size() + target_func.locals.size(), {.i32 = 0});
    new_frame.control_stack_base = control_stack.size();

    for (int i = target_type.params.size() - 1; i >= 0; --i) {
        new_frame.locals[i] = stack.back();
        stack.pop_back();
    }

    call_stack.push_back(new_frame);
}

void Interpreter::op_select() {
    uint32_t c = pop<int32_t>();

    const Value& b = stack.back();
    stack.pop_back();

    const Value& a = stack.back();
    stack.pop_back();

    if (c == 0){
        stack.push_back(b);
    } else {
        stack.push_back(a);
    }
}

void Interpreter::op_loop(const Function &func, size_t &pc, uint8_t opcode) {
    [[maybe_unused]] uint8_t blocktype = func.code[pc++];

    auto [_, end_target_pc] = scan_block_body(pc, func.code);

    ControlFrame frame;
    frame.opcode = opcode;
    frame.end = end_target_pc;

    frame.start = (opcode == 0x03) ? pc : 0;

    control_stack.push_back(frame);
}

void Interpreter::op_if(const Function &func, size_t &pc, uint8_t opcode) {
    [[maybe_unused]] uint8_t blocktype = func.code[pc++];

    // Scan ahead to find the 'else' and 'end' locations.
    auto [else_target, end_target_pc] = scan_block_body(pc, func.code);

    if (pop<int32_t>() == 0) {
        // Jump to 'else' or 'end'
        pc = (else_target != 0) ? else_target : end_target_pc;
        return; // jump
    }

    ControlFrame frame;
    frame.opcode = opcode;
    frame.end = end_target_pc;
    frame.start = 0;

    control_stack.push_back(frame);
}

void Interpreter::op_end(size_t &pc) {
    StackFrame& current_frame = call_stack.back();
    if (control_stack.size() > current_frame.control_stack_base) {
        control_stack.pop_back();
    }

    StackFrame& frame = call_stack.back();
    const Function& func = *frame.func;
    // Check if this 'end' corresponds to the end of the function body
    if (pc >= func.code.size()) {
        call_stack.pop_back();
    }
}

void Interpreter::op_grow(const Function &func, size_t &pc) {
    if (func.code[pc++] != 0x00) {
        throw std::runtime_error("Invalid memory.grow instruction format");
    }
    int32_t delta_pages = pop<int32_t>();
    int32_t old_size_pages = memory.size() / PAGE_SIZE;

    // We assume that we always have enough memory
    size_t new_size_bytes = (old_size_pages + delta_pages) * PAGE_SIZE;
    memory.resize(new_size_bytes);
    push<int32_t>(old_size_pages);
}

void Interpreter::op_br_if(const Function &func, size_t &pc) {
    uint32_t label_index = decode_leb128_u<uint32_t>(func.code, pc);
    if (pop<int32_t>() != 0) {
        perform_branch(label_index);
    }
}

void Interpreter::op_return() {
    const StackFrame& exiting_frame = call_stack.back();
    control_stack.resize(exiting_frame.control_stack_base);
    call_stack.pop_back();
}

void Interpreter::op_mem_size(const Function &func, size_t &pc) {
    if (func.code[pc++] != 0x00) {
        throw std::runtime_error("Invalid memory.size instruction format");
    }
    push<int32_t>(memory.size() / PAGE_SIZE);
}

void Interpreter::perform_branch(uint32_t label_index) {
    if (label_index >= control_stack.size()) {
        throw std::runtime_error("perform_branch: Invalid branch label index");
    }

    const ControlFrame& target = control_stack[control_stack.size() - 1 - label_index];
    StackFrame& current_frame = call_stack.back();

    for (uint32_t i = 0; i < label_index; ++i) {
        if (!control_stack.empty()) {
            control_stack.pop_back();
        }
    }

    if (target.opcode == 0x03) { // Loop
        current_frame.pc = target.start;
    } else {
        current_frame.pc = target.end;
    }
}


int32_t Interpreter::get_memory_i32(uint32_t address) const {
    if (address + 4 > memory.size()) {
        throw std::runtime_error("Memory read out of bounds");
    }
    return (int32_t)(memory[address] | (memory[address+1] << 8) | (memory[address+2] << 16) | (memory[address+3] << 24));
}

float Interpreter::get_memory_f32(uint32_t address) const {
    int32_t i32_val = get_memory_i32(address);
    float f_val;
    std::memcpy(&f_val, &i32_val, sizeof(f_val));
    return f_val;
}