#!/bin/bash
# Скрипт для отладочной сборки проекта
# Используется для разработки с включенными символами отладки

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build-debug"

echo "=== Отладочная сборка ==="
echo "Директория сборки: $BUILD_DIR"

# Создаем директорию сборки
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Конфигурируем проект в режиме Debug
echo "Запуск cmake в режиме Debug..."
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      ..

# Собираем проект
echo "Компиляция проекта..."
cmake --build . --config Debug --parallel $(nproc)

echo "=== Отладочная сборка завершена ==="
echo "Исполняемый файл: $BUILD_DIR/markdown-editor"
