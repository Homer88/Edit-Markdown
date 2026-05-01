#!/bin/bash
# Скрипт для релизной сборки проекта
# Оптимизированная сборка без символов отладки

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build-release"

echo "=== Релизная сборка ==="
echo "Директория сборки: $BUILD_DIR"

# Создаем директорию сборки
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Конфигурируем проект в режиме Release
echo "Запуск cmake в режиме Release..."
cmake -DCMAKE_BUILD_TYPE=Release \
      ..

# Собираем проект
echo "Компиляция проекта..."
cmake --build . --config Release --parallel $(nproc)

echo "=== Релизная сборка завершена ==="
echo "Исполняемый файл: $BUILD_DIR/markdown-editor"
