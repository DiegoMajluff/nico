#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_DIR"

PREFIX="/usr/local"
SKIP_DEPS=0
FORCE_BUILD=0
FORCE_BUILD_IDE=0

NICOC_BIN="$PROJECT_DIR/nico"
IDE_BIN_PATH="$PROJECT_DIR/nico-ide/build/nico-ide"

log() {
    echo "[nico-installer] $*"
}

warn() {
    echo "[nico-installer][WARN] $*" >&2
}

die() {
    echo "[nico-installer][ERROR] $*" >&2
    exit 1
}

usage() {
    cat <<'USAGE'
Instalador de Nico + Nico IDE para Raspberry Pi aarch64

Uso:
  sudo ./instalar-nico-rpi.sh [opciones]

Opciones:
  --prefix DIR       Prefijo de instalación (por defecto: /usr/local)
  --skip-deps        No instalar dependencias con apt
  --build            Forzar compilación del intérprete con ./compile.sh
  --build-ide        Forzar compilación del IDE con CMake
  -h, --help         Mostrar ayuda

Ejemplos:
  sudo ./instalar-nico-rpi.sh --skip-deps
  sudo ./instalar-nico-rpi.sh --build --build-ide
  ./instalar-nico-rpi.sh --prefix "$HOME/.local" --skip-deps
USAGE
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --prefix)
            PREFIX="${2:?}"
            shift 2
            ;;
        --skip-deps)
            SKIP_DEPS=1
            shift
            ;;
        --build)
            FORCE_BUILD=1
            shift
            ;;
        --build-ide)
            FORCE_BUILD_IDE=1
            shift
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

PREFIX="${PREFIX%/}"
[[ -n "$PREFIX" ]] || die "PREFIX vacío"

log "Directorio del proyecto: $PROJECT_DIR"
log "Prefijo de instalación: $PREFIX"

case "$PREFIX" in
    /usr*|/opt*)
        if [[ "$(id -u)" -ne 0 ]]; then
            die "Para instalar en $PREFIX ejecutá con sudo, o usá --prefix \"\$HOME/.local\""
        fi
        ;;
esac

# -----------------------------------------------------------------------------
# Dependencias
# -----------------------------------------------------------------------------

if [[ "$SKIP_DEPS" -eq 0 ]]; then
    if command -v apt-get >/dev/null 2>&1; then
        SUDO=""
        if [[ "$(id -u)" -ne 0 ]]; then
            SUDO="sudo"
        fi

        log "Actualizando paquetes..."
        $SUDO apt-get update

        log "Instalando dependencias..."
        $SUDO apt-get install -y \
            build-essential \
            cmake \
            pkg-config \
            libreadline-dev \
            libsqlite3-dev \
            libgpiod-dev \
            qtbase5-dev \
            libqt5svg5-dev
    else
        warn "No encontré apt-get. Instalá manualmente las dependencias."
    fi
else
    log "Omitiendo instalación de dependencias."
fi

# -----------------------------------------------------------------------------
# Compilación del intérprete si hace falta
# -----------------------------------------------------------------------------

if [[ ! -x "$NICOC_BIN" || "$FORCE_BUILD" -eq 1 ]]; then
    if [[ ! -f "$PROJECT_DIR/compile.sh" ]]; then
        die "No existe ./nico ni ./compile.sh"
    fi

    log "Compilando intérprete Nico con ./compile.sh..."
    chmod +x "$PROJECT_DIR/compile.sh" 2>/dev/null || true
    "$PROJECT_DIR/compile.sh"
fi

[[ -x "$NICOC_BIN" ]] || die "No existe el intérprete compilado: $NICOC_BIN"

# -----------------------------------------------------------------------------
# Compilación del IDE si hace falta
# -----------------------------------------------------------------------------

if [[ ! -x "$IDE_BIN_PATH" || "$FORCE_BUILD_IDE" -eq 1 ]]; then
    if [[ ! -f "$PROJECT_DIR/nico-ide/CMakeLists.txt" ]]; then
        die "No existe nico-ide/CMakeLists.txt"
    fi

    log "Compilando Nico IDE con CMake..."

    cmake -S "$PROJECT_DIR/nico-ide" \
          -B "$PROJECT_DIR/nico-ide/build" \
          -DCMAKE_BUILD_TYPE=Release

    cmake --build "$PROJECT_DIR/nico-ide/build" -j2
fi

[[ -x "$IDE_BIN_PATH" ]] || die "No existe el IDE compilado: $IDE_BIN_PATH"

# -----------------------------------------------------------------------------
# Directorios de instalación
# -----------------------------------------------------------------------------

BIN_DIR="$PREFIX/bin"
APPS_DIR="$PREFIX/share/applications"
ICON_DIR="$PREFIX/share/icons/hicolor/256x256/apps"
STATE_DIR="$PREFIX/share/nico"
MANIFEST="$STATE_DIR/installed-files.txt"

