#include "TestSuite.h"

const TestSuite test_08 = {
    "Test08",
    std::string(WASM_TEST_DIR) + "/08_test_post_mvp.wasm",
    {
        // === MULTIPLE RETURN VALUE TESTS ===
        {"Multi-return: Receive two values", 0, expect_i32(3000, 42)},
        {"Multi-return: Receive three values", 1, expect_i32(3000, 10)},
        {"Multi-return: Swap function", 2, expect_i32(3000, 20)},
        {"Multi-return: Divmod", 3, expect_i32(3000, 3)},
        {"Multi-return: Min/Max", 4, expect_i32(3000, 7)},
        {"Multi-return: Chaining calls", 5, expect_i32(3000, 100)},
        {"Multi-return: Discard one value", 6, expect_i32(3000, 42)},

        // === BULK MEMORY OPERATIONS ===
        {"Bulk Memory: memory.copy first byte", 7, expect_i32(3000, 65)}, // 'A'
        {"Bulk Memory: memory.copy third byte", 8, expect_i32(3000, 67)}, // 'C'
        {"Bulk Memory: memory.fill basic", 9, expect_i32(3000, 255)},
        {"Bulk Memory: memory.fill middle", 10, expect_i32(3000, 255)},
        {"Bulk Memory: memory.fill different value", 11, expect_i32(3000, 0x42)},
        {"Bulk Memory: memory.copy overlapping", 12, expect_i32(3000, 0x11)},
        {"Bulk Memory: copy entire string", 13, expect_i32(3000, 90)}, // 'Z'
        {"Bulk Memory: fill and verify range", 14, expect_i32(3000, 0xAB)},
        {"Bulk Memory: copy and modify", 15, expect_i32(3000, 67)}, // 'C'

        // === REFERENCE TYPES TESTS ===
        {"Reference Types: ref.is_null on funcref", 16, expect_i32(3000, 1)},
        {"Reference Types: ref.is_null on externref", 17, expect_i32(3000, 1)},
        {"Reference Types: ref.func is not null", 18, expect_i32(3000, 0)},
        {"Reference Types: store funcref in global", 19, expect_i32(3000, 0)},
        {"Reference Types: table.set/get funcref", 20, expect_i32(3000, 0)},
        {"Reference Types: table.get null slot", 21, expect_i32(3000, 1)},
        {"Reference Types: table.size initial", 22, expect_i32(3000, 8)},
        // Note: The following tests are stateful and depend on the previous ones.
        {"Reference Types: table.grow", 23, expect_i32(3000, 8)}, // Returns old size
        {"Reference Types: table.size after grow", 24, expect_i32(3000, 10)},
        {"Reference Types: table.fill with null", 25, expect_i32(3000, 1)},
        {"Reference Types: table.copy", 26, expect_i32(3000, 0)},
        {"Reference Types: externref global is null", 27, expect_i32(3000, 1)},
        {"Reference Types: store null externref", 28, expect_i32(3000, 1)},
        {"Reference Types: externref table size", 29, expect_i32(3000, 4)},

        // === COMBINED TESTS ===
        {"Combined: Multi-return and Bulk Memory", 30, expect_i32(3000, 65)},
        {"Combined: Table and Multi-return", 31, expect_i32(3000, 1)},
        {"Combined: Fill and Copy", 32, expect_i32(3000, 0x77)},
        {"Combined: Reference sizes sum", 33, expect_i32(3000, 14)}, // 10 + 4
        {"Combined: Swap with Bulk data", 34, expect_i32(3000, 66)}, // 'B'
        {"Combined: Bulk copy from pattern", 35, expect_i32(3000, 1)},
        {"Combined: Table info with multi-return", 36, expect_i32(3000, 10)},
    }
};