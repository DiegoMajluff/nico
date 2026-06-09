# 🔤 Funciones de Cadenas (TEXTO)

## 📏 Información y Consulta
| Función | Sintaxis | Ejemplo | Retorna / Nota |
|---------|----------|---------|---------------|
| `LONGITUDTEXTO` | `LONGITUDTEXTO($texto)` | `$len = LONGITUDTEXTO($nombre)` | `ENTERA`: cantidad de caracteres |
| `BUSCARTEXTO` | `BUSCARTEXTO($texto, $busqueda)` | `$pos = BUSCARTEXTO($completo, "Nico")` | `ENTERA`: posición (0-based) o `-1`. Case-insensitive. |
| `BUSCARCARACTER` | `BUSCARCARACTER($texto, 'c')` | `$pos = BUSCARCARACTER($nombre, 'i')` | `ENTERA`: posición del carácter o `-1`. |
| `COMPARARTEXTO` | `COMPARARTEXTO($t1, $t2)` | `$res = COMPARARTEXTO($a, $b)` | `ENTERA`: `0` si iguales, `!=0` si difieren. Case-sensitive. |
| `TEXTOVACIO` | `TEXTOVACIO($texto)` | `$es_vacio = TEXTOVACIO($var)` | `ENTERA`: `1` si vacío, `0` si tiene contenido. |

## ✏️ Modificación In-Place
| Procedimiento | Sintaxis | Ejemplo | Nota |
|--------------|----------|---------|------|
| `COPIARTEXTO` | `COPIARTEXTO($destino, $origen)` | `COPIARTEXTO($completo, $nombre)` | Copia contenido. `$destino` declarado. |
| `CONCATENARTEXTO` | `CONCATENARTEXTO($texto, $agregar)` | `CONCATENARTEXTO($completo, " Mundo")` | Agrega al final. |
| `MAYUSCULAS` | `MAYUSCULAS($texto)` | `MAYUSCULAS($completo)` | Convierte in-place. |
| `MINUSCULAS` | `MINUSCULAS($texto)` | `MINUSCULAS($completo)` | Convierte in-place. |
| `RECORTARTEXTO` | `RECORTARTEXTO($texto)` | `RECORTARTEXTO($con_espacios)` | Elimina espacios inicio/final. |
| `REEMPLAZARTEXTO` | `REEMPLAZARTEXTO($texto, $buscar, $reemplazar)` | `REEMPLAZARTEXTO($completo, "mundo", "Nico")` | Case-insensitive. Reemplaza todas. |

## 🔀 Extracción y División
| Procedimiento | Sintaxis | Ejemplo | Nota |
|--------------|----------|---------|------|
| `EXTRAERTEXTO` | `EXTRAERTEXTO($origen, $inicio, $longitud, $destino)` | `EXTRAERTEXTO($completo, 0, 4, $sub)` | Substring. Inicio 0-based. |
| `DIVIDIRTEXTO` | `DIVIDIRTEXTO($texto, $delimitador, $indice, $destino)` | `DIVIDIRTEXTO($csv, ',', 2, $parte)` | Obtiene parte `$indice`. |
| `REPETIRTEXTO` | `REPETIRTEXTO($texto, $veces, $destino)` | `REPETIRTEXTO("AB", 3, $res)` | Repite y guarda en `$destino`. |

