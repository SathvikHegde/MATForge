add_executable(matforge_cli
    main_cli.cpp
)

target_link_libraries(matforge_cli
    PRIVATE
        matforge_core
        matforge_engine
)

target_compile_features(matforge_cli PRIVATE cxx_std_20)

