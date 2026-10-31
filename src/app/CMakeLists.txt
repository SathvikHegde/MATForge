add_executable(matforge_app
    main.cpp
    resources/matforge.qrc
)

target_link_libraries(matforge_app
    PRIVATE
        matforge_core
        matforge_engine
        matforge_gui
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

target_compile_features(matforge_app PRIVATE cxx_std_20)

# Install rules
install(TARGETS matforge_app DESTINATION bin)

