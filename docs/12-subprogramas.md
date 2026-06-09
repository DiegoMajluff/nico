# 🔧 Subprogramas: Organización de Código con Scope Global

Los `SUBPROGRAMA` en Nico son bloques de código reutilizables diseñados para organizar lógica sin abultar el `BLOQUE PRINCIPAL`. Operan siempre sobre variables globales, sin parámetros ni valores de retorno.

> 💡 **Diferencia clave con FUNCION**: Los subprogramas organizan y mutan estado global sin retornos. Las funciones calculan y retornan un valor con aislamiento de scope.

---

## 📐 Sintaxis de Declaración y Llamada
```nico
// Declaración (fuera de BLOQUE PRINCIPAL)
SUBPROGRAMA NombreDelSubprograma
    // Código que opera sobre variables globales
    // Puede leer/modificar cualquier VARIABLE declarada a nivel de PROGRAMA
FIN SUBPROGRAMA

// Llamada (dentro de BLOQUE PRINCIPAL u otro SUBPROGRAMA)
LLAMAR A NombreDelSubprograma
```

---

## 📋 Características Clave

| Característica | Comportamiento en Nico | Nota |
|----------------|------------------------|------|
| Parámetros | ❌ No soportados | La sintaxis es `SUBPROGRAMA Nombre`, sin `($args)` |
| Retorno | ❌ No retorna valor | Es un procedimiento, no una función |
| Scope de variables | 🌐 Global siempre | Accede y modifica variables declaradas en el nivel del `PROGRAMA` |
| Variables locales | ❌ No existen dentro | Todo `$var` referenciada es la global |
| Llamada | `LLAMAR A Nombre` | Sin paréntesis, sin argumentos |

---

## 🧪 Ejemplo Mínimo Validado
```nico
PROGRAMA TestSubs
    VARIABLE ENTERA $x = 10

SUBPROGRAMA Saludar
    ESCRIBIR("¡Hola desde el Subprograma!") SALTO
    ESCRIBIR("El valor de \$x es: $x") SALTO  // Lee variable global
FIN SUBPROGRAMA

SUBPROGRAMA Sumar
    RESULTADO EN $x = $x + 5  // Modifica variable global
FIN SUBPROGRAMA

BLOQUE PRINCIPAL
    ESCRIBIR("\nInicio del programa") SALTO
    LLAMAR A Saludar          // Imprime: $x = 10
    LLAMAR A Sumar            // $x ahora es 15
    ESCRIBIR("Después de sumar, \$x es: $x\n") SALTO  // Imprime: 15
FIN PRINCIPAL
FINAL
```

---

## ⚠️ Notas Técnicas Críticas
- **Scope global explícito:** Cualquier `$variable` usada dentro de un `SUBPROGRAMA` refiere a la declaración global del `PROGRAMA`. No hay shadowing ni variables locales.
- **Efectos secundarios intencionales:** La modificación de variables globales es el mecanismo de comunicación entre subprogramas. Documentá qué variables lee/modifica cada uno.
- **Orden de declaración:** Los `SUBPROGRAMA` deben declararse antes del `BLOQUE PRINCIPAL` (o antes de su primera llamada, según tu build).
- **Sin parámetros = sin aislamiento:** Si dos subprogramas modifican la misma variable global, el orden de llamada afecta el resultado. Usá con precaución en lógica compleja.
- **Recursión:** Un `SUBPROGRAMA` puede llamar a otro, pero la recursión directa/indirecta puede causar comportamientos indefinidos si no se gestiona manualmente con flags globales.

---

## 🎯 Buenas Prácticas
```nico
// ✅ Documentar qué variables globales usa cada subprograma
// SUBPROGRAMA: ActualizarUI
//   Lee: $mensaje, $estado
//   Modifica: (ninguna, solo imprime)
SUBPROGRAMA ActualizarUI
    CURSOR(1, 1)
    COLORTEXTO(verde)
    ESCRIBIR("Estado: $estado") SALTO
    ESCRIBIR("Mensaje: $mensaje") SALTO
    RESETCOLOR
FIN SUBPROGRAMA

// ✅ Agrupar lógica relacionada en subprogramas pequeños
SUBPROGRAMA InicializarJuego
    ASIGNAR EN $vidas = 3
    ASIGNAR EN $puntos = 0
    COPIARTEXTO($mensaje, "¡Nuevo juego!")
FIN SUBPROGRAMA

// ✅ Usar flags globales para controlar flujo entre subprogramas
VARIABLE ENTERA $game_over = 0

SUBPROGRAMA VerificarDerrota
    SI($vidas IGUAL 0) ENTONCES
        ASIGNAR EN $game_over = 1
        COPIARTEXTO($mensaje, "Game Over")
    FIN SI
FIN SUBPROGRAMA

BLOQUE PRINCIPAL
    LLAMAR A InicializarJuego
    MIENTRAS($game_over IGUAL 0) HACER
        // ... lógica del juego ...
        LLAMAR A VerificarDerrota
        LLAMAR A ActualizarUI
    FIN MIENTRAS
FIN PRINCIPAL
```

---

## 🔄 Diferencias: SUBPROGRAMA vs FUNCION

| Aspecto | SUBPROGRAMA | FUNCION |
|---------|-------------|---------|
| Propósito | Organizar código, evitar repetición | Calcular y retornar un valor |
| Parámetros | ❌ No | ✅ Sí (por valor) |
| Retorno | ❌ No | ✅ Sí (`RETORNAR <expr>`) |
| Scope | 🌐 Global siempre | 🔒 Local + acceso a globales |
| Variables locales | ❌ No | ✅ Sí |
| Llamada | `LLAMAR A Nombre` | `$resultado = nombre($args)` |

---

## 📌 Cuándo Usar SUBPROGRAMA vs FUNCION

| Escenario | Recomendación | Razón |
|-----------|--------------|-------|
| Agrupar 5-10 líneas de UI repetida | ✅ SUBPROGRAMA | Sin overhead de parámetros, scope global conveniente |
| Calcular un valor a partir de inputs | ✅ FUNCION | Aislamiento, reusabilidad con distintos argumentos |
| Modificar múltiples variables de estado | ✅ SUBPROGRAMA | Acceso directo a globales, sin pasar por referencia |
| Lógica pura sin efectos secundarios | ✅ FUNCION | Testeable, predecible, sin dependencias ocultas |
| Inicialización / configuración global | ✅ SUBPROGRAMA | Diseñado para operar sobre estado global |

> 💡 **Compatibilidad:** Equivale a `void nombre() { ... }` en C con variables globales `extern`. Funciona idéntico en Linux y Windows.