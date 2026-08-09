#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "╔════════════════════════════════════════════╗"
echo "║   Compilando Nico IDE para Linux (nativo)  ║"
echo "╚════════════════════════════════════════════╝"
echo ""

# --- Verificación de dependencias ---
if ! command -v g++ >/dev/null 2>&1; then
    echo "❌ g++ no encontrado. Instalar: sudo apt install build-essential"
    exit 1
fi
if ! command -v cmake >/dev/null 2>&1; then
    echo "❌ cmake no encontrado. Instalar: sudo apt install cmake"
    exit 1
fi
if ! command -v pkg-config >/dev/null 2>&1; then
    echo "❌ pkg-config no encontrado. Instalar: sudo apt install pkg-config"
    exit 1
fi
if ! pkg-config --exists Qt5Widgets; then
    echo "❌ Qt5 no encontrado. Instalar: sudo apt install qtbase5-dev"
    exit 1
fi
echo "✅ Dependencias OK (g++, cmake, Qt5)."
echo ""

cd "$SCRIPT_DIR/nico-ide"

echo "Limpiando build anterior..."
rm -rf build

echo "Configurando con CMake..."
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

echo "Compilando con $(nproc) núcleos..."
cmake --build build -j"$(nproc)"

echo ""
echo "✅ IDE compilado."
echo "   Binario: $SCRIPT_DIR/nico-ide/build/nico-ide"
echo ""
echo "Para probarlo:"
echo "  cd \"$SCRIPT_DIR/nico-ide/build\""
echo "  cp ../../nico ."
echo "  ./nico-ide"
