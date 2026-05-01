#!/bin/bash
# Скрипт для чистой сборки проекта
# Полностью удаляет предыдущую сборку и собирает проект заново

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build-clean"

echo "=== Чистая сборка ==="

# Удаляем все предыдущие директории сборки
echo "Удаление предыдущих сборок..."
rm -rf "$SCRIPT_DIR/build-debug"
rm -rf "$SCRIPT_DIR/build-release"
rm -rf "$SCRIPT_DIR/build-asan"
rm -rf "$BUILD_DIR"

# Создаем новую директорию сборки
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Конфигурируем проект в режиме Release
echo "Запуск cmake в режиме Release..."
cmake -DCMAKE_BUILD_TYPE=Release \
      ..

# Собираем проект
echo "Компиляция проекта..."
cmake --build . --config Release --parallel $(nproc)

echo "=== Чистая сборка завершена ==="
echo "Исполняемый файл: $BUILD_DIR/markdown-editor"
