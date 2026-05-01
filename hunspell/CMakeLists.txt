cmake_minimum_required(VERSION 3.16)
project(hunspell LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Опции сборки
option(BUILD_SHARED_LIBS "Build shared libraries" OFF) # Статическая библиотека по умолчанию

# Собираем исходники из папки src
# Примечание: убедитесь, что путь к исходникам верный. 
# В официальном репозитории исходники обычно лежат в src/
file(GLOB HUNSPELL_SOURCES "src/*.cpp")
file(GLOB HUNSPELL_HEADERS "src/*.h" "src/*.hxx")

# Если исходники лежат в корне (редко, но бывает в старых версиях), раскомментируйте строку ниже:
# file(GLOB HUNSPELL_SOURCES "*.cpp")

add_library(hunspell ${HUNSPELL_SOURCES} ${HUNSPELL_HEADERS})

# Добавляем пути к заголовкам
target_include_directories(hunspell PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Для Windows отключаем экспорт символов, если собираем статическую библиотеку
if(WIN32 AND NOT BUILD_SHARED_LIBS)
    target_compile_definitions(hunspell PRIVATE HUNSPELL_STATIC)
endif()

# Установка (опционально)
install(TARGETS hunspell
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(FILES ${HUNSPELL_HEADERS} DESTINATION include/hunspell)
