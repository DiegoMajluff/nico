#!/bin/bash
# ============================================================================
# Nico v2.1.0 - Instalador del AppImage en el sistema
# Agrega 'nico' y 'nico-ide' al PATH del usuario de forma robusta
# ============================================================================
set -e

echo "╔════════════════════════════════════════╗"
echo "║   Instalando Nico AppImage en sistema  ║"
echo "╚════════════════════════════════════════╝"
echo ""

if [ "$EUID" -ne 0 ]; then
    echo -e "\e[31m[ERROR]\e[0m Este script debe ejecutarse con sudo"
    echo "Uso: sudo ./install_appimage.sh <archivo.AppImage>"
    exit 1
fi

if [ -z "$1" ]; then
    echo -e "\e[31m[ERROR]\e[0m Falta el archivo AppImage"
    echo "Uso: sudo ./install_appimage.sh <archivo.AppImage>"
    exit 1
fi

APPIMAGE_FILE="$1"
if [ ! -f "$APPIMAGE_FILE" ]; then
    echo -e "\e[31m[ERROR]\e[0m El archivo '$APPIMAGE_FILE' no existe"
    exit 1
fi

APPIMAGE_FILE="$(cd "$(dirname "$APPIMAGE_FILE")" && pwd)/$(basename "$APPIMAGE_FILE")"

echo -e "\e[34m[INFO]\e[0m Instalando: $APPIMAGE_FILE"
echo ""

# 1. Crear directorio de instalación
INSTALL_DIR="/opt/nico"
echo -e "\e[34m[1/4]\e[0m Creando directorio de instalación: $INSTALL_DIR"
mkdir -p "$INSTALL_DIR"

# 2. Copiar AppImage
echo -e "\e[34m[2/4]\e[0m Copiando AppImage..."
cp "$APPIMAGE_FILE" "$INSTALL_DIR/nico.AppImage"
chmod +x "$INSTALL_DIR/nico.AppImage"

# 3. Crear wrappers en /usr/local/bin usando variable de entorno
echo -e "\e[34m[3/4]\e[0m Creando accesos directos en /usr/local/bin/..."

# Wrapper para 'nico' (intérprete)
cat > /usr/local/bin/nico << 'WRAPPER_EOF'
#!/bin/bash
export NICO_MODE="interpreter"
exec /opt/nico/nico.AppImage "$@"
WRAPPER_EOF
chmod +x /usr/local/bin/nico

# Wrapper para 'nico-ide' (IDE)
cat > /usr/local/bin/nico-ide << 'WRAPPER_EOF'
#!/bin/bash
export NICO_MODE="ide"
exec /opt/nico/nico.AppImage "$@"
WRAPPER_EOF
chmod +x /usr/local/bin/nico-ide

# 4. Crear archivo .desktop para el menú de aplicaciones
echo -e "\e[34m[4/4]\e[0m Creando acceso directo en el menú de aplicaciones..."
mkdir -p /usr/share/applications
cat > /usr/share/applications/nico-ide.desktop << 'EOF'
[Desktop Entry]
Version=1.5
Type=Application
Name=Nico IDE
Comment=Entorno de desarrollo para el lenguaje Nico v2.1.0
Exec=/usr/local/bin/nico-ide %f
Icon=utilities-terminal
Terminal=false
Categories=Development;IDE;
MimeType=text/x-nico;
EOF

# 5. Crear script de desinstalación
cat > /usr/local/bin/nico-uninstall << 'UNINSTALL_EOF'
#!/bin/bash
if [ "$EUID" -ne 0 ]; then
    echo "Este script debe ejecutarse con sudo"
    exit 1
fi
echo "Desinstalando Nico..."
rm -f /usr/local/bin/nico
rm -f /usr/local/bin/nico-ide
rm -f /usr/local/bin/nico-uninstall
rm -f /usr/share/applications/nico-ide.desktop
rm -rf /opt/nico
echo "¡Nico ha sido desinstalado correctamente!"
UNINSTALL_EOF
chmod +x /usr/local/bin/nico-uninstall

echo ""
echo "╔════════════════════════════════════════╗"
echo "║    ¡Instalación completada!            ║"
echo "╚════════════════════════════════════════╝"
echo ""
echo "Ahora podés ejecutar desde cualquier terminal:"
echo ""
echo "  📝 Intérprete:  nico programa.nico"
echo "  💻 IDE:         nico-ide"
echo "  🗑️  Desinstalar: sudo nico-uninstall"
echo ""
