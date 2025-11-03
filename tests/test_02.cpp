#include "TestSuite.h"

const TestSuite test_02 = {
    "Test 02",
    std::string(WASM_TEST_DIR) + "/02_test_prio1.wasm",
    {
        // === FUNCTION CALL TESTS ===
        {"Call add(10, 5)", 0, expect_i32(0, 15)},
        {"Call composition", 1, expect_i32(0, 35)},
        {"Call square(7)", 2, expect_i32(0, 49)},
        {"Multiple calls", 3, expect_i32(0, 25)},
        {"Early return (true)", 4, expect_i32(0, 100)},
        {"Conditional return (false)", 5, expect_i32(0, 200)},
        {"Call abs (negative)", 6, expect_i32(0, 42)},
        {"Call abs (positive)", 7, expect_i32(0, 42)},
        {"Factorial of 5 (recursive)", 8, expect_i32(0, 120)},
        {"Fibonacci of 7 (recursive)", 9, expect_i32(0, 13)},

        // === FLOATING POINT TESTS (F32) ===
        {"F32 addition", 10, expect_f32(0, 6.0f)},
        {"F32 subtraction", 11, expect_f32(0, 7.0f)},
        {"F32 multiplication", 12, expect_f32(0, 10.0f)},
        {"F32 division", 13, expect_f32(0, 2.5f)},
        {"F32 min", 14, expect_f32(0, 2.1f)},
        {"F32 max", 15, expect_f32(0, 3.5f)},
        {"F32 abs", 16, expect_f32(0, 3.5f)},
        {"F32 neg", 17, expect_f32(0, -3.5f)},
        {"F32 sqrt", 18, expect_f32(0, 4.0f)},
        {"F32 ceil", 19, expect_f32(0, 4.0f)},
        {"F32 floor", 20, expect_f32(0, 3.0f)},
        {"F32 trunc", 21, expect_f32(0, 3.0f)},
        {"F32 nearest", 22, expect_f32(0, 4.0f)},
        {"F32 comparison eq", 23, expect_i32(0, 1)},
        {"F32 comparison ne", 24, expect_i32(0, 1)},
        {"F32 comparison lt", 25, expect_i32(0, 1)},
        {"F32 comparison gt", 26, expect_i32(0, 1)},
        {"F32 comparison le", 27, expect_i32(0, 1)},
        {"F32 comparison ge", 28, expect_i32(0, 1)},
        {"Call F32 function", 29, expect_f32(0, 4.0f)},

        // === FLOATING POINT TESTS (F64) ===
        {"F64 addition", 30, expect_f64_low32(0, 6.0)},
        {"F64 multiplication", 31, expect_f64_low32(0, 10.0)},
        {"F64 sqrt", 32, expect_f64_low32(0, 4.0)},
        {"F64 comparison gt", 33, expect_i32(0, 1)},

        // === TYPE CONVERSION TESTS ===
        {"i32 to f32 (signed)", 34, expect_f32(0, 42.0f)},
        {"i32 to f32 (unsigned)", 35, expect_f32(0, 42.0f)},
        {"f32 to i32 (trunc signed)", 36, expect_i32(0, 42)},
        {"f32 to i32 (trunc unsigned)", 37, expect_i32(0, 42)},
        {"i32 to f64 (signed)", 38, expect_f64_low32(0, 100.0)},
        {"f64 to i32 (trunc signed)", 39, expect_i32(0, 100)},
        {"f32 to f64 (promote)", 40, expect_f64_low32(0, 3.5)},
        {"f64 to f32 (demote)", 41, expect_f32(0, 3.5f)},
        {"Reinterpret f32 as i32", 42, expect_i32(0, 0x3F800000)},
        {"Reinterpret i32 as f32", 43, expect_i32(0, 0x40400000)},

        // === DROP AND NOP TESTS ===
        {"Drop value from stack", 44, expect_i32(0, 42)},
        {"Multiple drops", 45, expect_i32(0, 100)},
        {"Nop instructions", 46, expect_i32(0, 42)},
        {"Drop in computation", 47, expect_i32(0, 50)},

        // === MEMORY SIZE AND GROW TESTS (STATEFUL) ===
        {"Memory size (initial)", 48, expect_i32(0, 1)},
        {"Memory grow by 1 (returns old size)", 49, expect_i32(0, 1)},
        {"Memory size after grow", 50, expect_i32(0, 3)},
        {"Memory grow by 2 (returns old size)", 51, expect_i32(0, 3)},
        {"Write to newly grown memory", 52, expect_i32(0, 999)},

        // === COMBINED TESTS ===
        {"Combined function calls", 53, expect_i32(0, 11)},
        {"Combined float conversion", 54, expect_i32(0, 21)},
    }
};