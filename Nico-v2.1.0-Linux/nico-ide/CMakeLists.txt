cmake_minimum_required(VERSION 3.10)
project(NicoIDE VERSION 2.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/CodeEditor.cpp
    src/OutputPanel.cpp
    src/NicoRunner.cpp
    src/NicoSyntaxHighlighter.cpp
    src/FindDialog.cpp
    src/AnsiRenderer.cpp
    src/ReplPanel.cpp
)

set(HEADERS
    src/MainWindow.h
    src/CodeEditor.h
    src/OutputPanel.h
    src/NicoRunner.h
    src/NicoSyntaxHighlighter.h
    src/FindDialog.h
    src/AnsiRenderer.h
    src/ReplPanel.h
)

# Incluir el ícono en el ejecutable de Windows
if(WIN32)
    set(WIN_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/nico-ide.rc)
endif()

add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
add_executable(nico-ide ${SOURCES} ${HEADERS} ${WIN_RESOURCES})
target_link_libraries(nico-ide Qt5::Core Qt5::Gui Qt5::Widgets)
