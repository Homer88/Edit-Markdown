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

# Создаем исполняемый файл и указываем выходную папку
add_executable(markdown-editor ${PROJECT_SOURCES})
set_target_properties(markdown-editor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/application"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/application"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/application"
)

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

# Копируем ВСЕ файлы словарей Hunspell (*.aff и *.dic) в папку сборки (для разработки и запуска)
file(GLOB DICTIONARY_FILES "${CMAKE_SOURCE_DIR}/hunspell/*.aff" "${CMAKE_SOURCE_DIR}/hunspell/*.dic")
if(DICTIONARY_FILES)
    file(COPY ${DICTIONARY_FILES} DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Для Windows: копируем ВСЕ словари в папку application/dictionary рядом с исполняемым файлом
if(WIN32)
    add_custom_command(TARGET markdown-editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:markdown-editor>/dictionary"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DICTIONARY_FILES} "$<TARGET_FILE_DIR:markdown-editor>/dictionary/"
        COMMENT "Copying all dictionary files to application/dictionary folder"
    )
else()
    # Для Linux/macOS: создаем папку dictionaries и копируем ВСЕ словари
    add_custom_command(TARGET markdown-editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:markdown-editor>/dictionaries"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DICTIONARY_FILES} "$<TARGET_FILE_DIR:markdown-editor>/dictionaries/"
        COMMENT "Copying all dictionary files to application/dictionaries folder"
    )
endif()
