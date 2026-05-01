@echo off
REM Build script for x86 architecture
setlocal

echo Building for x86 architecture...

REM Set architecture
set ARCH=x86
set CMAKE_GENERATOR=Visual Studio 17 2022
set CMAKE_ARCH=Win32

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
echo x86 build completed successfully!

endlocal
