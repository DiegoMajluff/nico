# 🔤 Funciones de Manipulación de Texto

Comandos nativos para consultar, transformar y convertir cadenas de texto (`TEXTO`). Todas las funciones operan sobre variables declaradas con `VARIABLE TEXTO` y respetan el scope (global o local) donde se ejecutan.

> 💡 **Nota técnica**: Estas funciones están registradas en la tabla de despacho (`dispatch_table`) del runtime. Si agregás una nueva función de texto en `cadenas.c`, debés registrarla con un wrapper en `runtime.c` para que sea reconocida.

---

## 📏 Funciones de Consulta (Retornan valor)

| Función | Sintaxis | Retorna | Ejemplo | Nota |
|---------|----------|---------|---------|------|
| `LONGITUDTEXTO` | `LONGITUDTEXTO($texto)` | `ENTERA` | `$len = LONGITUDTEXTO($nombre)` | Cantidad de caracteres (0-based). |
| `BUSCARTEXTO` | `BUSCARTEXTO($texto, $busqueda)` | `ENTERA` | `$pos = BUSCARTEXTO($frase, "Nico")` | Posición de primera coincidencia (0-based) o `-1`. **Case-insensitive**. |
| `BUSCARCARACTER` | `BUSCARCARACTER($texto, 'c')` | `ENTERA` | `$pos = BUSCARCARACTER($nombre, 'i')` | Posición del carácter o `-1`. Comilla simple obligatoria. |
| `COMPARARTEXTO` | `COMPARARTEXTO($t1, $t2)` | `ENTERA` | `$res = COMPARARTEXTO($a, $b)` | `0` si iguales, `!=0` si difieren. **Case-sensitive**. |
| `TEXTOVACIO` | `TEXTOVACIO($texto)` | `ENTERA` | `$es_vacio = TEXTOVACIO($var)` | `1` si vacío, `0` si tiene contenido. |

### 🧪 Ejemplo: Búsqueda y comparación
```nico
PROGRAMA DemoBusqueda
    VARIABLE TEXTO $frase = "Hola Nico"
    VARIABLE ENTERA $pos
BLOQUE PRINCIPAL
    $pos = BUSCARTEXTO($frase, "nico")  // Case-insensitive: encuentra en posición 5
    SI ($pos >= 0) ENTONCES
        ESCRIBIR("Encontrado en posición: $pos") SALTO
    FIN SI

    SI (COMPARARTEXTO("Hola", "hola") == 0) ENTONCES  // Case-sensitive: NO son iguales
        ESCRIBIR("Iguales") SALTO
    SINO
        ESCRIBIR("Diferentes") SALTO  // ← Este camino se ejecuta
    FIN SI
FIN PRINCIPAL
FINAL
```

---

## ✏️ Procedimientos de Modificación (In-Place)

Modifican directamente el contenido de la variable destino. No retornan valor.

| Procedimiento | Sintaxis | Efecto | Ejemplo | Nota |
|--------------|----------|--------|---------|------|
| `COPIARTEXTO` | `COPIARTEXTO($destino, $origen)` | Copia contenido | `COPIARTEXTO($a, $b)` | `$destino` debe estar declarado. Sobrescribe contenido previo. |
| `CONCATENARTEXTO` | `CONCATENARTEXTO($texto, $agregar)` | Agrega al final | `CONCATENARTEXTO($saludo, " Mundo")` | Útil para construir mensajes dinámicos. |
| `MAYUSCULAS` | `MAYUSCULAS($texto)` | Convierte a mayúsculas | `MAYUSCULAS($nombre)` | Transformación in-place. |
| `MINUSCULAS` | `MINUSCULAS($texto)` | Convierte a minúsculas | `MINUSCULAS($titulo)` | Transformación in-place. |
| `RECORTARTEXTO` | `RECORTARTEXTO($texto)` | Elimina espacios inicio/final | `RECORTARTEXTO($entrada)` | Útil para sanitizar input de usuario. |
| `REEMPLAZARTEXTO` | `REEMPLAZARTEXTO($texto, $buscar, $reemplazar)` | Reemplaza todas las ocurrencias | `REEMPLAZARTEXTO($frase, "mundo", "Nico")` | Case-insensitive. |

