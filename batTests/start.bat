@echo off
setlocal enabledelayedexpansion

cd /d "%~dp0"

set EXE=..\x64\Debug\TruthTableMaker.exe
set INPUT_DIR=input
set OUTPUT_DIR=output
set FUNC_DIR=functions

if not exist "%EXE%" (
    echo ERROR: executable not found: %EXE%
    exit /b 1
)

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

set PASS=0
set FAIL=0

for %%f in (%INPUT_DIR%\*_input.txt) do (
    set "tname=%%~nf"
    set "tname=!tname:_input=!"

    set "ARGS=-i %%f -o %OUTPUT_DIR%\!tname!_out.csv"
    if exist "%FUNC_DIR%\!tname!_functions.txt" (
        set "ARGS=!ARGS! -f %FUNC_DIR%\!tname!_functions.txt"
    )

    echo [TEST] !tname!
    "%EXE%" !ARGS!
    if !errorlevel! equ 0 (
        echo   PASS
        set /a PASS+=1
    ) else (
        echo   FAIL
        set /a FAIL+=1
    )
)

echo.
echo Results: !PASS! passed, !FAIL! failed
if !FAIL! gtr 0 exit /b 1
