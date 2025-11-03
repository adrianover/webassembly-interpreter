#include "TestSuite.h"

const TestSuite test_09 = {
    "Test 09",
    std::string(WASM_TEST_DIR) + "/09_print_hello.wasm",
    {
        {"WASI fd_write should update nwritten", 1, expect_i32(20, 14)},
    }
};