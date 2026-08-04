#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_DIR"

VERSION="2.1.0"
APPDIR="$PROJECT_DIR/AppDir-x86_64"
OUTPUT=""

NICOC_BIN="$PROJECT_DIR/nico"
IDE_BIN_PATH="$PROJECT_DIR/nico-ide/build/nico-ide"

log() {
    echo "[nico-appimage] $*"
}

warn() {
    echo "[nico-appimage][WARN] $*" >&2
}

die() {
    echo "[nico-appimage][ERROR] $*" >&2
    exit 1
}

usage() {
    cat <<'USAGE'
Generador de AppImage para Nico + Nico IDE en Linux PC x86_64

Uso:
  ./appimage-nico-linux.sh [opciones]

Opciones:
  --version VERSION   Versión para el nombre del AppImage (por defecto: 2.1.0)
  --output ARCHIVO    Nombre del AppImage de salida
  -h, --help          Mostrar ayuda

Ejemplos:
  ./appimage-nico-linux.sh
  ./appimage-nico-linux.sh --version 2.1.0
USAGE
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --version)
            VERSION="${2:?}"
            shift 2
            ;;
        --output)
            OUTPUT="${2:?}"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            die "Opción desconocida: $1"
            ;;
    esac
done

if [[ "$(id -u)" -eq 0 ]]; then
    warn "Estás ejecutando como root. Para generar AppImage es mejor hacerlo como usuario normal."
fi

log "Directorio del proyecto: $PROJECT_DIR"
log "Versión: $VERSION"

# -----------------------------------------------------------------------------
# Verificar binarios
# -----------------------------------------------------------------------------

[[ -x "$NICOC_BIN" ]] || die "No existe ./nico. Compilalo con ./compile.sh"
[[ -x "$IDE_BIN_PATH" ]] || die "No existe nico-ide/build/nico-ide. Compilalo con CMake."

file "$NICOC_BIN" | grep -qiE "x86-64|x86_64" || warn "El intérprete no parece x86_64."
file "$IDE_BIN_PATH" | grep -qiE "x86-64|x86_64" || warn "El IDE no parece x86_64."
# -----------------------------------------------------------------------------
# Preparar AppDir
# -----------------------------------------------------------------------------

rm -rf "$APPDIR"

mkdir -p \
    "$APPDIR/usr/bin" \
    "$APPDIR/usr/lib" \
    "$APPDIR/usr/plugins" \
    "$APPDIR/usr/translations" \
    "$APPDIR/usr/share/applications" \
    "$APPDIR/usr/share/icons/hicolor/256x256/apps"

install -m755 "$NICOC_BIN" "$APPDIR/usr/bin/nico"
install -m755 "$IDE_BIN_PATH" "$APPDIR/usr/bin/nico-ide"

# -----------------------------------------------------------------------------
# qt.conf
# -----------------------------------------------------------------------------

cat > "$APPDIR/usr/bin/qt.conf" <<'EOF'
[Paths]
Prefix = ../
Plugins = plugins
Translations = translations
EOF

# -----------------------------------------------------------------------------
# AppRun
# -----------------------------------------------------------------------------

cat > "$APPDIR/AppRun" <<'EOF'
#!/bin/sh
HERE="$(dirname "$(readlink -f "$0")")"

export PATH="$HERE/usr/bin:$PATH"
export LD_LIBRARY_PATH="$HERE/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="$HERE/usr/plugins${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}"
export QT_QPA_PLATFORM_PLUGIN_PATH="$HERE/usr/plugins/platforms${QT_QPA_PLATFORM_PLUGIN_PATH:+:$QT_QPA_PLATFORM_PLUGIN_PATH}"
export XDG_DATA_DIRS="$HERE/usr/share:${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"

# Variables para evitar cursor gigante en Raspberry Pi
export QT_AUTO_SCREEN_SCALE_FACTOR="${QT_AUTO_SCREEN_SCALE_FACTOR:-0}"
export QT_SCALE_FACTOR="${QT_SCALE_FACTOR:-1}"
export XCURSOR_SIZE="${XCURSOR_SIZE:-2}"

# Forzar XCB en lugar de Wayland para mejor compatibilidad
if [ -z "${QT_QPA_PLATFORM:-}" ]; then
    export QT_QPA_PLATFORM=xcb
fi

case "$1" in
    --repl)
        shift
        if [ -x "$HERE/usr/bin/nico" ]; then
            exec "$HERE/usr/bin/nico" "$@"
        fi
        ;;
esac

if [ -x "$HERE/usr/bin/nico-ide" ]; then
    exec "$HERE/usr/bin/nico-ide" "$@"
elif [ -x "$HERE/usr/bin/nico" ]; then
    exec "$HERE/usr/bin/nico" "$@"
fi

echo "Nico: no se encontró nico-ide ni nico dentro del AppImage." >&2
exit 1
EOF

chmod 755 "$APPDIR/AppRun"

# -----------------------------------------------------------------------------
# Icono
# -----------------------------------------------------------------------------

