add_executable(run_tests
        test_main.cpp
        TestSuite.cpp
)

target_link_libraries(run_tests PRIVATE InterpreterLib)

add_test(
        NAME WasmTestSuite
        COMMAND run_tests
)
