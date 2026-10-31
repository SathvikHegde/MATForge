add_library(matforge_gui
    include/matforge/gui/MainWindow.hpp
    include/matforge/gui/CommandWindow.hpp
    include/matforge/gui/CodeEditor.hpp
    include/matforge/gui/MatlabHighlighter.hpp
    include/matforge/gui/WorkspaceWidget.hpp
    include/matforge/gui/FileBrowserWidget.hpp
    include/matforge/gui/CommandHistoryWidget.hpp
    include/matforge/gui/FigureWindow.hpp
    src/MainWindow.cpp
    src/CommandWindow.cpp
    src/CodeEditor.cpp
    src/MatlabHighlighter.cpp
    src/WorkspaceWidget.cpp
    src/FileBrowserWidget.cpp
    src/CommandHistoryWidget.cpp
    src/FigureWindow.cpp
)

target_include_directories(matforge_gui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(matforge_gui
    PUBLIC
        matforge_core
        matforge_engine
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

target_compile_features(matforge_gui PUBLIC cxx_std_20)

