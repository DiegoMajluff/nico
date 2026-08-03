# 🐧 Nico v2.1.0 - Intérprete Educativo de Scripting en Español

Nico es un lenguaje de programación interpretado y 100% en español, diseñado para aprender conceptos fundamentales de programación sin barreras de idioma. Con un solo binario y sin dependencias externas, incluye soporte nativo para hardware (GPIO/PWM), bases de datos SQLite, servidor web integrado, gráficos ASCII y un **motor lógico estilo Prolog**.

## ✨ Características Principales

- **Sintaxis en español**: `SI`, `MIENTRAS`, `FUNCION`, `BLOQUE PRINCIPAL`, `ESCRIBIR`, `CALCULAR EN`, etc.
- **Arquitectura AST**: Parser y evaluador basado en árbol de sintaxis abstracta.
- **Funciones y subprogramas**: Soporte completo con scopes locales aislados.
- **Sistema de inclusión**: `INCLUIR "archivo.nico"` para modularizar código.
- **15 tipos de datos nativos**: Enteros, decimales, caracteres, texto, logica y variantes `SIN SIGNO`, en `VARIABLE`, `LISTA`, `MATRIZ (2D y 3D)`.
- **Motor lógico (Prolog)**: Unificación, backtracking, corte, negación, listas con patrones `[H|T]`, y built-ins.
- **GPIO y PWM para Raspberry Pi**: Control nativo de pines digitales y señales PWM.
- **Base de datos SQLite**: Consultas SQL estándar directamente desde Nico.
- **Servidor web integrado**: API REST para consultar bases de datos.
- **Gráficos ASCII**: Primitivas de dibujo (`DIBUJARLINEA`, `DIBUJARCIRCULO`, `RELLENARRECTANGULO`).
- **Operaciones bit-a-bit**: Funciones completas (`BITY`, `BITO`, `BITXOR`, `LEERBIT`, etc.).
- **Manejo de errores**: Bloques `INTENTAR` / `ATRAPAR` con variables de contexto (`$ERROR`, `$LINEA_ERROR`, `$CODIGO_ERROR`).
- **Cross-platform**: Compila y corre en Linux (incluyendo Raspberry Pi) y Windows.

## 🚀 Inicio Rápido

### Compilar

**Linux:**
```bash
chmod +x compile.sh && ./compile.sh
```

**Windows (MSYS2):**
```cmd
compile_windows.bat
```

**Raspberry Pi:** `./compile.sh` (detecta y habilita GPIO automáticamente)

### Ejecutar

```bash
./nico ejemplo.nico
```

**Modo interactivo:** `./nico`

### Instalación automática (Linux)

```bash
sudo ./install.sh
```

El instalador detecta tu distribución, instala dependencias, compila todo y deja `nico` y `nico-ide` disponibles globalmente.

