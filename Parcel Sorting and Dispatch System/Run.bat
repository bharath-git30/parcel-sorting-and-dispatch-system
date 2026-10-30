@echo off
cd /d "%~dp0"
if not exist "parcel_system.exe" (
    echo The executable is missing. Run "Source Code\Build.bat" first.
    pause
    exit /b 1
)
parcel_system.exe
pause
