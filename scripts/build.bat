@echo off

SETLOCAL ENABLEEXTENSIONS
SET "SCRIPT_DIR=%~dp0"
FOR %%I IN ("%SCRIPT_DIR%..") DO SET "REPO_ROOT=%%~fI"

IF NOT DEFINED VCPKG_ROOT (
  echo VCPKG_ROOT must point to a bootstrapped vcpkg checkout. 1>&2
  EXIT /B 1
)

IF NOT EXIST "%VCPKG_ROOT%\vcpkg.exe" (
  echo VCPKG_ROOT does not contain vcpkg.exe: %VCPKG_ROOT% 1>&2
  EXIT /B 1
)

cmake --preset windows-msvc-release-developer-mode -S "%REPO_ROOT%" || EXIT /B 1
cmake --build "%REPO_ROOT%\out\build\windows-msvc-release-developer-mode" || EXIT /B 1
ctest --test-dir "%REPO_ROOT%\out\build\windows-msvc-release-developer-mode" --output-on-failure || EXIT /B 1
