#!/bin/bash
# ============================================================================
# Nico v2.1.0 - Desinstalador para Linux
# ============================================================================
set -e

if [ "$EUID" -ne 0 ]; then 
    echo -e "\e[31m[ERROR]\e[0m Este script debe ejecutarse con sudo"
    echo "Uso: sudo ./uninstall.sh"
    exit 1
fi

echo -e "\e[34m[INFO]\e[0m Desinstalando Nico v2.1.0..."

rm -f /usr/local/bin/nico
rm -f /usr/local/bin/nico-ide
rm -f /usr/share/icons/hicolor/256x256/apps/nico-ide.png
rm -f /usr/share/applications/nico-ide.desktop
rm -rf /usr/local/share/nico

if command -v update-desktop-database &> /dev/null; then
    update-desktop-database /usr/share/applications
fi
if command -v gtk-update-icon-cache &> /dev/null; then
    gtk-update-icon-cache -f /usr/share/icons/hicolor
fi

echo -e "\e[32m[OK]\e[0m Nico v2.1.0 ha sido desinstalado del sistema."