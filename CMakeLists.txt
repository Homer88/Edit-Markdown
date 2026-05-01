cmake_minimum_required(VERSION 3.16)
project(markdown-editor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Найти Qt5 или Qt6
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Исходные файлы проекта
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    helpwindow.cpp
    helpwindow.h
    markdownparser.cpp
    markdownparser.h
    spellchecker.cpp
    spellchecker.h
    resources.qrc
)

# Hunspell библиотека
find_package(PkgConfig REQUIRED)
pkg_check_modules(HUNSPELL REQUIRED hunspell)

# Создаем исполняемый файл
add_executable(markdown-editor ${PROJECT_SOURCES})

# Подключаем заголовочные файлы hunspell
target_include_directories(markdown-editor PRIVATE ${HUNSPELL_INCLUDE_DIRS})

# Линкуем библиотеки
target_link_libraries(markdown-editor PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets
    ${HUNSPELL_LIBRARIES}
)

# Установка
install(TARGETS markdown-editor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