## 🔢 Conversión de Tipos
| Función / Proc. | Sintaxis | Ejemplo | Retorna / Nota |
|----------------|----------|---------|--------------|
| `TEXTOAENTERO` | `TEXTOAENTERO($texto)` | `$n = TEXTOAENTERO("123")` | `ENTERA`. Si falla, `0`. |
| `ENTEROATEXTO` | `ENTEROATEXTO($numero, $destino)` | `ENTEROATEXTO(133, $txt)` | `void` en `$destino`. |
| `TEXTOADECIMAL` | `TEXTOADECIMAL($texto)` | `$d = TEXTOADECIMAL("3.14")` | `DECIMAL`. Si falla, `0.0`. |
| `DECIMALATEXTO` | `DECIMALATEXTO($decimal, $destino)` | `DECIMALATEXTO(3.14, $txt)` | `void` en `$destino`. |
| `TEXTOACARACTER` | `TEXTOACARACTER($texto)` | `$ascii = TEXTOACARACTER("A")` | `ENTERA`: ASCII primer carácter. |
| `CARACTERATEXTO` | `CARACTERATEXTO($codigo, $destino)` | `CARACTERATEXTO(65, $txt)` | `void`: 1 carácter en `$destino`. |

> 📌 **Notas:** In-Place vs Retorno, Case-Insensitivity en búsqueda/reemplazo, índices 0-based.

---

# 🔢 Funciones Matemáticas

## 📐 Constantes
| Función | Sintaxis | Retorna |
|---------|----------|---------|
| `NUMEROPI` | `NUMEROPI()` | `DECIMAL`: π ≈ 3.14159 |
| `NUMEROEULER` | `NUMEROEULER()` | `DECIMAL`: e ≈ 2.71828 |
| `RAIZDEUNMEDIO` | `RAIZDEUNMEDIO()` | `DECIMAL`: √0.5 ≈ 0.7071 |
| `LOGNATURALDE2` | `LOGNATURALDE2()` | `DECIMAL`: ln(2) ≈ 0.6931 |
| `LOGNATURALDE10` | `LOGNATURALDE10()` | `DECIMAL`: ln(10) ≈ 2.3025 |

## 🧮 Básicas
| Función | Sintaxis | Retorna / Nota |
|---------|----------|---------------|
| `RAIZ` | `RAIZ($x)` | Raíz cuadrada |
| `RAIZCUBICA` | `RAIZCUBICA($x)` | Raíz cúbica |
| `POTENCIA` | `POTENCIA($base, $expo)` | `$base ^ $expo` |
| `MODULO` | `MODULO($a, $b)` | Resto de `$a / $b` |
| `ABSOLUTO` | `ABSOLUTO($x)` | Valor absoluto |

## 📐 Trigonométricas (radianes)
| Función | Sintaxis | Retorna |
|---------|----------|---------|
| `SENO` | `SENO($rad)` | Seno |
| `COSENO` | `COSENO($rad)` | Coseno |
| `TANGENTE` | `TANGENTE($rad)` | Tangente |
| `ARCOSENO` | `ARCOSENO($x)` | Arcoseno [-1,1] |
| `ARCOCOSENO` | `ARCOCOSENO($x)` | Arcocoseno [-1,1] |
| `ARCOTANGENTE` | `ARCOTANGENTE($x)` | Arcotangente |

## 📊 Logarítmicas y Exponenciales
| Función | Sintaxis | Retorna |
|---------|----------|---------|
| `LOGNATURAL` | `LOGNATURAL($x)` | ln(x) |
| `LOGBASE10` | `LOGBASE10($x)` | log₁₀(x) |
| `LOGBASE2` | `LOGBASE2($x)` | log₂(x) |
| `LOGARITMO` | `LOGARITMO($x, $base)` | log_base(x) |
| `EXPONENCIAL` | `EXPONENCIAL($x)` | e^x |
| `DOSALAX` | `DOSALAX($x)` | 2^x |

## 🔢 Redondeo, Comparación y Operadores
| Función | Sintaxis | Retorna / Nota |
|---------|----------|---------------|
| `REDONDEAR` | `REDONDEAR($modo, $valor)` | `ARRIBA`, `ABAJO`, `ENTERO` |
| `QUITARDECIMAL` | `QUITARDECIMAL($x)` | Trunca hacia cero |
| `MAXIMO` | `MAXIMO($a, $b)` | Mayor valor |
| `MINIMO` | `MINIMO($a, $b)` | Menor valor |
| `+ - * / ^` | `$a op $b` | Operadores aritméticos estándar |

