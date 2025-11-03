#include "TestSuite.h"

const TestSuite test_05 = {
    "Test05",
    std::string(WASM_TEST_DIR) + "/05_test_complex.wasm",
    {
            {"Deeply nested blocks with br", 0, expect_i32(0, 42)},
            {"Blocks with different result types", 1, expect_i32(0, 50)},
            {"Conditional nested (param 0)", 2, expect_i32(0, 100)},
            {"Conditional nested (param 1)", 3, expect_i32(0, 200)},
            {"Conditional nested (param 2)", 4, expect_i32(0, 300)},
            {"Function call within nested blocks", 5, expect_i32(0, 42)},
            {"Loop with nested blocks", 6, expect_i32(0, 5)},
            {"Multiple function calls with label preservation", 7, expect_i32(0, 30)},
            {"br_table with nested blocks (case 0)", 8, expect_i32(0, 400)},
            {"br_table with nested blocks (case 1)", 9, expect_i32(0, 300)},
            {"br_table with nested blocks (case 2)", 10, expect_i32(0, 200)},
            {"br_table with nested blocks (default)", 11, expect_i32(0, 100)},
            {"Complex control flow with value stack", 12, expect_i32(0, 25)},
            {"Recursive calls with blocks (factorial)", 13, expect_i32(0, 120)},
            {"Empty and void blocks", 14, expect_i32(0, 42)},
            {"Loop with nested blocks and label cleanup", 15, expect_i32(0, 55)},
        }
};