install -d "$BIN_DIR"
install -d "$STATE_DIR"

: > "$MANIFEST"

add_manifest() {
    printf '%s\n' "$1" >> "$MANIFEST"
}

# -----------------------------------------------------------------------------
# Binarios
# -----------------------------------------------------------------------------

install -m755 "$NICOC_BIN" "$BIN_DIR/nico"
add_manifest "$BIN_DIR/nico"
log "Instalado: $BIN_DIR/nico"

install -m755 "$IDE_BIN_PATH" "$BIN_DIR/nico-ide.bin"
add_manifest "$BIN_DIR/nico-ide.bin"
log "Instalado: $BIN_DIR/nico-ide.bin"

# Wrapper para el IDE, con corrección de cursor/escala
cat > "$BIN_DIR/nico-ide" <<EOF
#!/bin/sh
export QT_AUTO_SCREEN_SCALE_FACTOR="\${QT_AUTO_SCREEN_SCALE_FACTOR:-0}"
export QT_SCALE_FACTOR="\${QT_SCALE_FACTOR:-1}"
export XCURSOR_SIZE="\${XCURSOR_SIZE:-2}"
exec "$BIN_DIR/nico-ide.bin" "\$@"
EOF

chmod 755 "$BIN_DIR/nico-ide"
add_manifest "$BIN_DIR/nico-ide"
log "Wrapper instalado: $BIN_DIR/nico-ide"

# -----------------------------------------------------------------------------
# Icono
# -----------------------------------------------------------------------------

ICON_SRC=""
for f in \
    "$PROJECT_DIR/nico-icon-256.png" \
    "$PROJECT_DIR/nico-icon.png" \
    "$PROJECT_DIR/nico-icon1.png" \
    "$PROJECT_DIR/nico-ide.png" \
    "$PROJECT_DIR/Nico.AppDir/nico-ide.png" \
    "$PROJECT_DIR/packaging/nico.png"
do
    if [[ -f "$f" ]]; then
        ICON_SRC="$f"
        break
    fi
done

ICON_DEST="$ICON_DIR/nico.png"
install -d "$ICON_DIR"

if [[ -n "$ICON_SRC" ]]; then
    install -m644 "$ICON_SRC" "$ICON_DEST"
    log "Icono instalado: $ICON_DEST"
else
    warn "No encontré un icono PNG. Generando icono placeholder."
    TMP_ICON="$(mktemp)"
    base64 -d > "$TMP_ICON" <<'PNG'
iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==
PNG
    install -m644 "$TMP_ICON" "$ICON_DEST"
    rm -f "$TMP_ICON"
fi

add_manifest "$ICON_DEST"

# -----------------------------------------------------------------------------
# Entradas de escritorio
# -----------------------------------------------------------------------------

install -d "$APPS_DIR"

TMP_DESKTOP="$(mktemp)"

cat > "$TMP_DESKTOP" <<EOF
[Desktop Entry]
Type=Application
Name=Nico IDE
Comment=IDE para el lenguaje Nico
Exec=$BIN_DIR/nico-ide
Icon=nico
Terminal=false
Categories=Development;Education;
EOF

install -m644 "$TMP_DESKTOP" "$APPS_DIR/nico-ide.desktop"
add_manifest "$APPS_DIR/nico-ide.desktop"
rm -f "$TMP_DESKTOP"

TMP_DESKTOP="$(mktemp)"

cat > "$TMP_DESKTOP" <<EOF
[Desktop Entry]
Type=Application
Name=Nico REPL
Comment=Intérprete interactivo Nico
Exec=$BIN_DIR/nico
Icon=nico
Terminal=true
Categories=Development;Education;ConsoleOnly;
EOF

install -m644 "$TMP_DESKTOP" "$APPS_DIR/nico-repl.desktop"
add_manifest "$APPS_DIR/nico-repl.desktop"
rm -f "$TMP_DESKTOP"

# -----------------------------------------------------------------------------
# Desinstalador
# -----------------------------------------------------------------------------

UNINSTALLER="$BIN_DIR/nico-uninstall"

cat > "$UNINSTALLER" <<EOF
#!/bin/sh
set -e

MANIFEST="$MANIFEST"

if [ -f "\$MANIFEST" ]; then
    while IFS= read -r f; do
        [ -n "\$f" ] && rm -f "\$f" 2>/dev/null || true
    done < "\$MANIFEST"

    rm -f "\$MANIFEST"
fi

rmdir "$STATE_DIR" 2>/dev/null || true
rm -f "$UNINSTALLER"

echo "Nico desinstalado."
EOF

chmod 755 "$UNINSTALLER"

# -----------------------------------------------------------------------------
# Cachés de escritorio/iconos
# -----------------------------------------------------------------------------

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$APPS_DIR" 2>/dev/null || true
fi

if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f "$PREFIX/share/icons/hicolor" 2>/dev/null || true
fi

log "Instalación completada."
log "Desinstalador: $UNINSTALLER"
