@echo off

SETLOCAL ENABLEEXTENSIONS
SET me=%~n0
SET parent=%~dp0

cd ..
IF NOT EXIST "build" mkdir build || EXIT /B 1
cd build
:: Assumes you have cloned vcpkg into your home directory
set VCPKG_PATH=%HOMEPATH%\vcpkg
set CMAKE_PATH=%VCPKG_PATH%\downloads\tools\cmake-3.14.0-windows\cmake-3.14.0-win32-x86\bin

:: In case you want to build from the command line with Ninja from vcpkg
set NINJA_PATH=%VCPKG_PATH%\downloads\tools\ninja-1.8.2-windows
set PATH=%PATH%;%VCPKG_PATH%;%CMAKE_PATH%;%NINJA_PATH%
:: cmake --help || GOTO :EOF
:: vcpkg fetch ninja
:: ninja -h || GOTO :EOF

:: Change to your version of Visual Studio
cmake -G "Visual Studio 16 2019" -A x64 -D CMAKE_BUILD_TYPE=Release ENABLE_TESTING:BOOL=FALSE -D ENABLE_CCACHE:BOOL=FALSE -D CMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%/scripts/buildsystems/vcpkg.cmake ..
cmake --build .

:: Executables are in \build\Release