@echo off

SETLOCAL ENABLEEXTENSIONS DISABLEDELAYEDEXPANSION
SET "SCRIPT_DIR=%~dp0"
FOR %%I IN ("%SCRIPT_DIR%..") DO SET "REPO_ROOT=%%~fI"
SET "CDT_VCPKG_MANIFEST=%REPO_ROOT%\vcpkg.json"
SET "MODE=%~1"

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
    echo Using vcpkg at %VCPKG_DIR% ^(%BASELINE%^)
    EXIT /B 0
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

echo Bootstrapped vcpkg at %VCPKG_DIR% ^(%BASELINE%^)
EXIT /B 0
