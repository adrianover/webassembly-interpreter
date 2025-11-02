#include "TestSuite.h"
#include <limits>

const TestSuite test_06 = {
    "Test06",
    "../../tests/wasm/06_test_fc.wasm",
    {
        // === i32.trunc_sat_f32_s TESTS ===
        {"i32.trunc_sat_f32_s: 10.5 -> 10", 0, expect_i32(0, 10)},
        {"i32.trunc_sat_f32_s: -5.8 -> -5", 1, expect_i32(0, -5)},
        {"i32.trunc_sat_f32_s: NaN -> 0", 2, expect_i32(0, 0)},
        {"i32.trunc_sat_f32_s: Overflow -> INT32_MAX", 3, expect_i32(0, std::numeric_limits<int32_t>::max())},
        {"i32.trunc_sat_f32_s: Underflow -> INT32_MIN", 4, expect_i32(0, std::numeric_limits<int32_t>::min())},

        // === i32.trunc_sat_f32_u TESTS ===
        {"i32.trunc_sat_f32_u: 42.7 -> 42", 5, expect_i32(0, 42)},
        {"i32.trunc_sat_f32_u: NaN -> 0", 6, expect_i32(0, 0)},
        {"i32.trunc_sat_f32_u: Negative -> 0", 7, expect_i32(0, 0)},
        {"i32.trunc_sat_f32_u: Overflow -> UINT32_MAX", 8, expect_i32(0, -1)}, // 0xFFFFFFFF

        // === i32.trunc_sat_f64_s TESTS ===
        {"i32.trunc_sat_f64_s: 123.456 -> 123", 9, expect_i32(0, 123)},
        {"i32.trunc_sat_f64_s: -99.99 -> -99", 10, expect_i32(0, -99)},
        {"i32.trunc_sat_f64_s: NaN -> 0", 11, expect_i32(0, 0)},
        {"i32.trunc_sat_f64_s: Overflow -> INT32_MAX", 12, expect_i32(0, std::numeric_limits<int32_t>::max())},
        {"i32.trunc_sat_f64_s: Underflow -> INT32_MIN", 13, expect_i32(0, std::numeric_limits<int32_t>::min())},

        // === i32.trunc_sat_f64_u TESTS ===
        {"i32.trunc_sat_f64_u: 255.9 -> 255", 14, expect_i32(0, 255)},
        {"i32.trunc_sat_f64_u: NaN -> 0", 15, expect_i32(0, 0)},
        {"i32.trunc_sat_f64_u: Negative -> 0", 16, expect_i32(0, 0)},
        {"i32.trunc_sat_f64_u: Overflow -> UINT32_MAX", 17, expect_i32(0, -1)}, // 0xFFFFFFFF

        // === i64.trunc_sat_f32_s TESTS ===
        {"i64.trunc_sat_f32_s: 42.5 -> 42", 18, expect_i32(0, 42)},
        {"i64.trunc_sat_f32_s: -7.3 -> -7", 19, expect_i32(0, -7)},
        {"i64.trunc_sat_f32_s: NaN -> 0", 20, expect_i32(0, 0)},

        // === i64.trunc_sat_f32_u TESTS ===
        {"i64.trunc_sat_f32_u: 100.9 -> 100", 21, expect_i32(0, 100)},
        {"i64.trunc_sat_f32_u: NaN -> 0", 22, expect_i32(0, 0)},
        {"i64.trunc_sat_f32_u: Negative -> 0", 23, expect_i32(0, 0)},

        // === i64.trunc_sat_f64_s TESTS ===
        {"i64.trunc_sat_f64_s: 1234.567 -> 1234", 24, expect_i32(0, 1234)},
        {"i64.trunc_sat_f64_s: -500.5 -> -500", 25, expect_i32(0, -500)},
        {"i64.trunc_sat_f64_s: NaN -> 0", 26, expect_i32(0, 0)},

        // === i64.trunc_sat_f64_u TESTS ===
        {"i64.trunc_sat_f64_u: 9999.1 -> 9999", 27, expect_i32(0, 9999)},
        {"i64.trunc_sat_f64_u: NaN -> 0", 28, expect_i32(0, 0)},
        {"i64.trunc_sat_f64_u: Negative -> 0", 29, expect_i32(0, 0)},

        // === EDGE CASE TESTS ===
        {"Edge Case: 0.0 f32 -> i32", 30, expect_i32(0, 0)},
        {"Edge Case: 0.1 f32 -> i32", 31, expect_i32(0, 0)},
        {"Edge Case: -0.0 f64 -> i32", 32, expect_i32(0, 0)},
        {"Edge Case: Large positive in-range f64 -> i32", 33, expect_i32(0, 1000000)},
    }
};