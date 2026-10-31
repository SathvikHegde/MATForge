add_library(matforge_engine
    include/matforge/engine/Token.hpp
    include/matforge/engine/Lexer.hpp
    include/matforge/engine/Ast.hpp
    include/matforge/engine/Parser.hpp
    include/matforge/engine/Workspace.hpp
    include/matforge/engine/Interpreter.hpp
    src/Token.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/Workspace.cpp
    src/Interpreter.cpp
)

target_include_directories(matforge_engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(matforge_engine
    PUBLIC
        matforge_core
)

target_compile_features(matforge_engine PUBLIC cxx_std_20)

