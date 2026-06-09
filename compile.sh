#!/bin/bash
# compile.sh - Script de compilación estándar para Linux y Raspberry Pi.
set -e  # Salir inmediatamente si algún comando falla

echo "╔════════════════════════════════════════╗"
echo "║   Compilando Nico v1.1.1               ║"
echo "╚════════════════════════════════════════╝"
echo ""

# Limpieza previa: evitar ejecutar binario viejo si falla compilación
rm -f nico

# Lista de archivos fuente (actualizada a src/)
SOURCES="src/main.c src/io.c src/expressions.c src/declaraciones.c src/funciones.c src/cadenas.c src/etiquetas.c src/flow.c src/nico_bits.c src/validador.c src/runtime.c src/scopes.c src/leertecla.c src/bd.c src/web.c"

# Flags base (consistentes en todas las plataformas)
CFLAGS="-std=c11 -Wall -Wextra -O2 -Isrc -Wno-stringop-truncation -Wno-unused-result"
LIBS="-lm -lsqlite3 -lpthread"

# Detectar si estamos en Raspberry Pi
if [ -f /proc/device-tree/model ]; then
    MODEL=$(tr -d '\0' < /proc/device-tree/model)
    if [[ "$MODEL" == *"Raspberry Pi"* ]]; then
        echo "🍓 Detectado: Raspberry Pi"
        echo "   Compilando con soporte GPIO (libgpiod)..."
        gcc $CFLAGS -DHAVE_LIBGPIOD -o nico $SOURCES src/nico_gpio.c $LIBS -lgpiod
    else
        echo "🖥️  Detectado: $MODEL"
        echo "   Compilando sin soporte GPIO..."
        gcc $CFLAGS -o nico $SOURCES src/nico_gpio_stub.c $LIBS
    fi
else
    echo "🖥️  Detectado: PC Linux (no Raspberry Pi)"
    echo "   Compilando sin soporte GPIO..."
    gcc $CFLAGS -o nico $SOURCES src/nico_gpio_stub.c $LIBS
fi

# Verificar si compiló correctamente
if [ $? -eq 0 ]; then
    echo ""
    echo "✅ ¡Compilación exitosa!"
    echo "   Ejecutable: ./nico"
    echo ""
    echo "   Para ejecutar: ./nico programa.nico"
    echo "   Para modo interactivo: ./nico"
    echo ""
    echo "   💡 Tip: Usa -DDEBUG para ver logs internos:"
    echo "      gcc -DDEBUG ... (o edita este script)"
else
    echo ""
    echo "❌ Error de compilación. Revisá los mensajes arriba."
    exit 1
fi
