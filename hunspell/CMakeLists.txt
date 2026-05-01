cmake_minimum_required(VERSION 3.10)
project(hunspell LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Опции сборки
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_BINRELOC "Enable binreloc" OFF)

# Сбор списка исходных файлов .cc и .cxx вручную
file(GLOB HUNSPELL_SOURCES
    "*.cxx"
    "*.cc"
)

file(GLOB HUNSPELL_HEADERS
    "*.hxx"
    "*.hh"
    "*.h"
)

# Создаем библиотеку
add_library(hunspell ${HUNSPELL_SOURCES} ${HUNSPELL_HEADERS})

# Добавляем текущую директорию в инклуды для самой библиотеки
target_include_directories(hunspell PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Определение макросов для экспорта/импорта
if(WIN32)
    # При статической сборке обязательно определяем HUNSPELL_STATIC
    target_compile_definitions(hunspell PUBLIC HUNSPELL_STATIC)
    # Отключаем спецификаторы dllexport/dllimport для статической библиотеки
    target_compile_definitions(hunspell PRIVATE -DHUNSPELL_STATIC)
    # При статической сборке обязательно определяем HUNSPELL_STATIC
       # Это должно отключить __declspec(dllimport/dllexport) в заголовках Hunspell
       #target_compile_definitions(hunspell PUBLIC HUNSPELL_STATIC)

       # Дополнительно убеждаемся, что макрос определен и приватно для компиляции самой библиотеки
       #target_compile_definitions(hunspell PRIVATE HUNSPELL_STATIC)

       # Отключаем спецификаторы dllexport/dllimport для статической библиотеки
       # Явно определяем, что это статическая сборка
       target_compile_definitions(hunspell PRIVATE -DHUNSPELL_STATIC)

       # Макрос BUILDING_LIBHUNSPELL нужен для корректной компиляции библиотеки
       # Без него hunvisapi.h будет использовать __declspec(dllimport) даже при сборке самой библиотеки
       target_compile_definitions(hunspell PRIVATE BUILDING_LIBHUNSPELL)
endif()

# Установка заголовков (опционально)
install(FILES ${HUNSPELL_HEADERS} DESTINATION include/hunspell)
install(TARGETS hunspell
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