> 📌 **Notas:** Radianes obligatorios en trig. `ASIGNAR EN`, `RESULTADO EN`, `CALCULAR EN` son alias. `ESCRIBIR(..., DECIMALES(n))` formatea salida.

---

# 🔀 Control de Flujo

## 🔀 SEGUN CASO
```nico
SEGUN CASO ($variable)
    CASO 1
        // código
        CORTE
    CASO 2
        // código
        CORTE
    POR DEFECTO
        // fallback
        CORTE
FIN SEGUN
```
- `CORTE` obligatorio por caso (evita fall-through).
- `POR DEFECTO` opcional.
- Soporta anidamiento ilimitado. Cada nivel requiere su `FIN SEGUN`.

## 🔁 REALIZAR...MIENTRAS (Do-While)
```nico
REALIZAR
    // se ejecuta al menos 1 vez
MIENTRAS ($condición)
```
- Evalúa condición al final.
- No lleva `FIN MIENTRAS` (el `MIENTRAS` cierra el bloque).
- Ideal para menús y validación de entrada.

---

# 🖥️ Sistema y Control de Consola

## ⏱️ Temporización y Shell
| Comando | Sintaxis | Ejemplo | Nota |
|---------|----------|---------|------|
| `ESPERAR` | `ESPERAR(valor, UNIDAD)` | `ESPERAR(500, MILISEGUNDOS)` | Unidades: `MICROS`/`US`, `MILISEGUNDOS`/`MS`, `SEGUNDOS`/`S`, `MINUTOS`/`MIN`. Case-insensitive. |
| `SISTEMA` | `SISTEMA("comando")` | `SISTEMA("ls -l")` | Ejecuta comando del SO. Captura `stdout` en terminal. Comillas obligatorias. |

## 📐 Información y Cursor
| Comando | Sintaxis | Retorna / Acción |
|---------|----------|-----------------|
| `ANCHOTERMINAL` | `ANCHOTERMINAL($destino)` | Columnas visibles (`ENTERA`) |
| `ALTOTERMINAL` | `ALTOTERMINAL($destino)` | Filas visibles (`ENTERA`) |
| `CURSOR` | `CURSOR($fila, $col)` | Posiciona cursor (1-based) |
| `LIMPIARPANTALLA` | `LIMPIARPANTALLA` | Borra pantalla + cursor a (1,1) |
| `OCULTARCURSOR` | `OCULTARCURSOR` | Oculta cursor (sin args) |
| `MOSTRARCURSOR` | `MOSTRARCURSOR` | Muestra cursor (sin args) |
| `LEERTECLA` | `LEERTECLA($destino)` | Captura 1 tecla sin `Enter`. Retorna código ASCII/byte. |

> ⚠️ `LEERTECLA` configura terminal en modo raw/cbreak automáticamente. En bucles, agregar `ESPERAR(10-50, MS)` para evitar CPU al 100%.

---

# 🎨 Colores y Estilos de Texto

## 🌈 Colores
| Comando | Sintaxis | Colores válidos (minúsculas, sin comillas) |
|---------|----------|-------------------------------------------|
| `COLORTEXTO` | `COLORTEXTO(color)` | `negro`, `rojo`, `verde`, `amarillo`, `azul`, `magenta`, `cyan`, `blanco` |
| `COLORFONDO` | `COLORFONDO(color)` | Mismos que `COLORTEXTO` |
| `RESETCOLOR` | `RESETCOLOR` | Restablece colores por defecto |

## ✨ Estilos
| Comando | Sintaxis | Efecto |
|---------|----------|--------|
| `TEXTONEGRITA` | `TEXTONEGRITA` | Peso fuerte (bold) |
| `TEXTOCURSIVA` | `TEXTOCURSIVA` | Itálico (soporte variable) |
| `TEXTOSUBRAYADO` | `TEXTOSUBRAYADO` | Subrayado simple |
| `RESETTEXTO` | `RESETTEXTO` | Quita estilos, **no afecta colores** |

