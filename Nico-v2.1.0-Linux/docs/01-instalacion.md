# Instalación y compilación de Nico v2.1.0

Esta guía está diseñada para que cualquier persona, sin importar su experiencia previa, pueda instalar y usar Nico correctamente.

> **Regla de oro:** La forma más sencilla es usar los instaladores precompilados (**Opción A**). La compilación desde fuentes (**Opción B**) está reservada para desarrolladores que deseen modificar o extender el código.

---

# Opción A: Instalación rápida (Recomendada)

## Windows 10 / 11

### 1. Descargar

Descargá el archivo **`Nico-2.1.0-Setup.exe`** desde la página de Releases:

<https://github.com/DiegoMajluff/nico/releases>

### 2. Instalar

- Hacé doble clic en el archivo descargado.
- Seguí el asistente (**Siguiente → Siguiente → Instalar**).
- **IMPORTANTE:** Dejá marcada la opción **"Agregar Nico al PATH del sistema"** (viene activada por defecto).
- La opción **"Instalar MSYS2"** está **desmarcada**. Activala únicamente si sos desarrollador y querés compilar el código fuente.

> **Nota sobre Windows Defender**
>
> Al ser un proyecto de código abierto sin certificado digital de pago, es posible que Windows SmartScreen muestre una advertencia de **"Windows protegió su PC"** al instalar o ejecutar el IDE por primera vez.
>
> Se trata de un falso positivo habitual. Hacé clic en **"Más información"** y luego en **"Ejecutar de todos modos"**.

### 3. Usar

- Buscá **Nico IDE** en el menú Inicio.
- O abrí una terminal (CMD o PowerShell) y ejecutá:

```bash
nico ejemplos\basicos\01_hola_mundo.nico
nico-ide
```

### 4. Desinstalar

- Abrí **Configuración → Aplicaciones → Aplicaciones instaladas**.
- Buscá **Nico**.
- Hacé clic en los tres puntos y elegí **Desinstalar**.

Esto eliminará los archivos, los accesos directos y limpiará automáticamente la variable **PATH**.

---

## Linux (Ubuntu, Debian, Mint, Fedora, etc.)

### 1. Descargar

Descargá el archivo:

**`Nico-2.1.0-x86_64.AppImage`**

desde:

<https://github.com/DiegoMajluff/nico/releases>

### 2. Dar permisos

```bash
chmod +x Nico-2.1.0-x86_64.AppImage
```

### 3. Instalar

```bash
sudo ./install_appimage.sh Nico-2.1.0-x86_64.AppImage
```

Este script:

- copia el AppImage a `/opt/nico/`
- crea los comandos `nico` y `nico-ide`
- los agrega al `PATH`
- incorpora el IDE al menú de aplicaciones

### 4. Usar

```bash
nico ejemplos/basicos/01_hola_mundo.nico
nico-ide
```

### 5. Desinstalar

```bash
sudo nico-uninstall
```

---

## Raspberry Pi (Raspberry Pi OS)

### 1. Descargar

Descargá:

**`Nico-2.1.0-armhf.AppImage`**

desde:

<https://github.com/DiegoMajluff/nico/releases>

Incluye soporte GPIO nativo.

### 2. Dar permisos

```bash
chmod +x Nico-2.1.0-armhf.AppImage
```

### 3. Instalar

```bash
sudo ./install_appimage.sh Nico-2.1.0-armhf.AppImage
```

### 4. Permisos de GPIO

Para controlar los pines sin usar `sudo` en cada ejecución:

```bash
sudo usermod -a -G gpio $USER
```

> Reiniciá la sesión o la Raspberry Pi para que el cambio tenga efecto.

### 5. Usar

```bash
nico ejemplos/gpio/TestRaspiPiLed.nico
nico-ide
```

### 6. Desinstalar

```bash
sudo nico-uninstall
```

---

# Opción B: Compilación desde fuentes (Para desarrolladores)

Si querés modificar el código fuente de Nico, seguí estas instrucciones.

---

## Windows

### 1. Instalar MSYS2

Descargalo desde:

<https://www.msys2.org/>

### 2. Actualizar MSYS2

Abrí la terminal **MSYS2 MSYS** y ejecutá:

```bash
pacman -Syu
```

Cerrá la terminal, volvé a abrirla y ejecutá:

```bash
pacman -Su
```

### 3. Instalar herramientas

Abrí **solamente** la terminal **MSYS2 MinGW x64** y ejecutá:

```bash
pacman -S mingw-w64-x86_64-gcc \
          mingw-w64-x86_64-make \
          mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-qt5-base \
          mingw-w64-x86_64-sqlite3
```

