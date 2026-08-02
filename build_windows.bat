@echo off
setlocal EnableDelayedExpansion
echo ========================================
echo  Compilando Nico v2.1.0 para Windows
echo ========================================
echo.

:: 1. Verificar herramientas
where gcc >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] gcc no encontrado.
    pause
    exit /b 1
)

where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] cmake no encontrado.
    pause
    exit /b 1
)

if not exist "C:\msys64\mingw64\bin\Qt5Core.dll" (
    echo [ERROR] Qt5 no encontrado.
    pause
    exit /b 1
)

:: 2. Compilar interprete
echo [1/3] Compilando interprete...
call compile_windows.bat
if %errorlevel% neq 0 (
    echo [ERROR] Fallo el interprete.
    pause
    exit /b 1
)

:: 3. Compilar IDE
echo [2/3] Compilando IDE...
if not exist nico-ide\build mkdir nico-ide\build
cd nico-ide\build
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:/msys64/mingw64 -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 (
    echo [ERROR] Fallo CMake.
    cd ..\..
    pause
    exit /b 1
)
mingw32-make -j%NUMBER_OF_PROCESSORS%
if %errorlevel% neq 0 (
    echo [ERROR] Fallo el IDE.
    cd ..\..
    pause
    exit /b 1
)
cd ..\..

:: 4. Empaquetar DLLs
echo [3/3] Empaquetando DLLs...
if not exist dist\bin mkdir dist\bin
copy nico.exe dist\bin\
copy nico-ide\build\nico-ide.exe dist\bin\

:: Desplegar dependencias de Qt5 usando windeployqt-qt5.exe
echo Desplegando dependencias de Qt5...
C:\msys64\mingw64\bin\windeployqt-qt5.exe --release --no-compiler-runtime dist\bin\nico-ide.exe

:: Copiar DLLs de SQLite
echo Copiando dependencias de SQLite...
copy C:\msys64\mingw64\bin\libsqlite3-0.dll dist\bin\ 2>nul

echo.
echo ========================================
echo   Listo! Paquete en dist\
echo ========================================
pause