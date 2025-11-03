add_executable(run_tests
        test_main.cpp
        TestSuite.cpp
)

target_link_libraries(run_tests PRIVATE InterpreterLib)

target_compile_definitions(run_tests PRIVATE WASM_TEST_DIR="${CMAKE_SOURCE_DIR}/tests/wasm")

add_test(
        NAME WasmTestSuite
        COMMAND run_tests
)
