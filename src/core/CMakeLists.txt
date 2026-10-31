add_library(matforge_core
    include/matforge/core/Matrix.hpp
    include/matforge/core/Value.hpp
    include/matforge/core/MathUtils.hpp
    src/Matrix.cpp
    src/Value.cpp
    src/MathUtils.cpp
)

target_include_directories(matforge_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(matforge_core PUBLIC cxx_std_20)