ICON_SRC=""
for f in \
    "$PROJECT_DIR/nico-icon-256.png" \
    "$PROJECT_DIR/nico-icon.png" \
    "$PROJECT_DIR/nico-icon1.png" \
    "$PROJECT_DIR/nico-ide/nico-ide.png" \
    "$PROJECT_DIR/packaging/nico.png"
do
    if [[ -f "$f" ]]; then
        ICON_SRC="$f"
        break
    fi
done

if [[ -n "$ICON_SRC" ]]; then
    install -m644 "$ICON_SRC" "$APPDIR/nico.png"
    install -m644 "$ICON_SRC" "$APPDIR/usr/share/icons/hicolor/256x256/apps/nico.png"
else
    warn "No encontré icono PNG. Generando icono placeholder."
    TMP_ICON="$(mktemp)"
    base64 -d > "$TMP_ICON" <<'PNG'
iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==
PNG
    install -m644 "$TMP_ICON" "$APPDIR/nico.png"
    install -m644 "$TMP_ICON" "$APPDIR/usr/share/icons/hicolor/256x256/apps/nico.png"
    rm -f "$TMP_ICON"
fi

# -----------------------------------------------------------------------------
# Desktop file
# -----------------------------------------------------------------------------

cat > "$APPDIR/nico.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Nico IDE
Comment=IDE para el lenguaje Nico
Exec=nico-ide
Icon=nico
Terminal=false
Categories=Development;Education;
EOF

install -m644 "$APPDIR/nico.desktop" "$APPDIR/usr/share/applications/nico.desktop"

# -----------------------------------------------------------------------------
# Documentación y ejemplos
# -----------------------------------------------------------------------------

if [[ -d "$PROJECT_DIR/docs" ]]; then
    log "Copiando documentación..."
    mkdir -p "$APPDIR/usr/share/nico/docs"
    cp -a "$PROJECT_DIR/docs/." "$APPDIR/usr/share/nico/docs/"
fi

if [[ -d "$PROJECT_DIR/ejemplos" ]]; then
    log "Copiando ejemplos..."
    mkdir -p "$APPDIR/usr/share/nico/ejemplos"
    cp -a "$PROJECT_DIR/ejemplos/." "$APPDIR/usr/share/nico/ejemplos/"
fi

# -----------------------------------------------------------------------------
# Archivos de proyecto (README, LICENSE, CHANGELOG)
# -----------------------------------------------------------------------------

mkdir -p "$APPDIR/usr/share/nico"

for extra in README.md LICENSE CHANGELOG.md; do
    if [[ -f "$PROJECT_DIR/$extra" ]]; then
        log "Copiando $extra..."
        cp "$PROJECT_DIR/$extra" "$APPDIR/usr/share/nico/"
    fi
done

# -----------------------------------------------------------------------------
# Código fuente
# -----------------------------------------------------------------------------

log "Empaquetando código fuente..."
mkdir -p "$APPDIR/usr/share/nico/src"

# Copiar código fuente del intérprete
if [[ -d "$PROJECT_DIR/src" ]]; then
    log "Copiando fuentes del intérprete..."
    cp -a "$PROJECT_DIR/src" "$APPDIR/usr/share/nico/src/nico"
    
    # Copiar archivos de compilación
    for archivo in compile.sh CMakeLists.txt Makefile; do
        if [[ -f "$PROJECT_DIR/$archivo" ]]; then
            cp "$PROJECT_DIR/$archivo" "$APPDIR/usr/share/nico/src/nico/"
        fi
    done
fi

# Copiar código fuente del IDE
if [[ -d "$PROJECT_DIR/nico-ide" ]]; then
    log "Copiando fuentes del IDE..."
    
    # Copiar todo el directorio nico-ide
    cp -a "$PROJECT_DIR/nico-ide" "$APPDIR/usr/share/nico/src/nico-ide"
    
    # Limpiar archivos compilados y el directorio build
    rm -rf "$APPDIR/usr/share/nico/src/nico-ide/build"
    find "$APPDIR/usr/share/nico/src/nico-ide" -name "*.o" -delete 2>/dev/null || true
    find "$APPDIR/usr/share/nico/src/nico-ide" -name "*.so" -delete 2>/dev/null || true
    find "$APPDIR/usr/share/nico/src/nico-ide" -name "*.exe" -delete 2>/dev/null || true
    
    log "Fuentes del IDE copiadas (sin binarios)."
fi

# -----------------------------------------------------------------------------
# Plugins de Qt
# -----------------------------------------------------------------------------

QT_PLUGIN_SRC=""
for d in \
    /usr/lib/x86_64-linux-gnu/qt5/plugins \
    /usr/lib/qt5/plugins
do
    if [[ -d "$d" ]]; then
        QT_PLUGIN_SRC="$d"
        break
    fi
done

if [[ -n "$QT_PLUGIN_SRC" ]]; then
    log "Copiando plugins de Qt desde: $QT_PLUGIN_SRC"

    PLUGIN_CATEGORIES=(
        platforms
        imageformats
        iconengines
        styles
        platformthemes
        platforminputcontexts
        xcbglintegrations
    )

    for category in "${PLUGIN_CATEGORIES[@]}"; do
        if [[ -d "$QT_PLUGIN_SRC/$category" ]]; then
            mkdir -p "$APPDIR/usr/plugins/$category"
            cp -a "$QT_PLUGIN_SRC/$category/." "$APPDIR/usr/plugins/$category/"
            log "Plugins copiados: $category"
        fi
    done
