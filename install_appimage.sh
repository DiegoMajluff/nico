#!/usr/bin/env bash
set -euo pipefail

log()  { echo "[nico-installer] $*"; }
warn() { echo "[nico-installer][WARN] $*" >&2; }
die()  { echo "[nico-installer][ERROR] $*" >&2; exit 1; }

usage() {
    cat <<'USAGE'
Instalador de AppImage para Nico

Uso:
  sudo ./install_appimage.sh <archivo.AppImage>

Ejemplos:
  sudo ./install_appimage.sh Nico-2.1.0-aarch64.AppImage
  sudo ./install_appimage.sh Nico-2.1.0-x86_64.AppImage
USAGE
}

[[ $# -ne 1 ]] && { usage; exit 1; }

APPIMAGE="$1"
[[ -f "$APPIMAGE" ]] || die "No existe el archivo: $APPIMAGE"
[[ -x "$APPIMAGE" ]] || die "El archivo no es ejecutable. Ejecutá: chmod +x $APPIMAGE"
[[ "$(id -u)" -ne 0 ]] && die "Este script debe ejecutarse con sudo"

log "Instalando AppImage: $APPIMAGE"

# Detectar usuario real (no root)
REAL_USER="${SUDO_USER:-$USER}"
REAL_HOME=$(eval echo "~$REAL_USER")

# Extraer versión del nombre del AppImage (ej: Nico-2.1.0-x86_64.AppImage -> 2.1.0)
APPIMAGE_BASENAME="$(basename "$APPIMAGE")"
VERSION=$(echo "$APPIMAGE_BASENAME" | grep -oP '\d+\.\d+\.\d+' | head -1)
if [[ -z "$VERSION" ]]; then
    VERSION="2.1.0"
    warn "No pude detectar la versión del nombre del archivo, usando $VERSION"
fi

NICO_HOME="$REAL_HOME/nico-v${VERSION}"
INSTALL_DIR="/opt/nico"
BIN_DIR="/usr/local/bin"
APPS_DIR="/usr/local/share/applications"
ICON_DIR="/usr/local/share/icons/hicolor/256x256/apps"

# -----------------------------------------------------------------------------
# Instalar AppImage y binarios
# -----------------------------------------------------------------------------

install -d "$INSTALL_DIR"
APPIMAGE_NAME="$(basename "$APPIMAGE")"
install -m755 "$APPIMAGE" "$INSTALL_DIR/$APPIMAGE_NAME"
log "AppImage copiado a: $INSTALL_DIR/$APPIMAGE_NAME"

# Extraer AppImage para obtener icono, desktop, docs, ejemplos y código fuente
TEMP_DIR="$(mktemp -d)"
cd "$TEMP_DIR"

if ! "$INSTALL_DIR/$APPIMAGE_NAME" --appimage-extract > /dev/null 2>&1; then
    warn "No pude extraer el AppImage, continuando sin recursos adicionales"
    ICON_SRC=""
    DESKTOP_FILE=""
    HAS_DOCS=0
    HAS_EJEMPLOS=0
    HAS_SRC=0
else
    # Buscar ícono específico de Nico primero, luego cualquier PNG como fallback
    ICON_SRC="$(find squashfs-root -name 'nico*.png' 2>/dev/null | head -n 1 || true)"
    if [[ -z "$ICON_SRC" ]]; then
        ICON_SRC="$(find squashfs-root -name '*.png' 2>/dev/null | head -n 1 || true)"
    fi
    DESKTOP_FILE="$(find squashfs-root -name '*.desktop' 2>/dev/null | head -n 1 || true)"
    
    HAS_DOCS=0
    HAS_EJEMPLOS=0
    HAS_SRC=0
    [[ -d "squashfs-root/usr/share/nico/docs" ]] && HAS_DOCS=1
    [[ -d "squashfs-root/usr/share/nico/ejemplos" ]] && HAS_EJEMPLOS=1
    [[ -d "squashfs-root/usr/share/nico/src" ]] && HAS_SRC=1
fi

cd - > /dev/null

# -----------------------------------------------------------------------------
# Instalar documentación, ejemplos y código fuente en el home del usuario
# -----------------------------------------------------------------------------

if [[ "$HAS_DOCS" -eq 1 || "$HAS_EJEMPLOS" -eq 1 || "$HAS_SRC" -eq 1 ]]; then
    log "Creando $NICO_HOME ..."
    mkdir -p "$NICO_HOME"

    if [[ "$HAS_DOCS" -eq 1 ]]; then
        cp -a "$TEMP_DIR/squashfs-root/usr/share/nico/docs" "$NICO_HOME/"
        log "Documentación instalada en: $NICO_HOME/docs/"
    fi

    if [[ "$HAS_EJEMPLOS" -eq 1 ]]; then
        cp -a "$TEMP_DIR/squashfs-root/usr/share/nico/ejemplos" "$NICO_HOME/"
        log "Ejemplos instalados en: $NICO_HOME/ejemplos/"
    fi

    if [[ "$HAS_SRC" -eq 1 ]]; then
        cp -a "$TEMP_DIR/squashfs-root/usr/share/nico/src" "$NICO_HOME/"
        log "Código fuente instalado en: $NICO_HOME/src/"
    fi

    # Copiar LICENSE, README y CHANGELOG si están en el AppImage
    for extra in LICENSE README.md CHANGELOG.md; do
        if [[ -f "$TEMP_DIR/squashfs-root/usr/share/nico/$extra" ]]; then
            cp "$TEMP_DIR/squashfs-root/usr/share/nico/$extra" "$NICO_HOME/"
        fi
    done

    chown -R "$REAL_USER:$REAL_USER" "$NICO_HOME"
    log "Recursos de usuario listos en: $NICO_HOME"
else
    warn "No se encontraron docs, ejemplos ni código fuente dentro del AppImage"
fi

# -----------------------------------------------------------------------------
# Instalar icono
# -----------------------------------------------------------------------------

if [[ -n "$ICON_SRC" && -f "$TEMP_DIR/$ICON_SRC" ]]; then
    install -d "$ICON_DIR"
    install -m644 "$TEMP_DIR/$ICON_SRC" "$ICON_DIR/nico.png"
    log "Icono instalado: $ICON_DIR/nico.png"
else
    warn "No se encontró icono en el AppImage"
fi

# -----------------------------------------------------------------------------
# Crear comandos nico y nico-ide
# -----------------------------------------------------------------------------

cat > "$BIN_DIR/nico" <<EOF
#!/bin/sh
export QT_AUTO_SCREEN_SCALE_FACTOR="\${QT_AUTO_SCREEN_SCALE_FACTOR:-0}"
export QT_SCALE_FACTOR="\${QT_SCALE_FACTOR:-1}"
export XCURSOR_SIZE="\${XCURSOR_SIZE:-2}"
exec "$INSTALL_DIR/$APPIMAGE_NAME" --repl "\$@"
EOF
chmod 755 "$BIN_DIR/nico"
log "Comando creado: $BIN_DIR/nico"

cat > "$BIN_DIR/nico-ide" <<EOF
#!/bin/sh
export QT_AUTO_SCREEN_SCALE_FACTOR="\${QT_AUTO_SCREEN_SCALE_FACTOR:-0}"
export QT_SCALE_FACTOR="\${QT_SCALE_FACTOR:-1}"
export XCURSOR_SIZE="\${XCURSOR_SIZE:-2}"
exec "$INSTALL_DIR/$APPIMAGE_NAME" "\$@"
EOF
chmod 755 "$BIN_DIR/nico-ide"
log "Comando creado: $BIN_DIR/nico-ide"

# -----------------------------------------------------------------------------
# Desktop files
# -----------------------------------------------------------------------------

install -d "$APPS_DIR"

cat > "$APPS_DIR/nico-ide.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Nico IDE
Comment=IDE para el lenguaje Nico
Exec=$BIN_DIR/nico-ide
Icon=nico
Terminal=false
Categories=Development;Education;
EOF

cat > "$APPS_DIR/nico-repl.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Nico REPL
Comment=Intérprete interactivo Nico
Exec=$BIN_DIR/nico
Icon=nico
Terminal=true
Categories=Development;Education;ConsoleOnly;
EOF

log "Desktop files instalados"

# -----------------------------------------------------------------------------
# Desinstalador
# -----------------------------------------------------------------------------

cat > "$BIN_DIR/nico-uninstall" <<EOF
#!/bin/sh
set -e

echo "Desinstalando Nico..."

rm -f "$BIN_DIR/nico"
rm -f "$BIN_DIR/nico-ide"
rm -f "$BIN_DIR/nico-uninstall"
rm -f "$APPS_DIR/nico-ide.desktop"
rm -f "$APPS_DIR/nico-repl.desktop"
rm -f "$ICON_DIR/nico.png"
rm -rf "$INSTALL_DIR"

echo ""
echo "Nota: la carpeta $NICO_HOME no se eliminó."
echo "Si querés borrar también los ejemplos, documentación y código fuente:"
echo "  rm -rf $NICO_HOME"
echo ""
echo "Nico desinstalado correctamente."
EOF
chmod 755 "$BIN_DIR/nico-uninstall"
log "Desinstalador creado: $BIN_DIR/nico-uninstall"

# -----------------------------------------------------------------------------
# Actualizar cachés y limpiar
# -----------------------------------------------------------------------------

if command -v update-desktop-database > /dev/null 2>&1; then
    update-desktop-database "$APPS_DIR" 2>/dev/null || true
fi

if command -v gtk-update-icon-cache > /dev/null 2>&1; then
    gtk-update-icon-cache -f "$ICON_DIR/.." 2>/dev/null || true
fi

rm -rf "$TEMP_DIR"

log ""
log "=========================================="
log "  Instalación completada"
log "=========================================="
log ""
log "Comandos disponibles:"
log "  nico            Intérprete REPL"
log "  nico-ide        IDE gráfico"
log "  nico-uninstall  Desinstalar Nico"
log ""
log "Recursos instalados:"
log "  $NICO_HOME/ejemplos/   Ejemplos de código"
log "  $NICO_HOME/docs/       Documentación"
log "  $NICO_HOME/src/        Código fuente"
log ""
log "Probá con:"
log "  nico $NICO_HOME/ejemplos/basicos/01_hola_mundo.nico"
log "  nico-ide"
