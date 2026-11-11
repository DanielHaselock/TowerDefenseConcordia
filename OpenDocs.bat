@echo off

echo Building documentation...
cmake --build build --target docs

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Failed to build documentation.
    pause
    exit /b %ERRORLEVEL%
)