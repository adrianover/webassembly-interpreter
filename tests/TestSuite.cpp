#include "TestSuite.h"
#include <iostream>
#include <cstring>

VerificationFn expect_i32(uint32_t address, int32_t expected_value) {
    return [address, expected_value](const Interpreter& interpreter) {
        int32_t actual = interpreter.get_memory_i32(address);
        std::cout << "Verifying i32. Result: " << actual << ", Expected: " << expected_value << std::endl;
        return actual == expected_value;
    };
}

VerificationFn expect_f32(uint32_t address, float expected_value) {
    return [address, expected_value](const Interpreter& interpreter) {
        float actual = interpreter.get_memory_f32(address);
        std::cout << "Verifying f32. Result: " << actual << ", Expected: " << expected_value << std::endl;
        return actual == expected_value;
    };
}

VerificationFn expect_f64_low32(uint32_t address, double expected_value) {
    return [address, expected_value](const Interpreter& interpreter) {
        uint64_t expected_bits;
        std::memcpy(&expected_bits, &expected_value, sizeof(double));

        auto expected_low32 = static_cast<int32_t>(expected_bits);

        int32_t actual_low32 = interpreter.get_memory_i32(address);

        std::cout << "  Verifying f64_low32. Result: " << actual_low32 << ", Expected: " << expected_low32 << std::endl;
        return actual_low32 == expected_low32;
    };
}

