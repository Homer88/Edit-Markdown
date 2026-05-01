cmake_minimum_required(VERSION 3.16)
project(markdown-editor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Найти Qt5 или Qt6
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets PrintSupport)

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

# --- Сборка Hunspell из исходников ---
# Добавляем подпроект Hunspell перед основным проектом
add_subdirectory(hunspell)

# Создаем исполняемый файл
add_executable(markdown-editor ${PROJECT_SOURCES})

# Подключаем заголовочные файлы проекта
target_include_directories(markdown-editor PRIVATE 
    ${CMAKE_SOURCE_DIR}
)

# Линкуем библиотеки
target_link_libraries(markdown-editor PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
    hunspell  # Линкуем цель, собранную из исходников
)

# Установка
install(TARGETS markdown-editor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