### 🧪 Ejemplo: Construcción dinámica de texto
```nico
PROGRAMA DemoModificacion
    VARIABLE TEXTO $saludo = "hola"
    VARIABLE TEXTO $nombre = " diego "
    VARIABLE TEXTO $final = ""
BLOQUE PRINCIPAL
    MAYUSCULAS($saludo)              // "hola" → "HOLA"
    RECORTARTEXTO($nombre)           // " diego " → "diego"
    CONCATENARTEXTO($final, $saludo) // $final = "HOLA"
    CONCATENARTEXTO($final, " ")     // $final = "HOLA "
    CONCATENARTEXTO($final, $nombre) // $final = "HOLA diego"
    
    ESCRIBIR("$final") SALTO         // Imprime: HOLA diego
FIN PRINCIPAL
FINAL
```

---

## 🔀 Extracción y División

| Procedimiento | Sintaxis | Efecto | Ejemplo | Nota |
|--------------|----------|--------|---------|------|
| `EXTRAERTEXTO` | `EXTRAERTEXTO($origen, $inicio, $longitud, $destino)` | Substring | `EXTRAERTEXTO($frase, 0, 4, $sub)` | `$inicio` es 0-based. `$destino` debe estar declarado. |
| `DIVIDIRTEXTO` | `DIVIDIRTEXTO($texto, $delimitador, $indice, $destino)` | Obtiene parte por índice | `DIVIDIRTEXTO($csv, ",", 1, $parte)` | `$indice` 0-based. Útil para parsear CSV. |
| `REPETIRTEXTO` | `REPETIRTEXTO($texto, $veces, $destino)` | Repite cadena | `REPETIRTEXTO("=", 10, $linea)` | `$destino` debe estar declarado. |

### 🧪 Ejemplo: Parseo de CSV simple
```nico
PROGRAMA DemoCSV
    VARIABLE TEXTO $datos = "Diego,Majluff,LU8DAM"
    VARIABLE TEXTO $nombre = "", $apellido = "", $callsign = ""
BLOQUE PRINCIPAL
    DIVIDIRTEXTO($datos, ",", 0, $nombre)     // "Diego"
    DIVIDIRTEXTO($datos, ",", 1, $apellido)   // "Majluff"
    DIVIDIRTEXTO($datos, ",", 2, $callsign)   // "LU8DAM"
    
    ESCRIBIR("Nombre: $nombre $apellido ($callsign)") SALTO
FIN PRINCIPAL
FINAL
```

---

## 🔢 Conversión de Tipos

### De texto a numérico (Funciones que retornan valor)

| Función | Sintaxis | Retorna | Ejemplo | Nota |
|---------|----------|---------|---------|------|
| `TEXTOAENTERO` | `TEXTOAENTERO($texto)` | `ENTERA` | `$num = TEXTOAENTERO("123")` | Si la conversión falla, retorna `0`. |
| `TEXTOADECIMAL` | `TEXTOADECIMAL($texto)` | `DECIMAL` | `$pi = TEXTOADECIMAL("3.14")` | Usa punto `.` como separador decimal. |
| `TEXTOACARACTER` | `TEXTOACARACTER($texto)` | `CARACTER` | `$c = TEXTOACARACTER("A")` | Retorna el primer carácter como código ASCII. |

### De numérico a texto (Procedimientos que modifican destino)

| Procedimiento | Sintaxis | Efecto | Ejemplo | Nota |
|--------------|----------|--------|---------|------|
| `ENTEROATEXTO` | `ENTEROATEXTO($valor, $destino)` | Convierte entero a texto | `ENTEROATEXTO(42, $txt)` | `$destino` debe ser `VARIABLE TEXTO`. |
| `DECIMALATEXTO` | `DECIMALATEXTO($valor, $destino)` | Convierte decimal a texto | `DECIMALATEXTO(3.14, $txt)` | Usa punto `.` como separador. |
| `CARACTERATEXTO` | `CARACTERATEXTO($valor, $destino)` | Convierte ASCII a texto | `CARACTERATEXTO(65, $txt)` | `65` → `"A"`. |

