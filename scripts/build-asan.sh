#!/bin/bash
# Скрипт для сборки проекта с адресным санитайзером (ASan)
# Используется для обнаружения утечек памяти и ошибок работы с памятью

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build-asan"

echo "=== Сборка с AddressSanitizer ==="
echo "Директория сборки: $BUILD_DIR"

# Создаем директорию сборки
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Конфигурируем проект с включенным ASan
echo "Запуск cmake с AddressSanitizer..."
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
      -DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
      -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address" \
      ..

# Собираем проект
echo "Компиляция проекта..."
cmake --build . --config Debug --parallel $(nproc)

echo "=== Сборка с AddressSanitizer завершена ==="
echo "Исполняемый файл: $BUILD_DIR/markdown-editor"
echo "Запускайте с переменной окружения ASAN_OPTIONS=detect_leaks=1"
