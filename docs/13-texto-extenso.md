# 📝 Tipo de Dato: TEXTO EXTENSO

Variante dinámica del tipo `TEXTO` que permite almacenar cadenas de longitud variable sin límite fijo predefinido. Ideal para construir mensajes largos, leer archivos de tamaño desconocido o concatenar texto en bucles.

> 💡 **Nota clave**: A diferencia de `TEXTO` (buffer estático), `TEXTO EXTENSO` usa asignación dinámica en heap. El runtime gestiona la memoria automáticamente al salir de scope o al finalizar el programa.

---

## 📐 Sintaxis de Declaración
```nico
// Declaración global (nivel PROGRAMA)
VARIABLE TEXTO EXTENSO $nombre

// Declaración local (dentro de FUNCION)
FUNCION ENTERA MiFuncion()
    VARIABLE TEXTO EXTENSO $local = "valor inicial"
    // ...
FIN FUNCION
```

---

## 📋 Características Clave

| Característica | Comportamiento en Nico | Nota |
|----------------|------------------------|------|
| Crecimiento dinámico | Se expande automáticamente al asignar o concatenar | Sin límite fijo de buffer |
| Inicialización | Opcional: `= "valor"` o vacío por defecto | Si no se inicializa, comienza como `""` |
| Concatenación | Soporta operador `+`: `$a = $a + "más"` | También: `CONCATENARTEXTO($a, "más")` |
| Interpolación | Funciona en `ESCRIBIR("$var")` | Reemplaza `$var` por su contenido real |
| Scope | Global o local según dónde se declare | Igual que otros tipos de variable |
| Compatibilidad | Funciona con todas las funciones de `TEXTO` | `LONGITUDTEXTO`, `BUSCARTEXTO`, etc. |

---

## 🧪 Ejemplo Mínimo Validado
```nico
PROGRAMA test_texto_extenso
    VARIABLE TEXTO EXTENSO $corto

FUNCION ENTERA TextoLargo()
    VARIABLE TEXTO EXTENSO $txtfuncion = "Variable TEXTO EXTENSO local"
    ESCRIBIR("$txtfuncion") SALTO
    RETORNAR 1
FIN FUNCION

BLOQUE PRINCIPAL
    // Variante corta
    ASIGNAR EN $corto = "Texto corto en $corto"
    
    // Variante larga con crecimiento dinámico
    VARIABLE TEXTO EXTENSO $largo = ""
    ASIGNAR EN $largo = "Texto largo dinámico sin límite fijo. "
    ASIGNAR EN $largo = $largo + "¡Concatenación exitosa!"
    
    ESCRIBIR("1 - $corto") SALTO
    ESCRIBIR("2 - $largo") SALTO
    
    // Llamada a función con TEXTO EXTENSO local
    TextoLargo()
FIN PRINCIPAL
FINAL
```

---

## ⚠️ Notas Técnicas Críticas
- **Memoria dinámica:** Usa `malloc`/`realloc`/`free` internamente. El runtime libera automáticamente al salir de scope o al terminar el programa.
- **No hay límite práctico:** La única restricción es la RAM disponible del sistema. Para textos de varios MB, validá que haya recursos suficientes.
- **Performance en bucles:** Cada `$a = $a + "x"` puede realocar memoria. Para concatenaciones repetidas, preferí `CONCATENARTEXTO($a, "x")` si tu build optimiza el resize internamente.
- **Inicialización recomendada:** Aunque puede declararse sin valor inicial, es buena práctica explicitarlo: `VARIABLE TEXTO EXTENSO $x = ""`
- **Compatibilidad total:** Todas las funciones de `TEXTO` funcionan sin modificación: `LONGITUDTEXTO($extenso)`, `RECORTARTEXTO($extenso)`, etc.

---

## 🔄 Diferencias: TEXTO vs TEXTO EXTENSO

| Aspecto | TEXTO | TEXTO EXTENSO |
|---------|-------|--------------|
| Buffer | Tamaño fijo predefinido | Dinámico, crece según necesidad |
| Asignación larga | Puede truncar si excede buffer | Nunca trunca (mientras haya memoria) |
| Memoria | Stack o buffer estático | Heap (asignación dinámica) |
| Uso típico | Mensajes cortos, nombres, flags | Logs, archivos, respuestas, UIs dinámicas |
| Declaración | `VARIABLE TEXTO $x` | `VARIABLE TEXTO EXTENSO $x` |

> 💡 **Regla práctica:** Usá `TEXTO` para longitud conocida y corta. Usá `TEXTO EXTENSO` para contenido que puede crecer impredeciblemente.

---

## 🧩 Integración con Otras Funcionalidades
```nico
// TEXTO EXTENSO + Archivos: leer archivo completo
VARIABLE TEXTO EXTENSO $contenido = ""
VARIABLE ARCHIVO $f
VARIABLE TEXTO $linea = ""

ABRIRARCHIVO($f, "grande.txt", LECTURA)
MIENTRAS(1) HACER
    LEERARCHIVO($f, $linea)
    SI(TEXTOVACIO($linea)) ENTONCES CORTE FIN SI
    CONCATENARTEXTO($contenido, $linea)
    CONCATENARTEXTO($contenido, "\n")
FIN MIENTRAS
CERRARARCHIVO($f)

// TEXTO EXTENSO + Funciones de cadena: todas compatibles
VARIABLE TEXTO EXTENSO $mensaje = "  Hola Nico  "
RECORTARTEXTO($mensaje)  // Ahora: "Hola Nico"
MAYUSCULAS($mensaje)     // Ahora: "HOLA NICO"
$len = LONGITUDTEXTO($mensaje)  // $len = 9
```

---

## 🪟 Compatibilidad Windows/Linux

| Aspecto | Comportamiento |
|---------|---------------|
| Implementación | `malloc`/`realloc`/`free` en C (ambas plataformas) |
| Memoria máxima | Limitada por RAM disponible + swap del SO |
| Fugas de memoria | Nico libera automáticamente al salir de scope. Seguro. |
| Performance | Similar en ambas plataformas. Heap manager puede variar ligeramente. |

> 💡 **Tip de depuración:** Si sospechas uso excesivo de memoria, imprimí `LONGITUDTEXTO($variable)` periódicamente para monitorear crecimiento.