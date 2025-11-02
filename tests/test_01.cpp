#include "TestSuite.h"

const TestSuite test_01 = {
    "Test 01",
    "../../tests/wasm/01_test.wasm",
    {
        // === BASIC ARITHMETIC TESTS ===
        {"Store value 42", 0, expect_i32(0, 42)},
        {"Addition", 1, expect_i32(0, 15)},
        {"Subtraction", 2, expect_i32(0, 12)},
        {"Multiplication", 3, expect_i32(0, 42)},
        {"Division (signed)", 4, expect_i32(0, 5)},
        {"Division (unsigned)", 5, expect_i32(0, 6)},
        {"Remainder (signed)", 6, expect_i32(0, 2)},

        // === BITWISE TESTS ===
        {"Bitwise AND", 7, expect_i32(0, 10)},
        {"Bitwise OR", 8, expect_i32(0, 14)},
        {"Bitwise XOR", 9, expect_i32(0, 6)},
        {"Left shift", 10, expect_i32(0, 20)},
        {"Right shift (signed)", 11, expect_i32(0, -4)},
        {"Right shift (unsigned)", 12, expect_i32(0, 4)},

        // === MEMORY LOAD/STORE TESTS ===
        {"Store and Load", 13, expect_i32(0, 99)},
        {"Store byte and load unsigned", 14, expect_i32(0, 255)},
        {"Store byte and load signed", 15, expect_i32(0, -1)},

        // === LOCAL VARIABLE TESTS ===
        {"Locals arithmetic", 16, expect_i32(0, 35)},
        {"Local variable tee", 17, expect_i32(0, 15)},

        // === GLOBAL VARIABLE TESTS (STATEFUL) ===
        // Note: These tests depend on the order of execution.
        {"Global increment", 18, expect_i32(0, 1)}, // Global 'counter' becomes 1
        {"Read constant global", 19, expect_i32(0, 100)},
        // The previous test set counter to 1. This test adds 10, so the result is 11.
        {"Multiple global operations", 20, expect_i32(0, 11)}, // Global 'counter' becomes 11

        // === COMPLEX TEST ===
        {"Combined operations", 21, expect_i32(0, 142)},

        // === COMPARISON OPERATIONS ===
        {"Equal", 22, expect_i32(0, 1)},
        {"Not equal", 23, expect_i32(0, 1)},
        {"Less than signed", 24, expect_i32(0, 1)},
        {"Less than unsigned", 25, expect_i32(0, 1)},
        {"Greater than signed", 26, expect_i32(0, 1)},
        {"Greater than unsigned", 27, expect_i32(0, 1)},
        {"Less than or equal signed", 28, expect_i32(0, 1)},
        {"Greater than or equal signed", 29, expect_i32(0, 1)},
        {"Equal zero (true)", 30, expect_i32(0, 1)},
        {"Equal zero (false)", 31, expect_i32(0, 0)},

        // === UNARY OPERATIONS ===
        {"Count leading zeros", 32, expect_i32(0, 28)},
        {"Count trailing zeros", 33, expect_i32(0, 3)},
        {"Population count", 34, expect_i32(0, 3)},
        {"Population count all bits", 35, expect_i32(0, 32)},

        // === ROTATE OPERATIONS ===
        {"Rotate left", 36, expect_i32(0, 16)},
        {"Rotate right", 37, expect_i32(0, 1)},
        {"Rotate left with wrap", 38, expect_i32(0, 1)},

        // === 16-BIT MEMORY OPERATIONS ===
        {"Store and load 16-bit unsigned", 39, expect_i32(0, 65535)},
        {"Store and load 16-bit signed", 40, expect_i32(0, -1)},
        {"Store 16-bit 32768 and load unsigned", 41, expect_i32(0, 32768)},

        // === CONTROL FLOW - SELECT ===
        {"Select (true)", 42, expect_i32(0, 10)},
        {"Select (false)", 43, expect_i32(0, 20)},

        // === CONTROL FLOW - IF/ELSE ===
        {"If/else (true)", 44, expect_i32(0, 100)},
        {"If/else (false)", 45, expect_i32(0, 200)},
        {"If without else", 46, expect_i32(0, 50)},
        {"Nested if/else", 47, expect_i32(0, 1)},

        // === CONTROL FLOW - BLOCK AND BR ===
        {"Block with break (br_if)", 48, expect_i32(0, 10)},
        {"Block without break", 49, expect_i32(0, 20)},

        // === CONTROL FLOW - LOOP ===
        {"Loop to sum", 50, expect_i32(0, 15)},
        {"Loop with early break", 51, expect_i32(0, 15)},

        // === CONTROL FLOW - BR_TABLE ===
        {"Branch table (case 0)", 52, expect_i32(0, 100)},
        {"Branch table (case 2)", 53, expect_i32(0, 102)},
    }
};