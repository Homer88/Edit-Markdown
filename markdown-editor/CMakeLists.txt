cmake_minimum_required(VERSION 3.16)
project(MarkdownEditor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)
find_package(PkgConfig REQUIRED)
pkg_check_modules(HUNSPELL REQUIRED hunspell)

qt_standard_project_setup()

qt_add_executable(MarkdownEditor
    main.cpp
    mainwindow.h
    mainwindow.cpp
    markdownparser.h
    markdownparser.cpp
    spellchecker.h
    spellchecker.cpp
    helpwindow.h
    helpwindow.cpp
)

target_link_libraries(MarkdownEditor PRIVATE
    Qt6::Widgets
    Qt6::Gui
    ${HUNSPELL_LIBRARIES}
)

target_include_directories(MarkdownEditor PRIVATE
    ${HUNSPELL_INCLUDE_DIRS}
)
