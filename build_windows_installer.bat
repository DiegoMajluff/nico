@echo off
setlocal EnableDelayedExpansion
chcp 65001 >nul

set ROOT=%~dp0
set DIST=%ROOT%dist
set BIN=%DIST%\bin
set SRC=%DIST%\src
set IDE_SRC=%DIST%\nico-ide\src
set EJEMPLOS=%DIST%\ejemplos
set DOCS=%DIST%\docs
set SCRIPTS=%DIST%\scripts

echo ========================================
echo  Empaquetando Nico v2.1.0 para Windows
echo ========================================
echo.

where gcc >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] gcc no encontrado.
    pause
    exit /b 1
)

echo [1/4] Compilando interprete (nico.exe)...
call compile_windows.bat
if %errorlevel% neq 0 (
    echo [ERROR] Fallo la compilacion del interprete
    pause
    exit /b 1
)
echo.

echo [2/4] Compilando IDE (nico-ide.exe)...
if not exist nico-ide\build mkdir nico-ide\build
cd nico-ide\build

cmake .. -G "MinGW Makefiles" ^
    -DCMAKE_PREFIX_PATH=C:/msys64/mingw64 ^
    -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe ^
    -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe ^
    -DCMAKE_BUILD_TYPE=Release

if %errorlevel% neq 0 (
    echo [ERROR] Fallo la configuracion de CMake
    cd ..\..
    pause
    exit /b 1
)

mingw32-make -j%NUMBER_OF_PROCESSORS%
if %errorlevel% neq 0 (
    echo [ERROR] Fallo la compilacion del IDE
    cd ..\..
    pause
    exit /b 1
)
cd ..\..
echo.

echo [3/4] Empaquetando dependencias (DLLs)...
if not exist %DIST% mkdir %DIST%
if not exist %BIN% mkdir %BIN%
if not exist %SRC% mkdir %SRC%
if not exist %DIST%\nico-ide mkdir %DIST%\nico-ide
if not exist %IDE_SRC% mkdir %IDE_SRC%
if not exist %EJEMPLOS% mkdir %EJEMPLOS%
if not exist %DOCS% mkdir %DOCS%
if not exist %SCRIPTS% mkdir %SCRIPTS%

copy nico.exe %BIN%\ >nul
copy nico-ide\build\nico-ide.exe %BIN%\ >nul

echo Desplegando dependencias de Qt5...
C:\msys64\mingw64\bin\windeployqt.exe --release --no-compiler-runtime %BIN%\nico-ide.exe >nul 2>&1

echo Copiando dependencias de sistema...
copy C:\msys64\mingw64\bin\libsqlite3-0.dll %BIN%\ >nul 2>&1
copy C:\msys64\mingw64\bin\libgcc_s_seh-1.dll %BIN%\ >nul 2>&1
copy C:\msys64\mingw64\bin\libwinpthread-1.dll %BIN%\ >nul 2>&1
echo.

echo [4/4] Empaquetando codigo fuente y recursos...
xcopy /E /I /Y src\*.c %SRC%\ >nul
xcopy /E /I /Y src\*.h %SRC%\ >nul
xcopy /E /I /Y nico-ide\src\*.cpp %IDE_SRC%\ >nul
xcopy /E /I /Y nico-ide\src\*.h %IDE_SRC%\ >nul
copy nico-ide\CMakeLists.txt %DIST%\nico-ide\ >nul

copy compile.sh %DIST%\ >nul 2>&1
copy compile_windows.bat %DIST%\ >nul 2>&1

xcopy /E /I /Y ejemplos %EJEMPLOS%\ >nul
xcopy /E /I /Y docs %DOCS%\ >nul
copy README.md %DIST%\ >nul 2>&1
copy LICENSE %DIST%\ >nul 2>&1
copy CHANGELOG.md %DIST%\ >nul 2>&1

if exist scripts\install_msys2.ps1 (
    copy scripts\install_msys2.ps1 %SCRIPTS%\ >nul 2>&1
)

echo.
echo ========================================
echo   Paquete listo en la carpeta: %DIST%
echo ========================================
pause