> 📌 **Regla de oro:** `RESETCOLOR` y `RESETTEXTO` son independientes. Para volver al estado original, usá ambos. Los estilos/colores persisten hasta reset explícito.

---

# 🎨 Gráficos ASCII y Relleno

## 📏 Primitivas de Dibujo
| Procedimiento | Sintaxis | Nota |
|--------------|----------|------|
| `DIBUJARLINEA` | `DIBUJARLINEA($x1, $y1, $x2, $y2, $char)` | Algoritmo de Bresenham. `$char` 1 carácter. |
| `DIBUJARCIRCULO` | `DIBUJARCIRCULO($cx, $cy, $radio, $char)` | Aproximación paramétrica. Fuente monoespaciada recomendada. |
| `RELLENARRECTANGULO` | `RELLENARRECTANGULO($x1, $y1, $x2, $y2, $char)` | Rellena área incluida. Ideal para fondos/paneles. |

> 📌 Coordenadas **1-based** `(columna, fila)`. Si `$x2 < $x1` o `$y2 < $y1`, comportamiento indefinido. Validar antes de llamar.

---

# 🕒 Fecha y Hora

| Comando | Sintaxis | Formato de Salida |
|---------|----------|------------------|
| `HORAACTUAL` | `HORAACTUAL($destino)` | `"HH:MM:SS"` (24h) |
| `FECHAACTUAL` | `FECHAACTUAL($destino)` | `"DD/MM/AAAA"` |

> 📌 Requieren `VARIABLE TEXTO` declarada. Escriben por referencia. Hora local del sistema (sin TZ/UTC en v1.1.0).

---

# ⌨️ Entrada y Formato de Salida

## 🔤 Lectura de Usuario
| Comando | Sintaxis | Comportamiento |
|---------|----------|---------------|
| `LEER` | `LEER($variable)` | Bloqueante hasta `Enter`. Conversión automática según tipo declarado (`ENTERA`, `DECIMAL`, `TEXTO`, `CARACTER`). Si falla conversión numérica → `0`. |

## 🔢 Formato `DECIMALES`
| Uso | Sintaxis | Regla |
|-----|----------|-------|
| Impresión | `ESCRIBIR("$a $b $c", DECIMALES($p1, $p2, $p3))` | `$p1` aplica a 1ª variable numérica, `$p2` a 2ª, etc. Ignora `TEXTO`. Orden posicional estricto. |
| Valores válidos | `0` a `~15` | Redondeo estándar (half-up). No modifica valor en memoria, solo salida. |

> ✅ `ESCRIBIR("Nota: $n", DECIMALES(2))` → imprime con 2 decimales.
> ❌ `DECIMALES` fuera de `ESCRIBIR`/`MOSTRAR` → error de sintaxis.

---

# 📁 Manejo de Archivos

## 📋 Comandos y Modos
| Comando | Sintaxis | Nota |
|---------|----------|------|
| `ABRIRARCHIVO` | `ABRIRARCHIVO($archivo, "ruta", MODO)` | `$archivo` debe ser `VARIABLE ARCHIVO`. `MODO` es constante sin comillas. |
| `ESCRIBIRARCHIVO` | `ESCRIBIRARCHIVO($archivo, $texto)` | Agrega `\n` automáticamente al final. |
| `LEERARCHIVO` | `LEERARCHIVO($archivo, $destino)` | Lee 1 línea hasta `\n`. Remueve salto final. `$destino` es `TEXTO`. |
| `CERRARARCHIVO` | `CERRARARCHIVO($archivo)` | Libera descriptor. Hace flush implícito. Obligatorio. |

