@echo off

SETLOCAL ENABLEEXTENSIONS
SET "SCRIPT_DIR=%~dp0"
FOR %%I IN ("%SCRIPT_DIR%..") DO SET "REPO_ROOT=%%~fI"

IF DEFINED CDT_VCPKG_CACHE_DIR (
  SET "PINNED_VCPKG_ROOT=%CDT_VCPKG_CACHE_DIR%"
) ELSE (
  SET "PINNED_VCPKG_ROOT=%REPO_ROOT%\.cache\vcpkg"
)

IF DEFINED VCPKG_ROOT (
  SET "CDT_VCPKG_CACHE_DIR=%VCPKG_ROOT%"
  CALL "%SCRIPT_DIR%bootstrap-vcpkg.bat" --check >NUL 2>NUL
  IF NOT ERRORLEVEL 1 GOTO VCPKG_READY
  echo Ignoring VCPKG_ROOT=%VCPKG_ROOT%; using the repository-pinned checkout instead. 1>&2
)

SET "CDT_VCPKG_CACHE_DIR=%PINNED_VCPKG_ROOT%"
SET "VCPKG_ROOT=%PINNED_VCPKG_ROOT%"
CALL "%SCRIPT_DIR%bootstrap-vcpkg.bat" || EXIT /B 1

:VCPKG_READY
SET "VCPKG_ROOT=%CDT_VCPKG_CACHE_DIR%"
CD /D "%REPO_ROOT%" || EXIT /B 1
IF EXIST "%REPO_ROOT%\out\build\reference\CMakeCache.txt" DEL /F /Q "%REPO_ROOT%\out\build\reference\CMakeCache.txt" || EXIT /B 1
IF EXIST "%REPO_ROOT%\out\build\reference\CMakeFiles\." RMDIR /S /Q "%REPO_ROOT%\out\build\reference\CMakeFiles" || EXIT /B 1
cmake --preset reference -S . || EXIT /B 1
cmake --build --preset reference || EXIT /B 1
ctest --preset reference-smoke || EXIT /B 1
EXIT /B 0
