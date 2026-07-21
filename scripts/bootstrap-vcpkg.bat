@echo off

SETLOCAL ENABLEEXTENSIONS DISABLEDELAYEDEXPANSION
SET "SCRIPT_DIR=%~dp0"
FOR %%I IN ("%SCRIPT_DIR%..") DO SET "REPO_ROOT=%%~fI"
SET "CDT_VCPKG_MANIFEST=%REPO_ROOT%\vcpkg.json"
SET "MODE=%~1"
REM GitHub release-asset SHA-256 digests for vcpkg-tool 2026-07-13.
SET "TRUSTED_VCPKG_TOOL_TAG=2026-07-13"
SET "TRUSTED_VCPKG_TOOL_SHA256_X64=67958c6a13a35130ff8035bef33097ffe3376a6708577a826cfa41fa592db611"
SET "TRUSTED_VCPKG_TOOL_SHA256_ARM64=8d87ed438db65b0015f624693612cb79d8c35908348c194984c23b63a2da0211"

IF NOT DEFINED MODE SET "MODE=bootstrap"
IF /I NOT "%MODE%"=="bootstrap" IF /I NOT "%MODE%"=="--check" (
  echo Usage: %~nx0 [--check] 1>&2
  EXIT /B 2
)

WHERE git.exe >NUL 2>NUL
IF ERRORLEVEL 1 (
  echo git.exe is required to bootstrap vcpkg. 1>&2
  EXIT /B 1
)

WHERE powershell.exe >NUL 2>NUL
IF ERRORLEVEL 1 (
  echo powershell.exe is required to read the vcpkg manifest baseline. 1>&2
  EXIT /B 1
)

IF NOT EXIST "%SystemRoot%\System32\certutil.exe" (
  echo certutil.exe is required to verify the vcpkg executable. 1>&2
  EXIT /B 1
)

SET "BASELINE="
FOR /F "usebackq delims=" %%I IN (`powershell.exe -NoLogo -NoProfile -NonInteractive -Command "$json = Get-Content -Raw -LiteralPath $env:CDT_VCPKG_MANIFEST; $baseline = (ConvertFrom-Json -InputObject $json).'builtin-baseline'; if ($baseline -notmatch '^[0-9a-f]{40}$') { exit 1 }; $baseline"`) DO SET "BASELINE=%%I"
IF NOT DEFINED BASELINE (
  echo Unable to read a 40-character builtin-baseline from %CDT_VCPKG_MANIFEST%. 1>&2
  EXIT /B 1
)

IF DEFINED CDT_VCPKG_CACHE_DIR (
  SET "VCPKG_DIR=%CDT_VCPKG_CACHE_DIR%"
) ELSE (
  SET "VCPKG_DIR=%REPO_ROOT%\.cache\vcpkg"
)

IF EXIST "%VCPKG_DIR%\.git\." (
  git -C "%VCPKG_DIR%" remote get-url origin 2>NUL ^| FINDSTR /X /C:"https://github.com/microsoft/vcpkg.git" >NUL
  IF ERRORLEVEL 1 (
    echo Refusing to reuse %VCPKG_DIR% because its origin is not microsoft/vcpkg. 1>&2
    EXIT /B 1
  )

  git -C "%VCPKG_DIR%" status --short 2>NUL ^| FINDSTR . >NUL
  IF NOT ERRORLEVEL 1 (
    echo Refusing to reuse a modified vcpkg checkout at %VCPKG_DIR%. 1>&2
    EXIT /B 1
  )

  git -C "%VCPKG_DIR%" rev-parse HEAD 2>NUL ^| FINDSTR /X /I /C:"%BASELINE%" >NUL
  IF NOT ERRORLEVEL 1 IF EXIST "%VCPKG_DIR%\vcpkg.exe" (
    CALL :VALIDATE_CACHED_VCPKG "%VCPKG_DIR%"
    IF NOT ERRORLEVEL 1 (
      echo Using vcpkg at %VCPKG_DIR% ^(%BASELINE%^)
      EXIT /B 0
    )
  )
) ELSE IF EXIST "%VCPKG_DIR%\." (
  DIR /B /A "%VCPKG_DIR%" 2>NUL ^| FINDSTR . >NUL
  IF NOT ERRORLEVEL 1 (
    echo Refusing to initialize vcpkg in non-empty directory %VCPKG_DIR%. 1>&2
    EXIT /B 1
  )
)

