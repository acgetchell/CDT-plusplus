@echo off

SETLOCAL ENABLEEXTENSIONS
SET me=%~n0
SET parent=%~dp0

cd ..
IF EXIST "build" rmdir /S /Q build || EXIT /B 1
:: Assumes you have cloned vcpkg into %HOMEPATH%\Projects
set VCPKG_PATH=%HOMEPATH%\Projects\vcpkg

:: In case you want to build from the command line with Ninja from vcpkg
set PATH=%PATH%;%VCPKG_PATH%

:: Change to your version of Visual Studio
cmake -G "Visual Studio 16 2019" -A x64 -D CMAKE_BUILD_TYPE=Debug -D ENABLE_TESTING:BOOL=TRUE -D ENABLE_CACHE:BOOL=FALSE -D CMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%/scripts/buildsystems/vcpkg.cmake -S . -B build
cmake --build build

:: Executables are in \build\src\Debug
:: Tests are in \build\tests\Debug