# CMake для сборки md4c как статической библиотеки
cmake_minimum_required(VERSION 3.16)
project(md4c_lib C)

# Настройки для C
set(CMAKE_C_STANDARD 99)

# Исходные файлы md4c
set(MD4C_SOURCES
    md4c.c
)

# Создаем статическую библиотеку
add_library(md4c STATIC ${MD4C_SOURCES})

# Публичные заголовки
target_include_directories(md4c PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Отключаем предупреждения для C библиотеки в C++ проекте (если нужно)
if(MSVC)
    target_compile_options(md4c PRIVATE /W4)
else()
    target_compile_options(md4c PRIVATE -Wall -Wextra)
endif()