IF /I "%MODE%"=="--check" (
  echo VCPKG_ROOT is not a clean official checkout at baseline %BASELINE%: %VCPKG_DIR% 1>&2
  EXIT /B 1
)

IF NOT EXIST "%VCPKG_DIR%\.git\." (
  IF NOT EXIST "%VCPKG_DIR%\." MD "%VCPKG_DIR%"
  IF ERRORLEVEL 1 EXIT /B 1
  git -C "%VCPKG_DIR%" init
  IF ERRORLEVEL 1 EXIT /B 1
  git -C "%VCPKG_DIR%" remote add origin https://github.com/microsoft/vcpkg.git
  IF ERRORLEVEL 1 EXIT /B 1
)

git -C "%VCPKG_DIR%" fetch --depth 1 origin "%BASELINE%"
IF ERRORLEVEL 1 EXIT /B 1
git -C "%VCPKG_DIR%" checkout --detach "%BASELINE%"
IF ERRORLEVEL 1 EXIT /B 1
CALL "%VCPKG_DIR%\bootstrap-vcpkg.bat" -disableMetrics
IF ERRORLEVEL 1 EXIT /B 1
CALL :VALIDATE_CACHED_VCPKG "%VCPKG_DIR%"
IF ERRORLEVEL 1 (
  echo Bootstrapped vcpkg does not match the tool release pinned by %BASELINE%. 1>&2
  EXIT /B 1
)

echo Bootstrapped vcpkg at %VCPKG_DIR% ^(%BASELINE%^)
EXIT /B 0

:VALIDATE_CACHED_VCPKG
SETLOCAL
SET "VALIDATE_DIR=%~1"
SET "EXPECTED_TOOL_TAG="
IF NOT EXIST "%VALIDATE_DIR%\scripts\vcpkg-tool-metadata.txt" (
  ENDLOCAL
  EXIT /B 1
)
FOR /F "usebackq tokens=1,* delims==" %%A IN ("%VALIDATE_DIR%\scripts\vcpkg-tool-metadata.txt") DO (
  IF /I "%%A"=="VCPKG_TOOL_RELEASE_TAG" SET "EXPECTED_TOOL_TAG=%%B"
)
IF NOT DEFINED EXPECTED_TOOL_TAG (
  ENDLOCAL
  EXIT /B 1
)
IF /I NOT "%EXPECTED_TOOL_TAG%"=="%TRUSTED_VCPKG_TOOL_TAG%" (
  ENDLOCAL
  EXIT /B 1
)
ECHO(%EXPECTED_TOOL_TAG%| FINDSTR /R /X "[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]" >NUL
IF ERRORLEVEL 1 (
  ENDLOCAL
  EXIT /B 1
)
SET "EXPECTED_TOOL_SHA256=%TRUSTED_VCPKG_TOOL_SHA256_X64%"
IF /I "%PROCESSOR_ARCHITECTURE%"=="ARM64" SET "EXPECTED_TOOL_SHA256=%TRUSTED_VCPKG_TOOL_SHA256_ARM64%"
IF /I "%PROCESSOR_ARCHITEW6432%"=="ARM64" SET "EXPECTED_TOOL_SHA256=%TRUSTED_VCPKG_TOOL_SHA256_ARM64%"
"%SystemRoot%\System32\certutil.exe" -hashfile "%VALIDATE_DIR%\vcpkg.exe" SHA256 2>NUL | FINDSTR /I /X /C:"%EXPECTED_TOOL_SHA256%" >NUL
IF ERRORLEVEL 1 (
  ENDLOCAL
  EXIT /B 1
)
"%VALIDATE_DIR%\vcpkg.exe" version 2>NUL | FINDSTR /B /I /L /C:"vcpkg package management program version %EXPECTED_TOOL_TAG%-" >NUL
SET "VALIDATION_RESULT=%ERRORLEVEL%"
ENDLOCAL & EXIT /B %VALIDATION_RESULT%
