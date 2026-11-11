@echo off
setlocal

rem Use VCPKG_ROOT if defined, otherwise C:\vcpkg
if not defined VCPKG_ROOT set "VCPKG_ROOT=C:\vcpkg"

set "VCPKG_TOOLCHAIN=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"

if not exist "%VCPKG_TOOLCHAIN%" (
    echo ERROR: vcpkg toolchain not found!
    echo %VCPKG_TOOLCHAIN%
    pause
    exit /b 1
)

set "BUILD_DIR=build"

echo ==============================
echo Generating CMake build files...
echo ==============================

cmake -S . -B "%BUILD_DIR%" -A x64 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_TOOLCHAIN%"

if errorlevel 1 (
    echo.
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo ==============================
echo CMake generation successful!
echo Build files are in .\%BUILD_DIR%
echo ==============================

pause