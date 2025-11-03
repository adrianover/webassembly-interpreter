#include "TestSuite.h"

const TestSuite test_07 = {
    "Test07",
    std::string(WASM_TEST_DIR) + "/07_test_bulk_memory.wasm",
    {
        // === memory.fill TESTS ===
            {"memory.fill: Basic", 0, expect_i32(0, 42)},
            {"memory.fill: Range", 1, expect_i32(0, 99)},
            {"memory.fill: Single byte", 2, expect_i32(0, 77)},
            {"memory.fill: Zero", 3, expect_i32(0, 0)},

            // === memory.copy TESTS ===
            {"memory.copy: Basic", 4, expect_i32(0, 1819043144)},
            {"memory.copy: Single byte", 5, expect_i32(0, 65)},
            {"memory.copy: Block", 6, expect_i32(0, 170)},
            {"memory.copy: Overlapping", 7, expect_i32(0, 1)},

            // === memory.init TESTS ===
            {"memory.init: Basic", 8, expect_i32(0, 72)},
            {"memory.init: Partial", 9, expect_i32(0, 87)},
            {"memory.init: Second segment", 10, expect_i32(0, 3)},

            // === data.drop TESTS ===
            {"data.drop: After use", 11, expect_i32(0, 72)},

            // === COMBINED TESTS ===
            {"Combined: Fill then copy", 12, expect_i32(0, 55)},
            {"Combined: Init then copy", 13, expect_i32(0, 72)},
            {"Combined: Zero-length operations", 14, expect_i32(0, 123)},
        }
};