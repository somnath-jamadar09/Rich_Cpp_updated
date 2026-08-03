@echo off
REM Windows Batch script to compile and run all rich-cpp C++ test drivers

cd /d "%~dp0"
echo === RICH-CPP TEST SUITE RUNNER ===
echo Working directory set to: %CD%

call tests\run_all.bat
