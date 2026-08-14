@echo off
setlocal EnableDelayedExpansion
chcp 65001 >nul
set ROOT=%~dp0
cd /d "%ROOT%"
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

:: Paquete siempre desde cero: evita archivos viejos enmascarando errores
if exist "%DIST%" (
    echo [INFO] Limpiando paquete anterior...
    rmdir /s /q "%DIST%"
)

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
if exist nico-ide\build (
    echo [INFO] Borrando build anterior para evitar conflictos de cache...
    rmdir /s /q nico-ide\build
)
mkdir nico-ide\build
cd nico-ide\build
cmake .. -G "MinGW Makefiles" ^
    -DCMAKE_PREFIX_PATH=C:/msys64/mingw64 ^
    -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe ^
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
if not exist "%DIST%" mkdir "%DIST%"
if not exist "%BIN%" mkdir "%BIN%"
if not exist "%SRC%" mkdir "%SRC%"
if not exist "%DIST%\nico-ide" mkdir "%DIST%\nico-ide"
if not exist "%IDE_SRC%" mkdir "%IDE_SRC%"
if not exist "%EJEMPLOS%" mkdir "%EJEMPLOS%"
if not exist "%DOCS%" mkdir "%DOCS%"
if not exist "%SCRIPTS%" mkdir "%SCRIPTS%"

echo Copiando binarios...
copy /Y nico.exe "%BIN%\"
if %errorlevel% neq 0 echo [AVISO] No se pudo copiar nico.exe
copy /Y nico-ide\build\nico-ide.exe "%BIN%\"
if %errorlevel% neq 0 echo [AVISO] No se pudo copiar nico-ide.exe

echo Desplegando dependencias de Qt5...
C:\msys64\mingw64\bin\windeployqt-qt5.exe --release --no-compiler-runtime "%BIN%\nico-ide.exe"

echo Copiando dependencias de sistema...
copy /Y C:\msys64\mingw64\bin\libsqlite3-0.dll "%BIN%\" >nul 2>&1
copy /Y C:\msys64\mingw64\bin\libgcc_s_seh-1.dll "%BIN%\" >nul 2>&1
copy /Y C:\msys64\mingw64\bin\libwinpthread-1.dll "%BIN%\" >nul 2>&1
copy /Y "C:\msys64\mingw64\bin\libstdc++-6.dll" "%BIN%\" >nul 2>&1

echo.
echo [4/4] Empaquetando codigo fuente y recursos...
xcopy /E /I /Y src\*.c "%SRC%\" >nul
xcopy /E /I /Y src\*.h "%SRC%\" >nul
xcopy /E /I /Y nico-ide\src\*.cpp "%IDE_SRC%\" >nul
xcopy /E /I /Y nico-ide\src\*.h "%IDE_SRC%\" >nul
if exist nico-ide\CMakeLists.txt copy /Y nico-ide\CMakeLists.txt "%DIST%\nico-ide\" >nul
if exist compile.sh copy /Y compile.sh "%DIST%\" >nul 2>&1
if exist compile_windows.bat copy /Y compile_windows.bat "%DIST%\" >nul 2>&1
if exist nico_stack.def copy /Y nico_stack.def "%DIST%\" >nul 2>&1
xcopy /E /I /Y ejemplos "%EJEMPLOS%\" >nul
xcopy /E /I /Y docs "%DOCS%\" >nul
if exist README.md copy /Y README.md "%DIST%\" >nul 2>&1
if exist LICENSE copy /Y LICENSE "%DIST%\" >nul 2>&1
if exist CHANGELOG.md copy /Y CHANGELOG.md "%DIST%\" >nul 2>&1
if exist scripts\install_msys2.ps1 copy /Y scripts\install_msys2.ps1 "%SCRIPTS%\" >nul 2>&1

echo.
echo ========================================
echo   Paquete listo en la carpeta: %DIST%
echo ========================================
echo.
echo Verificacion de binarios empaquetados:
for %%F in ("%BIN%\nico.exe" "%BIN%\nico-ide.exe") do (
    if exist %%F (echo   %%~nxF  -^>  %%~tF) else (echo   %%~nxF  -^> [NO EXISTE])
)
echo.
pause