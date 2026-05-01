@echo off
REM Build script for x64 architecture
setlocal

echo Building for x64 architecture...

REM Set architecture
set ARCH=x64
set CMAKE_GENERATOR=Visual Studio 17 2022
set CMAKE_ARCH=x64

REM Create build directory
if not exist build\%ARCH% mkdir build\%ARCH%

REM Configure with CMake
cd build\%ARCH%
cmake -G "%CMAKE_GENERATOR%" -A %CMAKE_ARCH% ../..
if errorlevel 1 (
    echo CMake configuration failed!
    cd ../..
    exit /b 1
)

REM Build
cmake --build . --config Release
if errorlevel 1 (
    echo Build failed!
    cd ../..
    exit /b 1
)

cd ../..
echo x64 build completed successfully!

endlocal
