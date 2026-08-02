#!/bin/bash
# ============================================================================
# Nico v2.1.0 - Constructor de AppImage para Linux/Raspberry Pi
# ============================================================================
set -e

echo "╔════════════════════════════════════════╗"
echo "║   Construyendo AppImage de Nico v2.1.0 ║"
echo "╚════════════════════════════════════════╝"
echo ""

BASE_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$BASE_DIR"

# 1. Compilar intérprete
echo -e "\e[34m[1/6]\e[0m Compilando intérprete Nico..."
chmod +x compile.sh
./compile.sh
echo ""

# 2. Compilar IDE
echo -e "\e[34m[2/6]\e[0m Compilando IDE Nico..."
if [ -d "nico-ide" ]; then
    cd nico-ide
    if [ -d "build" ]; then
        echo "   Limpiando build anterior..."
        if ! rm -rf build 2>/dev/null; then
            echo "   Se necesitan permisos de root para limpiar build/"
            sudo rm -rf build
        fi
    fi
    mkdir -p build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    cd "$BASE_DIR"
else
    echo -e "\e[33m[WARN]\e[0m Directorio nico-ide no encontrado. Saltando compilación del IDE."
fi
echo ""

# 3. Descargar linuxdeploy si no existe
echo -e "\e[34m[3/6]\e[0m Preparando herramientas de empaquetado..."
ARCH=$(uname -m)
LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-${ARCH}.AppImage"
QT_PLUGIN_URL="https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-${ARCH}.AppImage"

if [ ! -f linuxdeploy ]; then
    echo "   Descargando linuxdeploy..."
    wget -q "$LINUXDEPLOY_URL" -O linuxdeploy
    chmod +x linuxdeploy
fi

if [ ! -f linuxdeploy-plugin-qt ]; then
    echo "   Descargando plugin de Qt..."
    wget -q "$QT_PLUGIN_URL" -O linuxdeploy-plugin-qt
    chmod +x linuxdeploy-plugin-qt
fi
echo ""

# 4. Crear estructura AppDir
echo -e "\e[34m[4/6]\e[0m Creando estructura del AppImage..."
APPDIR="$BASE_DIR/nico.AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/share/nico"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"

# Copiar binarios
cp nico "$APPDIR/usr/bin/nico"
if [ -f "nico-ide/build/nico-ide" ]; then
    cp nico-ide/build/nico-ide "$APPDIR/usr/bin/nico-ide"
fi

# Copiar ejemplos y documentación
if [ -d "ejemplos" ]; then cp -r ejemplos "$APPDIR/usr/share/nico/"; fi
if [ -d "docs" ]; then cp -r docs "$APPDIR/usr/share/nico/"; fi
[ -f README.md ] && cp README.md "$APPDIR/usr/share/nico/"
[ -f LICENSE ] && cp LICENSE "$APPDIR/usr/share/nico/"

# Crear archivo .desktop (CORREGIDO: Version=1.5 es la especificación, no la versión de la app)
cat > "$APPDIR/nico-ide.desktop" << 'EOF'
[Desktop Entry]
Version=1.5
Type=Application
Name=Nico IDE
Comment=Entorno de desarrollo para el lenguaje Nico v2.1.0
Exec=nico-ide %f
Icon=nico-ide
Terminal=false
Categories=Development;IDE;
MimeType=text/x-nico;
X-AppImage-Version=2.1.0
EOF

# Crear ícono placeholder
convert -size 256x256 xc:gray "$APPDIR/nico-ide.png" 2>/dev/null || \
    echo "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg==" | base64 -d > "$APPDIR/nico-ide.png"

echo ""

# 5. Crear script AppRun personalizado (SIN ADVERTENCIAS DE WAYLAND)
echo -e "\e[34m[5/6]\e[0m Creando script wrapper interno (AppRun)..."
cat > "$BASE_DIR/custom_apprun.sh" << 'WRAPPER_EOF'
#!/bin/bash
# Script wrapper interno del AppImage

# Obtener el directorio real del AppImage montado
APPDIR="$(dirname "$(readlink -f "$0")")"

# 1. Detectar el nombre con el que se invocó el AppImage
INVOCATION_NAME=$(basename "$0")

# 2. Forzar plataforma XCB (X11) para evitar advertencias de Wayland
# y asegurar compatibilidad total con el gestor de ventanas
export QT_QPA_PLATFORM=xcb

# 3. Forzar tema GTK3 para integración visual con el sistema
export QT_QPA_PLATFORMTHEME=gtk3

# 4. Lógica de decisión: Si se llamó como "nico-ide" O NICO_MODE es "ide", abrir el IDE
if [[ "$INVOCATION_NAME" == *"nico-ide"* ]] || [ "$NICO_MODE" = "ide" ]; then
    exec "$APPDIR/usr/bin/nico-ide" "$@"
else
    exec "$APPDIR/usr/bin/nico" "$@"
fi
WRAPPER_EOF
chmod +x "$BASE_DIR/custom_apprun.sh"

echo ""

# 6. Empaquetar AppImage
echo -e "\e[34m[6/6]\e[0m Empaquetando AppImage..."
export VERSION=2.1.0
export ARCH=$(uname -m)

./linuxdeploy \
    --appdir "$APPDIR" \
    --plugin qt \
    --output appimage \
    --custom-apprun="$BASE_DIR/custom_apprun.sh"

# Renombrar el archivo generado
OUTPUT_FILE="Nico-2.1.0-${ARCH}.AppImage"
if [ -f "Nico_IDE-2.1.0-${ARCH}.AppImage" ]; then
    mv "Nico_IDE-2.1.0-${ARCH}.AppImage" "$OUTPUT_FILE"
elif [ -f "nico-ide-2.1.0-${ARCH}.AppImage" ]; then
    mv "nico-ide-2.1.0-${ARCH}.AppImage" "$OUTPUT_FILE"
fi

# Limpiar archivo temporal
rm -f "$BASE_DIR/custom_apprun.sh"

echo ""
echo "╔════════════════════════════════════════╗"
echo "║      ¡AppImage creado exitosamente!    ║"
echo "╚════════════════════════════════════════╝"
echo ""
echo "Archivo generado: $OUTPUT_FILE"
echo ""
echo "Para instalar en el sistema (agregar a PATH):"
echo "  sudo ./install_appimage.sh $OUTPUT_FILE"
echo ""