### 🧪 Ejemplo: Conversión bidireccional
```nico
PROGRAMA DemoConversion
    VARIABLE TEXTO $txt_num = "2026"
    VARIABLE ENTERA $anio
    VARIABLE TEXTO $txt_result = ""
BLOQUE PRINCIPAL
    $anio = TEXTOAENTERO($txt_num)        // "2026" → 2026
    CALCULAR EN $anio = $anio + 1         // 2027
    ENTEROATEXTO($anio, $txt_result)      // 2027 → "2027"
    
    ESCRIBIR("Próximo año: $txt_result") SALTO
FIN PRINCIPAL
FINAL
```

---

## ⚠️ Notas Técnicas Importantes

### 🔹 Registro en `dispatch_table`
Para que una función de texto sea reconocida por el runtime, debe estar registrada en `src/runtime.c`:

```c
// Wrapper genérico para funciones de texto
int cmd_texto_funcion(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)ctx; (void)linea_actual;
    procesar_funcion_texto(linea);
    return 0;
}

// Entrada en la tabla de despacho
const CmdEntry dispatch_table[] = {
    { "COPIARTEXTO",         cmd_texto_funcion },
    { "CONCATENARTEXTO",     cmd_texto_funcion },
    // ... resto de funciones ...
};
```

Si agregás una nueva función en `cadenas.c`, no olvides registrarla aquí, de lo contrario el runtime la ignorará silenciosamente.

### 🔹 Scope de variables
- Las funciones de texto operan sobre variables declaradas previamente en el scope actual.
- Variables globales (`PROGRAMA`) son accesibles desde cualquier bloque.
- Variables locales (`FUNCION`) solo existen dentro de su función.

### 🔹 Validación de comillas en inicialización
Al inicializar `LISTA CARACTER` o `MATRIZ CARACTER`, cada carácter debe ir entre comillas simples:

```nico
// ✅ VÁLIDO
LISTA CARACTER $letras[3] = {'A', 'B', 'C'}

// ❌ INVÁLIDO (error explícito)
LISTA CARACTER $letras[3] = {'A', B, 'C'}  // Error línea X: Los caracteres en LISTA CARACTER deben ir entre comillas simples (ej: 'A').
```

Esta validación estricta evita errores silenciosos y asegura que el parser interprete correctamente cada valor.

### 🔹 Límites prácticos

| Constante | Valor | Afecta a |
|-----------|-------|----------|
| `MAX_TEXTO_LEN` | `4096` | Longitud máxima de `VARIABLE TEXTO` |
| `MAX_LISTA` | `1024` | Tamaño máximo de listas |
| `MAX_DIMENSION_FILA/COLUMNA` | `64` | Tamaño máximo de matrices |

Intentar superar estos límites genera error en tiempo de compilación/ejecución.

---

## 🧰 Troubleshooting Rápido

| Síntoma | Causa probable | Solución |
|---------|---------------|----------|
| Función de texto no se ejecuta (sin error) | No registrada en `dispatch_table` | Agregar entrada con wrapper en `runtime.c` |
| `BUSCARTEXTO` no encuentra coincidencia | Case-sensitive vs insensitive | Recordar: `BUSCARTEXTO` es case-insensitive, `COMPARARTEXTO` es case-sensitive |
| `RECORTARTEXTO` no elimina todos los espacios | Solo quita inicio/final, no internos | Usar `REEMPLAZARTEXTO($txt, " ", "")` para quitar todos |
| Error "comilla simple faltante" en lista | Valor sin `' '` en inicialización | Envolver cada carácter: `{'A', 'B'}` no `{A, B}` |
| Conversión `TEXTOAENTERO` retorna `0` | Texto no numérico o formato inválido | Validar con `TEXTOVACIO` o `BUSCARCARACTER` antes de convertir |

---

## 🔗 Ver también
- `02-sintaxis.md` - Declaración de variables y tipos.
- `11-funciones.md` - Funciones definidas por el usuario.
- `13-texto-extenso.md` - Tipo `TEXTO EXTENSO` para cadenas dinámicas.
- `19-estructuras-datos.md` - Inicialización de listas y matrices.

> 📚 Documentación validada con Nico v1.1.0. Ejemplos probados en Linux/Raspberry Pi/Windows.