else
    warn "No encontré el directorio de plugins de Qt5."
fi

# -----------------------------------------------------------------------------
# Traducciones de Qt
# -----------------------------------------------------------------------------

QT_TRANS_SRC="/usr/share/qt5/translations"

if [[ -d "$QT_TRANS_SRC" ]]; then
    log "Copiando traducciones de Qt desde: $QT_TRANS_SRC"
    cp -a "$QT_TRANS_SRC/." "$APPDIR/usr/translations/"
else
    warn "No encontré traducciones de Qt5 en $QT_TRANS_SRC."
fi

# -----------------------------------------------------------------------------
# Copiar librerías necesarias
# -----------------------------------------------------------------------------

should_skip_lib() {
    local lib="$1"

    # No incluimos glibc/loader dentro del AppImage.
    case "$lib" in
        *linux-vdso*|*ld-linux*|*/ld-*.so*|*libc.so*|*libdl.so*|*libpthread.so*|*librt.so*|*libm.so*|*libresolv*|*libnss*|*libutil.so*|*libcrypt.so*)
            return 0
            ;;
    esac

    return 1
}

copy_libs_for() {
    local bin="$1"
    local line libpath dest pattern1 pattern2

    pattern1='=>[[:space:]](/[^[:space:]]+)'
    pattern2='^[[:space:]]*(/[^[:space:]]+)'

    ldd "$bin" 2>/dev/null | while IFS= read -r line; do
        libpath=""

        if [[ "$line" =~ $pattern1 ]]; then
            libpath="${BASH_REMATCH[1]}"
        elif [[ "$line" =~ $pattern2 ]]; then
            libpath="${BASH_REMATCH[1]}"
        fi

        [[ -n "$libpath" ]] || continue
        [[ -f "$libpath" ]] || continue

        if should_skip_lib "$libpath"; then
            continue
        fi

        dest="$APPDIR/usr/lib/$(basename "$libpath")"

        if [[ ! -e "$dest" ]]; then
            install -m644 "$libpath" "$dest"
            log "Lib incluida: $(basename "$libpath")"
        fi
    done || true
}

log "Analizando dependencias de binarios y plugins..."

while IFS= read -r -d '' f; do
    copy_libs_for "$f"
done < <(find "$APPDIR/usr/bin" -type f -executable -print0)

while IFS= read -r -d '' f; do
    copy_libs_for "$f"
done < <(find "$APPDIR/usr/plugins" -type f -name '*.so' -print0)

# -----------------------------------------------------------------------------
# Arquitectura
# -----------------------------------------------------------------------------

machine="$(uname -m)"

case "$machine" in
    aarch64)
        APPIMAGE_ARCH="aarch64"
        ;;
    armv7l|armv6l|armv8l)
        APPIMAGE_ARCH="armhf"
        ;;
    x86_64)
        APPIMAGE_ARCH="x86_64"
        ;;
    *)
        die "Arquitectura no soportada: $machine"
        ;;
esac

log "Arquitectura detectada: $machine -> $APPIMAGE_ARCH"

# -----------------------------------------------------------------------------
# appimagetool
# -----------------------------------------------------------------------------

TOOL="$PROJECT_DIR/appimagetool-$APPIMAGE_ARCH.AppImage"

if [[ ! -x "$TOOL" ]]; then
    url="https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-$APPIMAGE_ARCH.AppImage"

    log "Descargando appimagetool..."
    log "$url"

    if command -v wget >/dev/null 2>&1; then
        wget -O "$TOOL" "$url" || die "No pude descargar appimagetool."
    elif command -v curl >/dev/null 2>&1; then
        curl -L -o "$TOOL" "$url" || die "No pude descargar appimagetool."
    else
        die "Necesitás wget o curl para descargar appimagetool."
    fi

    chmod +x "$TOOL"
fi

# -----------------------------------------------------------------------------
# Generar AppImage
# -----------------------------------------------------------------------------

if [[ -z "$OUTPUT" ]]; then
    OUTPUT="Nico-${VERSION}-${APPIMAGE_ARCH}.AppImage"
fi

log "Generando AppImage: $OUTPUT"

if ! ARCH="$APPIMAGE_ARCH" "$TOOL" --appimage-extract-and-run "$APPDIR" "$OUTPUT"; then
    warn "Reintentando sin --appimage-extract-and-run..."
    ARCH="$APPIMAGE_ARCH" "$TOOL" "$APPDIR" "$OUTPUT"
fi

chmod +x "$OUTPUT"

log "✓ AppImage generado: $PROJECT_DIR/$OUTPUT"
log ""
log "Para probarlo:"
log "  ./$OUTPUT"
log "  ./$OUTPUT --repl"
log ""
log "Para instalarlo:"
log "  sudo ./install_appimage.sh $OUTPUT"
