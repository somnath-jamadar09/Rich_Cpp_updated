@echo off
REM Windows Batch script to compile and run all rich-cpp C++ test drivers in tests/port
cd /d "%~dp0"
echo === RICH-CPP TEST SUITE RUNNER ===
echo Working directory: %CD%

g++ -std=c++17 -Isrc src/main.cpp -o rich_demo.exe
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
.\rich_demo.exe

for %%f in (tests\port\*.cpp) do (
    echo Building %%f...
    g++ -std=c++17 -Isrc "%%f" -o "%%~dpnff.exe"
    if %ERRORLEVEL% EQU 0 (
        echo Running %%~nff...
        "%%~dpnff.exe"
    )
)
echo === ALL C++ TESTS PASSED ===

