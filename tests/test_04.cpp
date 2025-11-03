#include "TestSuite.h"

const TestSuite test_04 = {
    "Test04",
    std::string(WASM_TEST_DIR) + "/04_test_prio3.wasm",
    {
        // === UNSIGNED REMAINDER OPERATIONS ===
        {"i32.rem_u", 0, expect_i32(0, 2)},
        {"i32.rem_u (large)", 1, expect_i32(0, 1)},
        {"i64.rem_u", 2, expect_i32(0, 2)},
        {"i64.rem_u (large)", 3, expect_i32(0, 1)},

        // === UNSIGNED COMPARISON OPERATIONS (i32) ===
        {"i32.le_u (less)", 4, expect_i32(0, 1)},
        {"i32.le_u (equal)", 5, expect_i32(0, 1)},
        {"i32.le_u (large)", 6, expect_i32(0, 0)},
        {"i32.ge_u (greater)", 7, expect_i32(0, 1)},
        {"i32.ge_u (equal)", 8, expect_i32(0, 1)},
        {"i32.ge_u (large)", 9, expect_i32(0, 1)},

        // === UNSIGNED COMPARISON OPERATIONS (i64) ===
        {"i64.le_s (less)", 10, expect_i32(0, 1)},
        {"i64.le_s (negative)", 11, expect_i32(0, 1)},
        {"i64.le_u (less)", 12, expect_i32(0, 1)},
        {"i64.le_u (large)", 13, expect_i32(0, 0)},
        {"i64.ge_s (greater)", 14, expect_i32(0, 1)},
        {"i64.ge_s (negative)", 15, expect_i32(0, 1)},
        {"i64.ge_u (greater)", 16, expect_i32(0, 1)},
        {"i64.ge_u (large)", 17, expect_i32(0, 1)},

        // === F32 COPYSIGN ===
        {"f32.copysign (neg)", 18, expect_f32(0, -3.5f)},
        {"f32.copysign (pos)", 19, expect_f32(0, 3.5f)},
        {"f32.copysign (both pos)", 20, expect_f32(0, 3.5f)},

        // === F64 COPYSIGN ===
        {"f64.copysign (neg)", 21, expect_f64_low32(0, -3.5)},
        {"f64.copysign (pos)", 22, expect_f64_low32(0, 3.5)},

        // === F64 MISSING OPERATIONS ===
        {"f64.sub", 23, expect_f64_low32(0, 7.0)},
        {"f64.div", 24, expect_f64_low32(0, 2.5)},
        {"f64.min", 25, expect_f64_low32(0, 2.1)},
        {"f64.max", 26, expect_f64_low32(0, 3.5)},
        {"f64.abs", 27, expect_f64_low32(0, 3.5)},
        {"f64.neg", 28, expect_f64_low32(0, -3.5)},
        {"f64.ceil", 29, expect_f64_low32(0, 4.0)},
        {"f64.floor", 30, expect_f64_low32(0, 3.0)},
        {"f64.trunc", 31, expect_f64_low32(0, 3.0)},
        {"f64.nearest", 32, expect_f64_low32(0, 4.0)},

        // === F64 COMPARISONS ===
        {"f64.le", 33, expect_i32(0, 1)},
        {"f64.ge", 34, expect_i32(0, 1)},

        // === FLOAT MEMORY OPERATIONS ===
        {"f32.store and f32.load", 35, expect_f32(0, 3.14159f)},
        {"f32.store and f32.load (negative)", 36, expect_f32(0, -2.5f)},
        {"f64.store and f64.load", 37, expect_f64_low32(0, 2.718281828)},
        {"f64.store with large value", 38, expect_f64_low32(0, 123456.789)},
        {"f32 arithmetic with load", 39, expect_f32(0, 8.0f)},
        
        // === UNREACHABLE INSTRUCTION ===
        {"Unreachable (not reached by return)", 40, expect_i32(0, 42)},
        {"Unreachable (in false if branch)", 41, expect_i32(0, 100)},
        {"Unreachable (in else branch, not taken)", 42, expect_i32(0, 50)},
    }
};