| Modo (Nico) | Equivalente C | Comportamiento |
|-------------|--------------|---------------|
| `LECTURA` | `"r"` | Solo lectura. Debe existir. |
| `ESCRITURA` | `"w"` | Escritura pura. Trunca si existe. Crea si no. |
| `AGREGAR` | `"a"` | Append al final. No trunca. Crea si no. |
| `LECTOESCRITURA` | `"r+"` | Lectura/escritura. Debe existir. |

> 📌 Rutas: Relativas al directorio de ejecución. Usar `/` para portabilidad Win/Linux.

---

# 🔁 Funciones con Retorno

## 📐 Sintaxis
```nico
FUNCION <TIPO_RETORNO> nombre($param1, $param2, ...)
    VARIABLE <TIPO> $local = valor
    // lógica...
    RETORNAR <expresión>
FIN FUNCION
```

## 📋 Tipos Válidos
| Tipo | Rango Aprox. | Uso |
|------|-------------|-----|
| `ENTERA` | ±2.1×10⁹ | Contadores, índices |
| `DECIMAL` | ±1.7×10³⁰⁸ | Cálculos, promedios |
| `ENTERA SIN SIGNO` | 0 a 4.2×10⁹ | IDs, tamaños |
| `DECIMAL SIN SIGNO` | 0 a +1.7×10³⁰⁸ | Magnitudes físicas |

> ⚠️ No existe `VOID`. Toda función debe retornar un valor compatible con su tipo declarado. Parámetros por valor. Scope local para variables declaradas dentro.

---

# 🔧 Subprogramas (Scope Global)

## 📐 Sintaxis
```nico
SUBPROGRAMA Nombre
    // Opera sobre variables globales del PROGRAMA
    // Sin parámetros, sin retorno
FIN SUBPROGRAMA

// Llamada
LLAMAR A Nombre
```

## 📋 Características
- ❌ Sin parámetros ni `RETORNAR`
- 🌐 Scope global siempre: accede/modifica variables declaradas a nivel de `PROGRAMA`
- 🎯 Propósito: organizar código, evitar `BLOQUE PRINCIPAL` extenso
- 📝 Documentar qué variables globales lee/modifica cada uno

> 📌 **Diferencia clave:** `FUNCION` = calcula y retorna (aislado). `SUBPROGRAMA` = organiza y muta estado global.

---

# 🏗️ Estructura Base y Sintaxis

## 📐 Esqueleto Mínimo
```nico
PROGRAMA NombreDelPrograma

    // Declaraciones globales (fuera de bloques)
    VARIABLE ENTERA $contador = 0
    VARIABLE TEXTO $mensaje = "Hola"

SUBPROGRAMA MiSub
    // ...
FIN SUBPROGRAMA

FUNCION ENTERA MiFuncion($a, $b)
    // ...
FIN FUNCION

BLOQUE PRINCIPAL
    // Lógica ejecutable
    ESCRIBIR("Inicio") SALTO
    LLAMAR A MiSub
    ASIGNAR EN $resultado = MiFuncion(2, 3)
FIN PRINCIPAL
FINAL
```

## 📋 Reglas Fundamentales
| Concepto | Regla |
|----------|-------|
| `PROGRAMA` | Nombre único. Sin extensión `.nico` en el código. |
| `FINAL` | Cierra obligatoriamente el archivo. |
| `BLOQUE PRINCIPAL` | Punto de entrada. Obligatorio. Todo código ejecutable debe ir aquí o en sub/funciones. |
| `VARIABLE` | Declaración explícita. Tipos: `ENTERA`, `DECIMAL`, `TEXTO`, `CARACTER`, `ARCHIVO`, `ENTERA SIN SIGNO`, `DECIMAL SIN SIGNO`. |
| `SALTO` | Equivalente a `\n`. Opcional tras `ESCRIBIR`. |
| Comentarios | `// comentario de línea`. Nico no admite comentarios en bloque. |

> 📌 **Asignación:** `ASIGNAR EN $var = valor`, `CALCULAR EN $var = expresión`, `RESULTADO EN $var = valor` (alias válidos).