### Raspberry Pi (aarch64)
Descargá el paquete `Nico-2.1.0-rpi-aarch64.zip` desde [Releases](https://github.com/DiegoMajluff/nico/releases), extraé el contenido y seguí las instrucciones del archivo `LEEME.txt` incluido.

En resumen:
chmod +x Nico-2.1.0-aarch64.AppImage
sudo ./install_appimage.sh Nico-2.1.0-aarch64.AppImage

Esto instalará `nico` y `nico-ide` globalmente, creará accesos en el menú de aplicaciones y dejará ejemplos y documentación en `~/nico-v2.1.0/`.

Para usar GPIO sin sudo:
sudo usermod -a -G gpio $USER

## 📖 Ejemplo de Uso

```nico
PROGRAMA HolaNico
    VARIABLE TEXTO $nombre = "Mundo"
    VARIABLE DECIMAL $precio = 19.99
    VARIABLE ENTERA $cantidad = 3
BLOQUE PRINCIPAL
    CALCULAR EN $total = $precio * $cantidad
    ESCRIBIR("¡Hola, $nombre!") SALTO
    ESCRIBIR("Total: $total") SALTO
FIN PRINCIPAL
FINAL
```

## 🧠 Motor Lógico (Estilo Prolog)

Nico v2.1.0 incluye un **motor de inferencia lógica completo** integrado al lenguaje imperativo, permitiendo combinar programación lógica con imperativa en un mismo programa.

### Comandos del motor lógico

| Comando | Descripción |
|---------|-------------|
| `AGREGARHECHO("predicado(args)")` | Agrega un hecho a la base de conocimiento |
| `AGREGARREGLA("cabeza SI cuerpo")` | Agrega una regla (también acepta `:-`) |
| `CONSULTAR("predicado(args)")` | Realiza una consulta |
| `LIMPIARLOGICA` | Limpia la base de conocimiento |
| `LISTARHECHOS` / `LISTARREGLAS` | Muestra la base de conocimiento |
| `BUSCARTODOS("variable", "consulta", $resultado)` | Recolecta todas las soluciones en una lista |

### Built-ins del motor lógico

| Built-in | Descripción |
|----------|-------------|
| `MIEMBRO(X, Lista)` | Verifica si X es miembro de Lista |
| `LONGITUD(Lista, N)` | Calcula la longitud de una lista |
| `PRIMER(Lista, X)` | Obtiene el primer elemento |
| `ULTIMO(Lista, X)` | Obtiene el último elemento |
| `JUNTAR(L1, L2, L3)` | Concatenación bidireccional (estilo `APPEND`) |
| `REVERSA(L1, L2)` | Invierte una lista (estilo `REVERSE`) |
| `=` | Unificación directa |

### Características avanzadas

- **Unificación con Occurs Check**: Evita unificaciones circulares (`X = [X]`)
- **Listas y patrones**: `[a, b, c]`, `[H|T]`, `[[H|T]|R]`
- **Backtracking automático**: Explora todas las soluciones
- **Operador de corte** `!`: Controla el backtracking
- **Negación por fallo** `NO`: `NO padre(X, Y)`
- **Disyunción** `O` / `;` en reglas
- **Aritmética completa**: `+`, `-`, `*`, `/`, `%`, `^`, `MOD`
- **Comparadores**: `MAYOR`, `MENOR`, `IGUAL`, `DIFERENTE`, `ES`

### Ejemplo: Árbol genealógico

```nico
PROGRAMA Familia
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    AGREGARHECHO("padre(juan, pedro)")
    AGREGARHECHO("padre(juan, ana)")
    AGREGARHECHO("padre(pedro, luis)")
    
    AGREGARREGLA("abuelo(X, Y) SI padre(X, Z), padre(Z, Y)")
    
    // Consultar abuelos
    CONSULTAR("abuelo(X, Y)")
    
    // Recolectar todos los abuelos
    BUSCARTODOS("X", "abuelo(X, Y)", $abuelos)
    ESCRIBIR("Abuelos: $abuelos") SALTO
FIN PRINCIPAL
FINAL
```

### Ejemplo: Manipulación de listas

```nico
PROGRAMA Listas
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    // JUNTAR es bidireccional (como APPEND en Prolog)
    CONSULTAR("juntar([1,2], [3,4], X)")    // X = [1,2,3,4]
    CONSULTAR("juntar(X, Y, [1,2,3])")      // Genera todas las divisiones
    
    // Invertir lista
    CONSULTAR("reversa([1,2,3], X)")        // X = [3,2,1]
    
    // Miembro
    CONSULTAR("miembro(X, [a,b,c])")        // X = a, X = b, X = c
FIN PRINCIPAL
FINAL
```

## 🧠 Sistema de Tipos

Nico usa 15 tipos explícitos en 3 categorías:

- **Escalares**: `ENTERA`, `DECIMAL`, `CARACTER`, `LOGICA`, `TEXTO`, y sus variantes `SIN SIGNO`.
- **Listas**: `LISTA ENTERA`, `LISTA DECIMAL`, `LISTA LOGICA`, `LISTA TEXTO`, etc. (arrays 1D).
- **Matrices**: `MATRIZ ENTERA`, `MATRIZ DECIMAL`, `MATRIZ LOGICA`, etc. (arrays 2D y 3D).

## 📌 Filosofía de Asignación

| Tipo | Sintaxis | Razón |
|------|----------|-------|
| Escalares | `CALCULAR EN $var = expresión` | Claridad semántica para cálculos |
| Resultado | `RESULTADO EN $var = valor` | Retorno explícito de funciones |
| Colecciones | `$lista[indice] = valor` | La indexación ya implica mutación |
| Texto | `COPIARTEXTO($txt, "...")` | Copia explícita para evitar efectos secundarios |

## 🗄️ Base de Datos SQLite

```nico
PROGRAMA test_bd
BLOQUE PRINCIPAL
    CONECTARBD("test.db")
    EJECUTARBD("CREATE TABLE IF NOT EXISTS usuarios (id INTEGER PRIMARY KEY, nombre TEXT)")
    EJECUTARBD("INSERT OR IGNORE INTO usuarios VALUES (1, 'Diego')")
    CONSULTARBD("SELECT * FROM usuarios")
    MIENTRAS(SIGUIENTEFILABD()) HACER
        ESCRIBIR("Usuario: $BDCOL2") SALTO
    FIN MIENTRAS
    CERRARCONSULTABD
    CERRARBD
FIN PRINCIPAL
FINAL
```

## 🌐 Servidor Web Integrado

```nico
PROGRAMA ServidorWeb
BLOQUE PRINCIPAL
    INICIARSERVER(8080)
    ESCRIBIR("Servidor en http://localhost:8080") SALTO
    MIENTRAS(VERDADERO) HACER
        ESPERAR(1, SEGUNDOS)
    FIN MIENTRAS
FIN PRINCIPAL
FINAL
```

## 🔌 Hardware: GPIO y PWM (Raspberry Pi)

```nico
CONFIGURARPIN(17, ENTRADA, PULLUP)
CONFIGURARPIN(18, SALIDA)
GENERARPWM(18, 100, 50)
```
Disponible en el AppImage para Raspberry Pi aarch64 con soporte nativo mediante libgpiod.

## 📄 Licencia y Contribuciones

**MIT License**. Contribuciones bienvenidas vía Issues/PRs.

### Créditos

- **Diseño/Arquitectura/Supervisión**: Diego Alejandro Majluff
- **Implementación**: Qwen (Alibaba Cloud)
- **Versión**: 2.1.0 (Agosto 2026)

### Cross-platform

Cross-platform
Compila y corre en Windows, Linux y Raspberry Pi.

Distribución:

- Windows: Instalador tradicional (`Nico-2.1.0-Setup.exe`) con todo incluido
- Linux x86_64: AppImage (`Nico-2.1.0-x86_64.AppImage`) con instalador (`install_appimage.sh`)
- Raspberry Pi aarch64: Paquete ZIP (`Nico-2.1.0-rpi-aarch64.zip`) con AppImage, instalador y guía completa

En Windows requiere DLLs incluidas (sqlite3, pthread, Qt5). En Linux y Raspberry Pi es un binario autocontenido.

Compilación desde fuentes disponible para todas las plataformas (MSYS2 opcional para Windows).

Compila y corre en Linux (incluyendo Raspberry Pi) y Windows. En Linux/RPi es un binario único; en Windows requiere DLLs incluidas (sqlite3, pthread, Qt5).

**Distribución**:
- **Linux**: Binario único + IDE Qt5
- **Windows**: Instalador con todo incluido (MSYS2 opcional para desarrolladores)
