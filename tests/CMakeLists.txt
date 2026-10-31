add_executable(test_matrix
    test_matrix.cpp
)
target_link_libraries(test_matrix
    PRIVATE
        matforge_core
)
target_compile_features(test_matrix PRIVATE cxx_std_20)
add_test(NAME MatrixTests COMMAND test_matrix)

add_executable(test_interpreter
    test_interpreter.cpp
)
target_link_libraries(test_interpreter
    PRIVATE
        matforge_core
        matforge_engine
)
target_compile_features(test_interpreter PRIVATE cxx_std_20)
add_test(NAME InterpreterTests COMMAND test_interpreter)

