add_executable(WasmTests
        test_main.cpp
        TestSuite.cpp
)

target_link_libraries(WasmTests PRIVATE InterpreterLib)

target_compile_definitions(WasmTests PRIVATE WASM_TEST_DIR="${CMAKE_SOURCE_DIR}/tests/wasm")

add_test(
        NAME WasmTestSuite
        COMMAND $<TARGET_FILE:WasmTests>
)
