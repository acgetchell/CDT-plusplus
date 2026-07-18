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
cmake --preset windows-msvc-release-user-mode -S "%REPO_ROOT%" || EXIT /B 1
cmake --build "%REPO_ROOT%\out\build\windows-msvc-release-user-mode" || EXIT /B 1
EXIT /B 0
