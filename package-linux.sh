#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

VERSION="2.1.0"
ARCH="x86_64"
PKG="Nico-${VERSION}-linux-${ARCH}"
STAGE="dist-linux/${PKG}"
APPIMAGE="Nico-${VERSION}-${ARCH}.AppImage"

echo "========================================"
echo " Empaquetando Nico ${VERSION} para Linux (${ARCH})"
echo "========================================"

# --- Verificar los 3 elementos que describe el LEEME ---
[[ -f "$APPIMAGE" ]]          || { echo "[ERROR] Falta '$APPIMAGE'. Corré ./appimage-nico-linux.sh"; exit 1; }
[[ -f install_appimage.sh ]]  || { echo "[ERROR] Falta 'install_appimage.sh'"; exit 1; }
[[ -f LEEME.txt ]]            || { echo "[ERROR] Falta 'LEEME.txt' en la raíz"; exit 1; }

# --- Staging limpio ---
rm -rf dist-linux
mkdir -p "${STAGE}"

echo "[1/3] Copiando AppImage..."
install -m755 "$APPIMAGE" "${STAGE}/${APPIMAGE}"

echo "[2/3] Copiando instalador y LEEME..."
install -m755 install_appimage.sh "${STAGE}/install_appimage.sh"
cp LEEME.txt "${STAGE}/LEEME.txt"

echo "[3/3] Comprimiendo en ${PKG}.tar.gz..."
rm -f "${PKG}.tar.gz"
tar -czf "${PKG}.tar.gz" -C dist-linux "${PKG}"

echo ""
echo "========================================"
echo " Paquete listo:"
ls -lh "${PKG}.tar.gz"
echo "========================================"
echo ""
echo "Contenido del tar.gz:"
tar -tzf "${PKG}.tar.gz"
