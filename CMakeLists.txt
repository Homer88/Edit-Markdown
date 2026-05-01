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
    settings.cpp
    settings.h
    settings_dialog.cpp
    settings_dialog.h
    resources.qrc
)

# --- Сборка Hunspell из исходников ---
# Добавляем подпроект Hunspell (исходники лежат в hunspell/hunspell)
add_subdirectory(hunspell)

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

# Копируем файлы переводов (*.qm) в папку application/translations рядом с исполняемым файлом
file(GLOB TRANSLATION_FILES "${CMAKE_SOURCE_DIR}/translations/*.qm")
if(TRANSLATION_FILES)
    if(WIN32)
        add_custom_command(TARGET markdown-editor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:markdown-editor>/translations"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TRANSLATION_FILES} "$<TARGET_FILE_DIR:markdown-editor>/translations/"
            COMMENT "Copying all translation files to application/translations folder"
        )
    else()
        # Для Linux/macOS также копируем в папку translations
        add_custom_command(TARGET markdown-editor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:markdown-editor>/translations"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TRANSLATION_FILES} "$<TARGET_FILE_DIR:markdown-editor>/translations/"
            COMMENT "Copying all translation files to application/translations folder"
        )
    endif()
endif()
# --- Автоматическое копирование библиотек Qt (Windows) ---
if(WIN32)
    # Находим утилиту windeployqt
    get_target_property(_qmake_executable Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
    if(_qmake_executable)
        get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
    endif()

    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET markdown-editor POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE}
                    --qmldir "${CMAKE_SOURCE_DIR}"
                    --no-compiler-runtime
                    --no-opengl-sw
                    "$<TARGET_FILE_DIR:markdown-editor>/$<TARGET_FILE_NAME:markdown-editor>"
            COMMENT "Running windeployqt to copy Qt dependencies..."
        )
    else()
        message(WARNING "windeployqt not found! Qt libraries will not be copied automatically. Please ensure Qt is installed correctly.")
    endif()
endif()
