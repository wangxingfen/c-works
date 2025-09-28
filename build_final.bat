@echo off
setlocal

echo Compiling chat program...
echo Current directory: %CD%

REM Set VCPKG_ROOT if not already set
if "%VCPKG_ROOT%"=="" (
    set VCPKG_ROOT=C:\c++\vcpkg
    echo Setting VCPKG_ROOT to %VCPKG_ROOT%
) else (
    echo Using existing VCPKG_ROOT=%VCPKG_ROOT%
)

REM Check that required directories exist
echo Checking for include directory...
if exist "%VCPKG_ROOT%\installed\x64-windows\include" (
    echo Found include directory
) else (
    echo Missing include directory
    exit /b 1
)

echo Checking for library directory...
if exist "%VCPKG_ROOT%\installed\x64-windows\lib" (
    echo Found library directory
) else (
    echo Missing library directory
    exit /b 1
)

REM Use g++ to compile with vcpkg paths
echo Compiling with g++...
g++ -std=c++17 chat.cpp -o chat.exe ^
    -I"%VCPKG_ROOT%\installed\x64-windows\include" ^
    -L"%VCPKG_ROOT%\installed\x64-windows\lib" ^
    -lcurl

if %errorlevel% == 0 (
    echo Compilation completed successfully!
    echo Executable file: chat.exe
) else (
    echo Compilation failed!
    exit /b 1
)