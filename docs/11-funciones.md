# 🔁 Funciones con Retorno y Variables Locales

Nico soporta funciones definidas por el usuario con tipo de retorno explícito, parámetros por valor y variables locales aisladas por scope. Todas las funciones deben retornar un valor numérico compatible con su declaración.

> 💡 **Diferencia clave con SUBPROGRAMA**: Las funciones calculan y retornan un valor con aislamiento de scope. Los subprogramas organizan código y mutan estado global sin retornos.

---

## 📐 Sintaxis de Declaración
```nico
FUNCION <TIPO_RETORNO> nombre_funcion($param1, $param2, ...)
    VARIABLE <TIPO> $local = valor  // Variables locales (opcionales)
    // ... lógica de cálculo ...
    RETORNAR <expresión>  // Obligatorio si el tipo no es vacío
FIN FUNCION
```

---

## 📋 Tipos de Retorno Válidos

| Tipo (Nico) | Rango Aprox. | Uso Típico | Nota |
|-------------|--------------|------------|------|
| `ENTERA` | ±2.1×10⁹ | Contadores, índices, flags | Signed 32-bit |
| `DECIMAL` | ±1.7×10³⁰⁸ | Cálculos, promedios, precios | IEEE 754 double |
| `ENTERA SIN SIGNO` | 0 a 4.2×10⁹ | IDs, tamaños, hardware | Unsigned 32-bit |
| `DECIMAL SIN SIGNO` | 0 a +1.7×10³⁰⁸ | Magnitudes físicas, prob. | Sin negativos |

> ⚠️ **No existe `VOID`:** Toda función debe declarar un tipo de retorno y ejecutar al menos un `RETORNAR <valor>` compatible.

---

## 📋 Referencia de Keywords

| Keyword | Función | ¿Obligatorio? | Nota |
|---------|---------|--------------|------|
| `FUNCION <TIPO>` | Declara función con retorno | ✅ Sí | `<TIPO>`: uno de los 4 listados arriba. |
| `($param1, ...)` | Lista de parámetros | ❌ No | Se pasan por valor. Modificarlos dentro no afecta el original. |
| `VARIABLE ...` | Variable local | ❌ No | Scope limitado a la función. No colisiona con globales. |
| `RETORNAR <expr>` | Devuelve valor al llamador | ✅ Sí | Debe coincidir con `<TIPO_RETORNO>`. |

---

## 🧪 Ejemplo Mínimo Validado
```nico
PROGRAMA FuncionesTipos
    VARIABLE DECIMAL $d = 0.0

FUNCION DECIMAL promedio($a, $b, $c)
    VARIABLE DECIMAL $resultado = ($a + $b + $c) / 3.0
    RETORNAR $resultado
FIN FUNCION

BLOQUE PRINCIPAL
    ASIGNAR EN $d = promedio(8.5, 9.0, 7.5)
    ESCRIBIR("Promedio: $d", DECIMALES(2)) SALTO
FIN PRINCIPAL
FINAL
```

---

## ⚠️ Notas Técnicas Críticas
- **Conversión implícita en `RETORNAR`:** Si el tipo de la expresión no coincide exactamente, Nico intenta convertir: `DECIMAL` → `ENTERA` trunca hacia cero (`3.9` → `3`). `ENTERA` → `DECIMAL` promueve sin pérdida.
- **Scope aislado:** Variables declaradas dentro son locales. No son visibles fuera, ni colisionan con globales del mismo nombre.
- **Parámetros por valor:** Modificar `$a` dentro no cambia la variable original del llamador. Para múltiples salidas, usá `TEXTO` con delimitadores o variables globales (con precaución).
- **`RETORNAR` obligatorio:** Debe haber al menos uno en cada ruta de ejecución. Si el flujo puede salir sin retornar, el comportamiento es indefinido.
- **Sin sobrecarga:** No podés tener dos funciones con el mismo nombre y distintos parámetros. El nombre debe ser único en el programa.

---

## 🎯 Buenas Prácticas
```nico
// ✅ Usar SIN SIGNO cuando el dominio es no-negativo
FUNCION ENTERA SIN SIGNO calcular_tamano($ancho, $alto)
    RETORNAR $ancho * $alto
FIN FUNCION

// ✅ Validar antes de retornar para evitar errores
FUNCION DECIMAL dividir_seguro($numerador, $denominador)
    SI($denominador IGUAL 0) ENTONCES
        RETORNAR 0.0
    FIN SI
    RETORNAR $numerador / $denominador
FIN FUNCION

// ✅ Documentar tipos de parámetros y retorno
// Retorna DECIMAL: promedio de 3 notas (0.0 - 10.0)
FUNCION DECIMAL calcular_promedio($a, $b, $c)
    RETORNAR ($a + $b + $c) / 3.0
FIN FUNCION

// ❌ Evitar conversión peligrosa o división por cero sin validar
FUNCION ENTERA dividir_y_truncar($a, $b)
    RETORNAR $a / $b  // ← Sin validación previa
FIN FUNCION
```

---

## 🔄 Equivalencias C Internas

| Nico | C Runtime | Descripción |
|------|-----------|-------------|
| `FUNCION ENTERA f($x)` | `int f(int x) {` | Signed 32-bit |
| `FUNCION DECIMAL f($x)` | `double f(double x) {` | IEEE 754 double |
| `FUNCION ENTERA SIN SIGNO f($x)` | `unsigned int f(...) {` | Unsigned 32-bit |
| `RETORNAR $expr` | `return expr;` | Retorno de valor |
| `FIN FUNCION` | `}` | Cierre de función |

---

## 🧩 Integración con Otras Estructuras
```nico
// Función + DECIMALES en impresión
FUNCION DECIMAL calcular_iva($monto, $tasa)
    RETORNAR $monto * $tasa / 100.0
FIN FUNCION

BLOQUE PRINCIPAL
    VARIABLE DECIMAL $precio = 100.0, $iva = 0.0
    ASIGNAR EN $iva = calcular_iva($precio, 21.0)
    ESCRIBIR("IVA: $iva", DECIMALES(2)) SALTO
FIN PRINCIPAL

// Función dentro de expresiones matemáticas
CALCULAR EN $total = calcular_iva($precio, 21.0) + $precio
```

> 💡 **Compatibilidad:** Funciona idéntico en Linux y Windows. Los tipos se mapean directamente a tipos estándar de C, garantizando portabilidad y rendimiento nativo.