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
# Добавляем подпроект Hunspell (исходники лежат в hunspell/hunspell)
add_subdirectory(hunspell/hunspell)

# Создаем исполняемый файл
add_executable(markdown-editor ${PROJECT_SOURCES})

# Подключаем заголовочные файлы проекта и Hunspell
target_include_directories(markdown-editor PRIVATE 
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/hunspell/hunspell
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

# Копируем файлы словарей Hunspell в папку сборки (для разработки и запуска)
file(COPY ${CMAKE_SOURCE_DIR}/hunspell/ru_RU.aff 
          ${CMAKE_SOURCE_DIR}/hunspell/ru_RU.dic
     DESTINATION ${CMAKE_BINARY_DIR})

# Для Windows: копируем словари в папку application/словари рядом с исполняемым файлом
if(WIN32)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/словари)
    file(COPY ${CMAKE_SOURCE_DIR}/hunspell/ru_RU.aff 
              ${CMAKE_SOURCE_DIR}/hunspell/ru_RU.dic
         DESTINATION ${CMAKE_BINARY_DIR}/$<CONFIG>/словари)
else()
    # Для Linux/macOS: создаем папку dictionaries и копируем туда словари
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/dictionaries)
    file(COPY ${CMAKE_SOURCE_DIR}/hunspell/ru_RU.aff 
              ${CMAKE_SOURCE_DIR}/hunspell/ru_RU.dic
         DESTINATION ${CMAKE_BINARY_DIR}/dictionaries)
endif()
