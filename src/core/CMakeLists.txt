add_library(matforge_core
    include/matforge/core/Matrix.hpp
    include/matforge/core/Value.hpp
    include/matforge/core/MathUtils.hpp
    include/matforge/core/SignalProcessing.hpp
    include/matforge/core/CsvIO.hpp
    include/matforge/core/RandomUtils.hpp
    src/Matrix.cpp
    src/Value.cpp
    src/MathUtils.cpp
    src/SignalProcessing.cpp
    src/CsvIO.cpp
    src/RandomUtils.cpp
)

target_include_directories(matforge_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(matforge_core
    PUBLIC
        Eigen3::Eigen
)

target_compile_features(matforge_core PUBLIC cxx_std_20)

