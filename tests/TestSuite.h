#ifndef TESTSUITE_H
#define TESTSUITE_H

#include <string>
#include <vector>
#include <functional>
#include "../src/Interpreter.h"

using VerificationFn = std::function<bool(const Interpreter&)>;

struct Test {
    std::string name;
    uint32_t function_index_to_run;
    VerificationFn verify;
};

struct TestSuite {
    std::string name;
    std::string wasm_path;
    std::vector<Test> tests;
};

VerificationFn expect_i32(uint32_t address, int32_t expected_value) ;
VerificationFn expect_f32(uint32_t address, float expected_value);
VerificationFn expect_f64_low32(uint32_t address, double expected_value);

#endif //TESTSUITE_H