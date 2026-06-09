# 🐧 Nico v1.1.1 - Intérprete Educativo de Scripting en Español

Nico es un lenguaje de programación minimalista, interpretado y 100% en español, diseñado para aprender conceptos fundamentales de programación sin barreras de idioma ni complejidad innecesaria.

## ✨ Características Principales
- **Sintaxis en español**: `SI`, `MIENTRAS`, `FUNCION`, `BLOQUE PRINCIPAL`, `MOSTRAR`, `ASIGNAR EN`, etc.
- **Scopes funcionales**: Variables locales con shadowing real y aislamiento completo entre funciones.
- **14 tipos de datos nativos**: Enteros, decimales, caracteres, texto, y variantes `SIN SIGNO`, en `VARIABLE`, `LISTA` y `MATRIZ`.
- **Seguridad de tipos**: Pools separados, validación de índices y gestión automática de memoria para colecciones.
- **E/S completa**: `ESCRIBIR`, `LEER`, manejo de archivos, colores, cursor y efectos de terminal.
- **Biblioteca nativa**: Funciones matemáticas (`SENO`, `RAIZ`, `POTENCIA`) y de texto (`LONGITUDTEXTO`, `COPIARTEXTO`, etc.).
- **Cross-platform**: Compila y corre en Linux (incluyendo Raspberry Pi) y Windows sin dependencias externas.

## 🚀 Inicio Rápido
### Compilar
**Linux / macOS:**
`chmod +x compile.sh && ./compile.sh`
**Windows:** `compile_windows.bat`
**Raspberry Pi:** `./compile.sh` (detecta y habilita GPIO automáticamente)

### Ejecutar
`./nico ejemplo.nico`
**Modo interactivo:** `./nico`

## 📖 Ejemplo de Uso
    PROGRAMA HolaNico
        VARIABLE TEXTO $nombre = "Mundo"
        VARIABLE DECIMAL $precio = 19.99
        VARIABLE ENTERA $cantidad = 3
    BLOQUE PRINCIPAL
        CALCULAR EN $total = $precio * $cantidad
        MOSTRAR("¡Hola, $nombre!") SALTO
        MOSTRAR("Total: $total", DECIMALES(2)) SALTO
    FIN PRINCIPAL
    FINAL

## 🧠 Sistema de Tipos
Nico usa 14 tipos explícitos en 3 categorías:
**Escalares:** `ENTERA`, `DECIMAL`, `CARACTER`, y sus variantes `SIN SIGNO`, más `TEXTO`.
**Listas:** `LISTA ENTERA`, `LISTA DECIMAL`, `LISTA TEXTO`, etc. (arrays 1D).
**Matrices:** `MATRIZ ENTERA`, `MATRIZ DECIMAL`, etc. (arrays 2D).

## 🔒 Scopes y Shadowing
Cada función crea un entorno local que oculta variables globales con el mismo nombre. Al retornar, el contexto global se restaura intacto.
    PROGRAMA Test
        VARIABLE ENTERA $x = 100
        VARIABLE ENTERA $asignar = 0
        FUNCION ENTERA demo()
            VARIABLE ENTERA $x = 50
            MOSTRAR("Local: $x") SALTO
            RETORNAR $x
        FIN FUNCION
        BLOQUE PRINCIPAL
            MOSTRAR("Global: $x") SALTO
            ASIGNAR EN $asignar = demo()
            MOSTRAR("Global después: $x") SALTO
        FIN PRINCIPAL
    FINAL

## 📝 Filosofía de Asignación
| Tipo | Sintaxis | Razón |
|------|----------|-------|
| Escalares | `ASIGNAR EN $var = valor` | Claridad semántica (asignar vs calcular) |
| Colecciones | `$lista[indice] = valor` | La indexación ya implica mutación |
| Texto | `COPIARTEXTO($txt, "..."` | Copia explícita para evitar efectos secundarios |

## 📌 Interpolación y DECIMALES()
    MOSTRAR("Precio: $precio", DECIMALES(2))
    MOSTRAR("A y B: $a y $b", DECIMALES(2, 4))
⚠️ **Corchetes literales:** Si querés imprimir `[ ]` sin que se interpreten como expresión, escapalos con `\`: `MOSTRAR("d\[0\]\[0\] = $d[0][0]", DECIMALES(2))`

## 🛠️ Estructura y Compilación
    nico/
    ├── src/       # Código fuente C (.h / .c)
    ├── ejemplos/  # Scripts .nico de prueba
    ├── bin/       # Directorio de salida de compilación (nico.exe y librerías para Windows)     
    ├── compile.sh # Build Linux/RPi
    ├── compile_windows.bat
    ├── README.md
    └── CHANGELOG.md

## 📄 Licencia y Contribuciones
MIT License. Contribuciones bienvenidas vía Issues/PRs.
**Créditos:** Diseño/Arquitectura/Supervisión: Diego Alejandro Majluff | Implementación: Qwen (Alibaba Cloud)