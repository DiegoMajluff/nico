#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/nico-ide"

echo "╔════════════════════════════════════════╗"
echo "║      Compilando Nico IDE para Pi       ║"
echo "╚════════════════════════════════════════╝"
echo ""

echo "Limpiando build anterior..."
rm -rf build

echo "Configurando con CMake..."
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

echo "Compilando..."
# Si tenés poca RAM, cambiá -j2 por -j1
cmake --build build -j2

echo ""
echo "✅ IDE compilado."
echo "   Binario: $SCRIPT_DIR/nico-ide/build/nico-ide"
echo ""
echo "Para probarlo:"
echo "  cd \"$SCRIPT_DIR/nico-ide/build\""
echo "  cp ../../nico ."
echo "  ./nico-ide"
