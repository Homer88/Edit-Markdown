cmake_minimum_required(VERSION 3.10)
project(hunspell LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Опции сборки
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_BINRELOC "Enable binreloc" OFF)

# Сбор списка исходных файлов .cc и .hh вручную, так как в репозитории нет готового списка для CMake
# Ищем все файлы в текущей директории и поддиректориях (кроме тестов и примеров, если они есть отдельно)
file(GLOB HUNSPELL_SOURCES
    "*.cxx"
    "*.cc"
)

file(GLOB HUNSPELL_HEADERS
    "*.hxx"
    "*.hh"
    "*.h"
)

# Если файлы лежат в подпапках (например, src), раскомментируйте и поправьте пути ниже:
# file(GLOB_RECURSE HUNSPELL_SOURCES "src/*.cxx" "src/*.cc")
# file(GLOB_RECURSE HUNSPELL_HEADERS "src/*.hxx" "src/*.hh" "src/*.h")

# Создаем библиотеку
add_library(hunspell ${HUNSPELL_SOURCES} ${HUNSPELL_HEADERS})

# Добавляем текущую директорию в инклуды для самой библиотеки
target_include_directories(hunspell PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Определение макросов для экспорта/импорта (если нужно для Windows)
if(WIN32)
    target_compile_definitions(hunspell PRIVATE -DHUNSPELL_STATIC)
endif()

# Установка заголовков (опционально)
install(FILES ${HUNSPELL_HEADERS} DESTINATION include/hunspell)
install(TARGETS hunspell
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