### 4. Agregar MinGW al PATH

1. Presioná **Windows + R**.
2. Escribí `sysdm.cpl`.
3. Abrí **Opciones avanzadas → Variables de entorno**.
4. Editá la variable **Path** del sistema.
5. Agregá:

```text
C:\msys64\mingw64\bin
```

6. Aceptá todos los cambios.

### 5. Compilar

Abrí una nueva consola CMD, ubicate en la carpeta del proyecto y ejecutá:

```bat
build_windows_installer.bat
```

El script:

- compila el intérprete
- compila el IDE
- ejecuta `windeployqt`
- recopila las DLL de Qt5
- deja preparada la carpeta `dist/`

---

## Linux

### 1. Instalar dependencias

```bash
sudo apt update

sudo apt install -y \
    build-essential \
    gcc \
    cmake \
    qt5-qmake \
    qtbase5-dev \
    libsqlite3-dev \
    libreadline-dev
```

### 2. Compilar el intérprete

```bash
chmod +x compile.sh
./compile.sh
```

### 3. Compilar el IDE

```bash
cd nico-ide

mkdir -p build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release

make -j$(nproc)
```

### 4. Probar

```bash
./nico -e "2 + 2"

./nico-ide/build/nico-ide
```

> **Tip para recursión profunda**
>
> Antes de ejecutar programas complejos (como Ackermann), hacé:
>
> ```bash
> ulimit -s unlimited
> ```
>
> Esto evita desbordamientos de pila.

---

## Raspberry Pi

Los pasos son idénticos a Linux, agregando la biblioteca GPIO:

```bash
sudo apt install -y \
    build-essential \
    gcc \
    cmake \
    qt5-qmake \
    qtbase5-dev \
    libsqlite3-dev \
    libreadline-dev \
    libgpiod-dev
```

El script `compile.sh` detectará automáticamente la Raspberry Pi y habilitará el soporte GPIO.

---

# Verificación de instalación

En cualquier sistema operativo ejecutá:

```bash
nico -e "3 * 4 + 2"
```

Si la terminal responde:

```text
14
```

Nico está correctamente compilado, vinculado con SQLite3 y listo para usarse.

---

# Extensión para VS Code (Opcional)

Si preferís usar Visual Studio Code en lugar del IDE de Nico:

1. Abrí VS Code.
2. Andá a **Extensiones** (`Ctrl + Shift + X`).
3. Elegí **Instalar desde VSIX...**
4. Seleccioná:

```text
nico-language-1.0.0.vsix
```

5. Reiniciá VS Code.

Ahora tendrás resaltado de sintaxis para archivos `.nico`.

---

# Documentación completa

Nico incluye **21 guías temáticas** dentro de `docs/`.

| Archivo | Descripción |
|---------|-------------|
| `docs/MANUAL.md` | Manual completo del lenguaje |
| `docs/02-sintaxis.md` | Sintaxis detallada |
| `docs/03-base-de-datos.md` | SQLite integrado |
| `docs/04-panel-web.md` | Servidor HTTP y panel web |
| `docs/05-sistema-consola.md` | Comandos del sistema |
| `docs/06-graficos-ascii.md` | Gráficos ASCII |
| `docs/07-colores-estilos.md` | Colores y estilos |
| `docs/08-fecha-hora.md` | Fecha y hora |
| `docs/09-entrada-formato.md` | Entrada con formato |
| `docs/10-archivos.md` | Manejo de archivos |
| `docs/11-funciones.md` | Funciones |
| `docs/12-subprogramas.md` | Subprogramas |
| `docs/13-saltos-etiquetas.md` | Saltos y etiquetas |
| `docs/14-colisiones.md` | Colisiones |
| `docs/15-funciones-texto.md` | Funciones de texto |
| `docs/16-operaciones-bit.md` | Operaciones a nivel de bit |
| `docs/17-dibujarcirculo.md` | Dibujar círculos |
| `docs/18-incluir.md` | Inclusión de archivos |
| `docs/19-GPIO-y-PWM.md` | GPIO y PWM (Raspberry Pi) |
| `docs/20-vscode-extension.md` | Extensión para VS Code |
| `docs/21-motor-logico.md` | Motor lógico estilo Prolog |

Podés abrir cualquiera de estos documentos desde tu editor favorito o directamente desde el IDE de Nico.

---

# Próximo paso

Abrí:

```text
ejemplos/basicos/01_hola_mundo.nico
```

Modificá el mensaje y ejecutá:

```bash
nico ejemplos/basicos/01_hola_mundo.nico
```

**¡Bienvenido a Nico!**