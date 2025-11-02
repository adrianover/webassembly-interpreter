#include "TestSuite.h"

const TestSuite test_03 = {
    "Test 03",
    "../../tests/wasm/03_test_prio2.wasm",
    {
        // === DATA SEGMENT TESTS ===
        {"Data: Read char 'H'", 6, expect_i32(200, 72)},
        {"Data: Read char 'e'", 7, expect_i32(200, 101)},
        {"Data: Read i32 42", 8, expect_i32(200, 42)},
        {"Data: Read i32 255", 9, expect_i32(200, 255)},
        {"Data: Read char 'T'", 10, expect_i32(200, 84)},
        {"Data: Read char '!'", 11, expect_i32(200, 33)},

        // === CALL_INDIRECT TESTS ===
        {"call_indirect: add", 12, expect_i32(200, 15)},
        {"call_indirect: sub", 13, expect_i32(200, 5)},
        {"call_indirect: mul", 14, expect_i32(200, 50)},
        {"call_indirect: div", 15, expect_i32(200, 5)},
        {"call_indirect: dynamic", 16, expect_i32(200, 50)},
        {"call_indirect: sequence", 17, expect_i32(200, 8)},

        // === I64 ARITHMETIC TESTS ===
        {"i64.add", 18, expect_i32(200, 15)},
        {"i64.sub", 19, expect_i32(200, 12)},
        {"i64.mul", 20, expect_i32(200, 42)},
        {"i64.div_s", 21, expect_i32(200, 5)},
        {"i64.div_u", 22, expect_i32(200, 6)},
        {"i64.rem_s", 23, expect_i32(200, 2)},
        {"i64.and", 24, expect_i32(200, 10)},
        {"i64.or", 25, expect_i32(200, 15)},
        {"i64.xor", 26, expect_i32(200, 15)},
        {"i64.shl", 27, expect_i32(200, 20)},
        {"i64.shr_s", 28, expect_i32(200, -4)},
        {"i64.shr_u", 29, expect_i32(200, 4)},
        {"i64.rotl", 30, expect_i32(200, 16)},
        {"i64.rotr", 31, expect_i32(200, 1)},
        {"i64.clz", 32, expect_i32(200, 60)},
        {"i64.ctz", 33, expect_i32(200, 3)},
        {"i64.popcnt", 34, expect_i32(200, 3)},

        // === I64 COMPARISON TESTS ===
        {"i64.eq", 35, expect_i32(200, 1)},
        {"i64.ne", 36, expect_i32(200, 1)},
        {"i64.lt_s", 37, expect_i32(200, 1)},
        {"i64.gt_s", 38, expect_i32(200, 1)},
        {"i64.eqz", 39, expect_i32(200, 1)},

        // === I64 CONVERSION TESTS ===
        {"i64.extend_i32_s", 40, expect_i32(200, -1)},
        {"i64.extend_i32_u", 41, expect_i32(200, 255)},
        {"i32.wrap_i64", 42, expect_i32(200, 255)},
        {"i64.trunc_f32_s", 43, expect_i32(200, 42)},
        {"i64.trunc_f64_s", 44, expect_i32(200, 100)},
        {"f32.convert_i64_s", 45, expect_f32(200, 42.0f)},
        {"f64.convert_i64_s", 46, expect_f64_low32(200, 100.0)},

        // === I64 MEMORY OPERATIONS ===
        {"i64.store/load", 47, expect_i32(200, 255)},
        {"i64.load32_u", 48, expect_i32(200, 240)},
        {"i64.load32_s", 49, expect_i32(200, -2147483648)},

        // === I64 FUNCTION CALL ===
        {"Call i64 function", 50, expect_i32(200, 42)},
        {"Large i64 multiplication", 51, expect_i32(200, 1000000000)},
        {"i64 bit pattern", 52, expect_i32(200, -1)},

        // === TRAP HANDLING TESTS ===
        {"Safe division", 53, expect_i32(200, 5)},
        {"Divisor is zero", 54, expect_i32(200, 0)},
        {"Check for division by zero", 55, expect_i32(200, -1)},
        {"Check memory in bounds", 56, expect_i32(200, 1)},
        {"Check memory out of bounds", 57, expect_i32(200, 0)},
        {"Check integer overflow", 58, expect_i32(200, 1)},
        {"Check modulo by zero", 59, expect_i32(200, -1)},
        {"Check i64 division by zero", 60, expect_i32(200, -1)},

        // === COMBINED TESTS ===
        {"Combined data and i64", 61, expect_i32(200, 84)},
        {"Combined indirect and i64", 62, expect_i32(200, 30)},
        {"Combined all features", 63, expect_i32(200, 114)},
    }
};