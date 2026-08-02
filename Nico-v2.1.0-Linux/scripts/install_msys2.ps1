# ============================================================================
# Nico v2.1.0 - Script de instalación de MSYS2 para Windows
# Uso: Ejecutar como Administrador
# ============================================================================

param(
    [switch]$Silent,
    [string]$InstallPath = "C:\msys64"
)

$ErrorActionPreference = "Stop"

function Write-Log($Message, $Type = "INFO") {
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $color = switch ($Type) {
        "INFO"    { "Cyan" }
        "SUCCESS" { "Green" }
        "WARNING" { "Yellow" }
        "ERROR"   { "Red" }
        default   { "White" }
    }
    Write-Host "[$timestamp] [$Type] $Message" -ForegroundColor $color
}

function Test-Admin {
    $currentUser = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
    return $currentUser.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

# Verificar permisos de administrador
if (-not (Test-Admin)) {
    Write-Log "Este script debe ejecutarse como Administrador" "ERROR"
    Write-Log "Haga clic derecho en PowerShell y seleccione 'Ejecutar como administrador'" "ERROR"
    exit 1
}

Write-Log "==========================================" "INFO"
Write-Log "Instalador de MSYS2 para Nico v2.1.0" "INFO"
Write-Log "==========================================" "INFO"

# URL del instalador de MSYS2 (última versión estable)
$msys2Url = "https://github.com/msys2/msys2-installer/releases/download/2024-07-27/msys2-x86_64-20240727.exe"
$installerPath = "$env:TEMP\msys2-installer.exe"

# Paso 1: Verificar si MSYS2 ya está instalado
if (Test-Path "$InstallPath\usr\bin\bash.exe") {
    Write-Log "MSYS2 ya está instalado en $InstallPath" "WARNING"
    Write-Log "Solo se instalarán los paquetes necesarios" "INFO"
} else {
    # Paso 2: Descargar MSYS2
    Write-Log "Descargando MSYS2 desde: $msys2Url" "INFO"
    Write-Log "Esto puede tardar varios minutos dependiendo de su conexión..." "INFO"
    
    try {
        Invoke-WebRequest -Uri $msys2Url -OutFile $installerPath -UseBasicParsing
        Write-Log "Descarga completada" "SUCCESS"
    } catch {
        Write-Log "Error al descargar MSYS2: $_" "ERROR"
        Write-Log "Puede descargarlo manualmente desde: https://www.msys2.org/" "ERROR"
        exit 1
    }
    
    # Paso 3: Instalar MSYS2
    Write-Log "Instalando MSYS2 en $InstallPath..." "INFO"
    Write-Log "Esto puede tardar varios minutos..." "INFO"
    
    try {
        $arguments = "install --root `"$InstallPath`" --confirm-command"
        Start-Process -FilePath $installerPath -ArgumentList $arguments -Wait -NoNewWindow
        
        if (-not (Test-Path "$InstallPath\usr\bin\bash.exe")) {
            Write-Log "La instalación de MSYS2 falló" "ERROR"
            exit 1
        }
        
        Write-Log "MSYS2 instalado correctamente" "SUCCESS"
    } catch {
        Write-Log "Error al instalar MSYS2: $_" "ERROR"
        exit 1
    }
    
    # Limpiar archivo de instalación
    Remove-Item $installerPath -Force -ErrorAction SilentlyContinue
}

# Paso 4: Actualizar paquetes base
Write-Log "Actualizando paquetes base de MSYS2..." "INFO"
try {
    & "$InstallPath\usr\bin\bash.exe" -lc "pacman -Syu --noconfirm"
    Write-Log "Paquetes base actualizados" "SUCCESS"
} catch {
    Write-Log "Error al actualizar paquetes base: $_" "WARNING"
}

# Paso 5: Instalar paquetes de desarrollo necesarios para Nico
Write-Log "Instalando herramientas de compilación..." "INFO"
$packages = @(
    "mingw-w64-x86_64-gcc",
    "mingw-w64-x86_64-cmake",
    "mingw-w64-x86_64-make",
    "mingw-w64-x86_64-qt5-base",
    "mingw-w64-x86_64-qt5-tools",
    "mingw-w64-x86_64-sqlite3",
    "mingw-w64-x86_64-readline",
    "mingw-w64-x86_64-pkg-config"
)

$packageList = $packages -join " "

try {
    & "$InstallPath\usr\bin\bash.exe" -lc "pacman -S --noconfirm --needed $packageList"
    Write-Log "Paquetes instalados correctamente" "SUCCESS"
} catch {
    Write-Log "Error al instalar paquetes: $_" "ERROR"
    Write-Log "Puede intentar instalarlos manualmente ejecutando:" "ERROR"
    Write-Log "  pacman -S $packageList" "ERROR"
    exit 1
}

# Paso 6: Agregar MSYS2 al PATH del sistema
Write-Log "Agregando MSYS2 al PATH del sistema..." "INFO"
$msys2Paths = @(
    "$InstallPath\mingw64\bin",
    "$InstallPath\usr\bin"
)

$currentPath = [Environment]::GetEnvironmentVariable("Path", "Machine")

foreach ($path in $msys2Paths) {
    if ($currentPath -notlike "*$path*") {
        $currentPath = "$currentPath;$path"
        Write-Log "Agregado al PATH: $path" "INFO"
    } else {
        Write-Log "Ya está en el PATH: $path" "WARNING"
    }
}

[Environment]::SetEnvironmentVariable("Path", $currentPath, "Machine")
Write-Log "PATH actualizado" "SUCCESS"

# Paso 7: Verificar instalación
Write-Log "Verificando instalación..." "INFO"
$tools = @(
    @{ Name = "gcc"; Path = "$InstallPath\mingw64\bin\gcc.exe" },
    @{ Name = "cmake"; Path = "$InstallPath\mingw64\bin\cmake.exe" },
    @{ Name = "qmake"; Path = "$InstallPath\mingw64\bin\qmake.exe" }
)

$allOk = $true
foreach ($tool in $tools) {
    if (Test-Path $tool.Path) {
        Write-Log "✓ $($tool.Name) instalado" "SUCCESS"
    } else {
        Write-Log "✗ $($tool.Name) NO encontrado" "ERROR"
        $allOk = $false
    }
}

if ($allOk) {
    Write-Log "==========================================" "SUCCESS"
    Write-Log "¡MSYS2 instalado correctamente!" "SUCCESS"
    Write-Log "==========================================" "SUCCESS"
    Write-Log "" "INFO"
    Write-Log "Ahora puede compilar Nico ejecutando:" "INFO"
    Write-Log "  cd 'C:\Program Files\Nico'" "INFO"
    Write-Log "  compile_windows.bat" "INFO"
    Write-Log "" "INFO"
    Write-Log "Para compilar el IDE:" "INFO"
    Write-Log "  cd nico-ide" "INFO"
    Write-Log "  mkdir build && cd build" "INFO"
    Write-Log "  cmake .. -G 'MinGW Makefiles'" "INFO"
    Write-Log "  mingw32-make" "INFO"
} else {
    Write-Log "==========================================" "ERROR"
    Write-Log "La instalación tuvo problemas" "ERROR"
    Write-Log "==========================================" "ERROR"
    exit 1
}

if (-not $Silent) {
    Write-Host ""
    Write-Host "Presione cualquier tecla para salir..." -ForegroundColor Cyan
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
}
