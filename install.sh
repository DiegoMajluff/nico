#!/bin/bash
# ============================================================================
# Nico v2.1.0 - Instalador para Linux
# ============================================================================
set -e

echo -e "\e[34m[INFO]\e[0m Iniciando instalación de Nico v2.1.0..."

# 1. Verificar permisos de root
if [ "$EUID" -ne 0 ]; then 
    echo -e "\e[31m[ERROR]\e[0m Este script debe ejecutarse con sudo"
    echo "Uso: sudo ./install.sh"
    exit 1
fi

# 2. Detectar distribución e instalar dependencias (Opcional, para desarrolladores)
echo -e "\e[34m[INFO]\e[0m Verificando dependencias del sistema..."
if [ -f /etc/os-release ]; then
    . /etc/os-release
    if [[ "$ID" == "ubuntu" || "$ID" == "debian" || "$ID" == "linuxmint" ]]; then
        apt-get update -qq
        apt-get install -y -qq build-essential gcc cmake qt5-qmake qtbase5-dev libsqlite3-dev
    elif [[ "$ID" == "fedora" || "$ID" == "rhel" ]]; then
        dnf install -y -q gcc gcc-c++ make cmake qt5-qtbase-devel sqlite-devel
    elif [[ "$ID" == "arch" || "$ID" == "manjaro" ]]; then
        pacman -S --noconfirm --needed gcc cmake qt5-base sqlite
    fi
fi

# 3. Compilar intérprete y IDE
echo -e "\e[34m[INFO]\e[0m Compilando intérprete y IDE..."
cd "$(dirname "$0")"
chmod +x compile.sh
./compile.sh

if [ -d "nico-ide" ]; then
    cd nico-ide
    if [ -d "build" ]; then rm -rf build; fi
    mkdir -p build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    cd ../..
fi

# 4. Instalar binarios
echo -e "\e[34m[INFO]\e[0m Instalando binarios en /usr/local/bin/..."
cp nico /usr/local/bin/nico
chmod +x /usr/local/bin/nico
if [ -f "nico-ide/build/nico-ide" ]; then
    cp nico-ide/build/nico-ide /usr/local/bin/nico-ide
    chmod +x /usr/local/bin/nico-ide
fi

# 5. Instalar Ícono y crear acceso directo (.desktop)
echo -e "\e[34m[INFO]\e[0m Instalando ícono y creando acceso directo..."
mkdir -p /usr/share/icons/hicolor/256x256/apps
mkdir -p /usr/share/applications

# Copiar el ícono personalizado si existe
if [ -f "nico-icon.png" ]; then
    cp nico-icon.png /usr/share/icons/hicolor/256x256/apps/nico-ide.png
fi

# Crear el .desktop apuntando al ícono instalado
cat > /usr/share/applications/nico-ide.desktop << 'EOF'
[Desktop Entry]
Version=2.1.0
Type=Application
Name=Nico IDE
Comment=Entorno de desarrollo para el lenguaje Nico v2.1.0
Exec=/usr/local/bin/nico-ide %f
Icon=nico-ide
Terminal=false
Categories=Development;IDE;
MimeType=text/x-nico;
EOF

# 6. Copiar ejemplos y documentación (Para que el REPL los encuentre)
echo -e "\e[34m[INFO]\e[0m Copiando ejemplos y documentación..."
mkdir -p /usr/local/share/nico
if [ -d "ejemplos" ]; then cp -r ejemplos /usr/local/share/nico/; fi
if [ -d "docs" ]; then cp -r docs /usr/local/share/nico/; fi
[ -f README.md ] && cp README.md /usr/local/share/nico/
[ -f LICENSE ] && cp LICENSE /usr/local/share/nico/

# 7. Actualizar cachés del sistema (¡Clave para que aparezca el ícono al instante!)
echo -e "\e[34m[INFO]\e[0m Actualizando cachés del sistema..."
if command -v update-desktop-database &> /dev/null; then
    update-desktop-database /usr/share/applications
fi
if command -v gtk-update-icon-cache &> /dev/null; then
    gtk-update-icon-cache -f /usr/share/icons/hicolor
fi

echo -e "\e[32m[OK]\e[0m ¡Instalación de Nico v2.1.0 completada exitosamente!"
echo -e "Ejecuta \e[1mnico\e[0m o \e[1mnico-ide\e[0m desde cualquier terminal."