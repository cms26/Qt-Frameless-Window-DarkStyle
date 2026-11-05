# Qt Frameless Window with ModernStyle (light/dark) - Qt 6
#
# Build the demo:
#   cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=<your Qt kit>
#   cmake --build build
#
# Use it in your own CMake project:
#   add_subdirectory(path/to/Qt-Frameless-Window-DarkStyle)    # or FetchContent
#   target_link_libraries(MyApp PRIVATE FramelessWindow::FramelessWindow)
# and in code: #include "framelesswindow.h" / "modernstyle.h" (or "DarkStyle.h")

cmake_minimum_required(VERSION 3.16)
project(QtFramelessWindow VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build the demo only when this is the top-level project, not when it's
# used from another project through add_subdirectory()
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(_is_top_level ON)
else()
    set(_is_top_level OFF)
endif()
option(FRAMELESSWINDOW_BUILD_DEMO "Build the demo application" ${_is_top_level})

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

# --- The library: FramelessWindow + ModernStyle ---------------------------------
add_library(FramelessWindow STATIC
    framelesswindow/framelesswindow.h
    framelesswindow/framelesswindow.cpp
    framelesswindow/titlebar.h
    framelesswindow/titlebar.cpp
    modernstyle/modernstyle.h
    modernstyle/modernstyle.cpp
    modernstyle/DarkStyle.h
)
add_library(FramelessWindow::FramelessWindow ALIAS FramelessWindow)
set_target_properties(FramelessWindow PROPERTIES AUTOMOC ON)
target_include_directories(FramelessWindow PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/framelesswindow"
    "${CMAKE_CURRENT_SOURCE_DIR}/modernstyle"
)
target_link_libraries(FramelessWindow PUBLIC Qt6::Widgets)

# The style sheet, compiled in as the resource :/modernstyle/modernstyle.qss
qt_add_resources(FramelessWindow "modernstyle"
    PREFIX "/modernstyle"
    BASE "modernstyle"
    FILES modernstyle/modernstyle.qss
)

# --- The demo ---------------------------------------------------------------------
if(FRAMELESSWINDOW_BUILD_DEMO)
    qt_add_executable(QtFramelessWindowDemo WIN32 MACOSX_BUNDLE
        demo/main.cpp
        demo/mainwindow.h
        demo/mainwindow.cpp
        demo/mainwindow.ui
    )
    set_target_properties(QtFramelessWindowDemo PROPERTIES AUTOMOC ON AUTOUIC ON)
    target_link_libraries(QtFramelessWindowDemo PRIVATE FramelessWindow::FramelessWindow)
endif()
