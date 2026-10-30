@echo off
setlocal
cd /d "%~dp0"
where gcc >nul 2>nul
if %errorlevel% equ 0 goto gcc
where cl >nul 2>nul
if %errorlevel% equ 0 goto msvc
if exist "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86 >nul
    goto msvc
)
echo No C compiler was found. Install GCC or use a Visual Studio Developer Command Prompt.
exit /b 1

:gcc
gcc -std=c11 -Wall -Wextra -Iinclude src\parcel.c src\queue.c src\stack.c src\bst.c src\hashtable.c src\circular_list.c src\config.c src\simulation.c src\ui.c src\main.c -o "..\parcel_system.exe"
if errorlevel 1 exit /b 1
goto success

:msvc
if not exist build mkdir build
cl /nologo /W3 /D_CRT_SECURE_NO_WARNINGS /Iinclude /Fo"build\\" src\parcel.c src\queue.c src\stack.c src\bst.c src\hashtable.c src\circular_list.c src\config.c src\simulation.c src\ui.c src\main.c /Fe"..\parcel_system.exe"
if errorlevel 1 exit /b 1

:success
echo Build successful. Open Run.bat in the main project folder.
exit /b 0
