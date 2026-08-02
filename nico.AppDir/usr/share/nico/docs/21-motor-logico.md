# 21. Motor Lógico (Estilo Prolog)

Nico v2.1.0 incluye un **motor de inferencia lógica completo** integrado al lenguaje imperativo, permitiendo combinar programación lógica con imperativa en un mismo programa.

## 📚 Índice

1. [Introducción](#introducción)
2. [Conceptos Básicos](#conceptos-básicos)
3. [Comandos del Motor](#comandos-del-motor)
4. [Built-ins del Motor](#built-ins-del-motor)
5. [BUSCARTODOS](#buscartodos)
6. [Integración con Variables Imperativas](#integración-con-variables-imperativas)
7. [Ejemplos Completos](#ejemplos-completos)
8. [Limitaciones y Consideraciones](#limitaciones-y-consideraciones)

---

## 🎯 Introducción

El motor lógico de Nico implementa un subconjunto del paradigma **Prolog**, permitiendo:

- **Hechos**: Declaraciones de verdades sobre el dominio
- **Reglas**: Relaciones condicionales entre hechos
- **Consultas**: Preguntas al motor para inferir nueva información
- **Backtracking**: Exploración automática de todas las soluciones
- **Unificación**: Matching de patrones con variables
- **Listas**: Estructuras de datos con patrones `[H|T]`

### ¿Por qué un motor lógico en Nico?

La programación lógica es ideal para:
- Sistemas expertos y bases de conocimiento
- Resolución de problemas de satisfacción de restricciones
- Manipulación de listas y árboles
- Algoritmos de búsqueda
- Verificación de propiedades

---

## 📖 Conceptos Básicos

### Hechos

Un **hecho** es una declaración de verdad sobre el dominio:

```nico
AGREGARHECHO("padre(juan, pedro)")
AGREGARHECHO("padre(juan, ana)")
AGREGARHECHO("padre(pedro, luis)")
```

**Sintaxis**: `predicado(argumento1, argumento2, ...)`

- Los predicados deben empezar con **minúscula**
- Los argumentos pueden ser:
  - **Constantes**: `juan`, `pedro`, `luis` (empiezan con minúscula)
  - **Variables**: `X`, `Y`, `Z` (empiezan con mayúscula)
  - **Listas**: `[a, b, c]`, `[H|T]`

### Reglas

Una **regla** define una relación condicional:

```nico
AGREGARREGLA("abuelo(X, Y) SI padre(X, Z), padre(Z, Y)")
```

**Sintaxis**: `cabeza SI cuerpo` o `cabeza :- cuerpo`

- **Cabeza**: El predicado que se define
- **Cuerpo**: Condiciones que deben cumplirse (separadas por comas)
- **Variables**: Se usan para relacionar la cabeza con el cuerpo

### Variables

Las **variables** empiezan con **mayúscula** y representan valores desconocidos:

```nico
// X es una variable
CONSULTAR("padre(X, pedro)")  // ¿Quién es padre de pedro?
```

La **variable anónima** `_` unifica con cualquier cosa sin crear binding:

```nico
CONSULTAR("padre(_, Y)")  // ¿De quién es padre alguien?
```

### Listas

Las **listas** son secuencias de elementos:

```nico
// Lista concreta
AGREGARHECHO("colores([rojo, verde, azul])")

// Patrón cabeza|cola
CONSULTAR("miembro(X, [a, b, c])")  // X = a, X = b, X = c

// Patrón anidado
CONSULTAR("juntar([1, 2], [3, 4], X)")  // X = [1, 2, 3, 4]
```

**Sintaxis de patrones**:
- `[H|T]`: Cabeza `H` y cola `T`
- `[a, b|R]`: Primeros elementos `a, b` y resto `R`
- `[[H|T]|R]`: Lista de listas
---

## 🛠️ Comandos del Motor

### AGREGARHECHO

Agrega un hecho a la base de conocimiento:

```nico
AGREGARHECHO("predicado(argumento1, argumento2)")
```

**Ejemplo**:
```nico
AGREGARHECHO("color(rojo)")
AGREGARHECHO("color(verde)")
AGREGARHECHO("color(azul)")
```

### AGREGARREGLA

Agrega una regla a la base de conocimiento:

```nico
AGREGARREGLA("cabeza SI cuerpo")
AGREGARREGLA("cabeza :- cuerpo")  // Sintaxis alternativa
```

**Ejemplo**:
```nico
AGREGARHECHO("padre(juan, pedro)")
AGREGARHECHO("padre(pedro, luis)")

// Regla: X es abuelo de Y si X es padre de Z y Z es padre de Y
AGREGARREGLA("abuelo(X, Y) SI padre(X, Z), padre(Z, Y)")
```

### CONSULTAR

Realiza una consulta al motor:

```nico
CONSULTAR("predicado(argumento1, argumento2)")
```

**Ejemplos**:
```nico
// Consulta simple
CONSULTAR("color(rojo)")  // Imprime: VERDADERO

// Consulta con variables
CONSULTAR("color(X)")  // Imprime: X = rojo, X = verde, X = azul

// Consulta con múltiples condiciones
CONSULTAR("padre(X, Y), padre(Y, Z)")  // Encadena relaciones
```

### LIMPIARLOGICA

Limpia toda la base de conocimiento:

```nico
LIMPIARLOGICA
```

### LISTARHECHOS / LISTARREGLAS

Muestra la base de conocimiento actual:

```nico
LISTARHECHOS
LISTARREGLAS
```

---

## 🔧 Built-ins del Motor

Los **built-ins** son predicados especiales implementados directamente en el motor lógico de Nico. A diferencia de los hechos y reglas que el usuario define, estos predicados están disponibles automáticamente y permiten operaciones comunes sobre listas, unificación y aritmética.

### MIEMBRO(X, Lista)

Verifica si `X` es miembro de `Lista`. Si `X` es una variable, genera una solución por cada elemento de la lista.

```nico
// Caso 1: X es variable → genera todas las soluciones
CONSULTAR("miembro(X, [a, b, c])")
// Resultado: X = a, X = b, X = c

// Caso 2: X es constante → verifica si pertenece
CONSULTAR("miembro(b, [a, b, c])")
// Resultado: VERDADERO

CONSULTAR("miembro(z, [a, b, c])")
// Resultado: FALSO

// Caso 3: Usar con BUSCARTODOS
BUSCARTODOS("X", "miembro(X, [rojo, verde, azul])", $colores)
ESCRIBIR("Colores: $colores") SALTO
// Resultado: Colores: [rojo, verde, azul]
```

### LONGITUD(Lista, N)

Calcula la longitud de una lista y la unifica con `N`. La lista debe estar instanciada.

```nico
CONSULTAR("longitud([a, b, c], N)")
// Resultado: N = 3

CONSULTAR("longitud([], N)")
// Resultado: N = 0

CONSULTAR("longitud([1, 2, 3, 4, 5], N)")
// Resultado: N = 5
```

### PRIMER(Lista, X)

Obtiene el **primer elemento** de una lista. La lista debe estar instanciada y no vacía.

```nico
CONSULTAR("primer([a, b, c], X)")
// Resultado: X = a

CONSULTAR("primer([10, 20, 30], X)")
// Resultado: X = 10

CONSULTAR("primer([], X)")
// Resultado: FALSO (lista vacía)
```

### ULTIMO(Lista, X)

Obtiene el **último elemento** de una lista. La lista debe estar instanciada y no vacía.

```nico
CONSULTAR("ultimo([a, b, c], X)")
// Resultado: X = c

CONSULTAR("ultimo([10, 20, 30], X)")
// Resultado: X = 30

CONSULTAR("ultimo([], X)")
// Resultado: FALSO (lista vacía)
```

### JUNTAR(L1, L2, L3)

Concatenación **bidireccional** de listas (equivalente a `APPEND` en Prolog). Soporta tres modos de uso:

```nico
// Modo 1: L1 y L2 instanciadas → concatena en L3
CONSULTAR("juntar([1, 2], [3, 4], X)")
// Resultado: X = [1, 2, 3, 4]

// Modo 2: L3 instanciada → genera todas las divisiones posibles
CONSULTAR("juntar(X, Y, [1, 2, 3])")
// Resultado:
// X = [], Y = [1, 2, 3]
// X = [1], Y = [2, 3]
// X = [1, 2], Y = [3]
// X = [1, 2, 3], Y = []

// Modo 3: Usar en reglas personalizadas
AGREGARREGLA("prefijo(P, L) SI juntar(P, _, L)")
CONSULTAR("prefijo([1, 2], [1, 2, 3, 4])")
// Resultado: VERDADERO
```

### REVERSA(L1, L2)

Invierte una lista (equivalente a `REVERSE` en Prolog). Funciona en ambas direcciones.

```nico
// Modo 1: L1 instanciada → invierte en L2
CONSULTAR("reversa([1, 2, 3], X)")
// Resultado: X = [3, 2, 1]

CONSULTAR("reversa([a, b, c, d], X)")
// Resultado: X = [d, c, b, a]

// Lista vacía
CONSULTAR("reversa([], X)")
// Resultado: X = []

// Modo 2: L2 instanciada → invierte en L1
CONSULTAR("reversa(X, [3, 2, 1])")
// Resultado: X = [1, 2, 3]
```

### Predicado = (Unificación Directa)

El predicado `=` permite unificar dos términos directamente. Es especialmente útil para:
- Asignar valores a variables
- Descomponer listas con patrones `[H|T]`
- Verificar igualdad estructural

```nico
// Unificación simple
CONSULTAR("X = 42")
// Resultado: X = 42

CONSULTAR("X = [1, 2, 3]")
// Resultado: X = [1, 2, 3]

// Descomposición de listas con patrón [H|T]
CONSULTAR("[H|T] = [a, b, c]")
// Resultado: H = a, T = [b, c]

// Verificar igualdad
CONSULTAR("5 = 5")
// Resultado: VERDADERO

CONSULTAR("5 = 6")
// Resultado: FALSO

// Usar en reglas
AGREGARREGLA("doble(X, Y) SI Y ES X * 2")
CONSULTAR("doble(5, Y)")
// Resultado: Y = 10
```

### Combinación de Built-ins

Los built-ins se pueden combinar libremente en consultas complejas:

```nico
// Obtener primer elemento y longitud
CONSULTAR("primer([a, b, c], P), longitud([a, b, c], N)")
// Resultado: P = a, N = 3

// Verificar que un elemento está en la lista y obtener longitud
CONSULTAR("miembro(b, [a, b, c]), longitud([a, b, c], N)")
// Resultado: N = 3

// Concatenar e invertir
CONSULTAR("juntar([1, 2], [3, 4], X), reversa(X, Y)")
// Resultado: X = [1, 2, 3, 4], Y = [4, 3, 2, 1]
```

---

## 🔍 BUSCARTODOS

### Sintaxis

```nico
BUSCARTODOS("variable", "consulta", $resultado)
```

Recolecta **todas las soluciones** de una consulta en una lista. Es el equivalente a `FINDALL` en Prolog.

### Parámetros

| Parámetro | Tipo | Descripción |
|-----------|------|-------------|
| `"variable"` | TEXTO | Nombre de la variable a recolectar (con mayúscula, ej: `"X"`) |
| `"consulta"` | TEXTO | Consulta lógica a ejecutar |
| `$resultado` | LISTA | Variable destino donde se guardan las soluciones |

### Ejemplo Básico

```nico
PROGRAMA EjemploBuscartodos
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    AGREGARHECHO("color(rojo)")
    AGREGARHECHO("color(verde)")
    AGREGARHECHO("color(azul)")
    
    // Recolectar todos los colores
    BUSCARTODOS("X", "color(X)", $colores)
    
    ESCRIBIR("Colores: $colores") SALTO
    // Resultado: Colores: [rojo, verde, azul]
FIN PRINCIPAL
FINAL
```

### Ejemplo con Relaciones

```nico
PROGRAMA FamiliaCompleta
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    AGREGARHECHO("padre(juan, pedro)")
    AGREGARHECHO("padre(juan, ana)")
    AGREGARHECHO("padre(pedro, luis)")
    AGREGARHECHO("padre(pedro, maria)")
    
    // Recolectar todos los hijos de juan
    BUSCARTODOS("Y", "padre(juan, Y)", $hijos_juan)
    ESCRIBIR("Hijos de Juan: $hijos_juan") SALTO
    // Resultado: Hijos de Juan: [pedro, ana]
    
    // Recolectar todos los padres
    BUSCARTODOS("X", "padre(X, Y)", $padres)
    ESCRIBIR("Padres: $padres") SALTO
    // Resultado: Padres: [juan, pedro]
FIN PRINCIPAL
FINAL
```

### Ejemplo con Aritmética

```nico
PROGRAMA NumerosPares
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    // Generar números del 1 al 10
    PARA $i DESDE 1 HASTA 10 HACER
        // Verificar si es par
        SI($i MOD 2 IGUAL 0) ENTONCES
            AGREGARHECHO("par($i)")
        FIN SI
    FIN PARA
    
    // Recolectar todos los números pares
    BUSCARTODOS("X", "par(X)", $pares)
    ESCRIBIR("Números pares: $pares") SALTO
    // Resultado: Números pares: [2, 4, 6, 8, 10]
FIN PRINCIPAL
FINAL
```

### Ejemplo con Reglas

```nico
PROGRAMA Abuelos
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    AGREGARHECHO("padre(juan, pedro)")
    AGREGARHECHO("padre(juan, ana)")
    AGREGARHECHO("padre(pedro, luis)")
    AGREGARHECHO("padre(pedro, maria)")
    
    AGREGARREGLA("abuelo(X, Y) SI padre(X, Z), padre(Z, Y)")
    
    // Recolectar todos los abuelos
    BUSCARTODOS("X", "abuelo(X, Y)", $abuelos)
    ESCRIBIR("Abuelos: $abuelos") SALTO
    // Resultado: Abuelos: [juan]
    
    // Recolectar todos los nietos
    BUSCARTODOS("Y", "abuelo(X, Y)", $nietos)
    ESCRIBIR("Nietos: $nietos") SALTO
    // Resultado: Nietos: [luis, maria]
FIN PRINCIPAL
FINAL
```

### Ejemplo con Condiciones Múltiples

```nico
PROGRAMA FiltroComplejo
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    AGREGARHECHO("persona(juan, 25)")
    AGREGARHECHO("persona(ana, 30)")
    AGREGARHECHO("persona(pedro, 17)")
    AGREGARHECHO("persona(maria, 22)")
    
    // Recolectar personas mayores de edad
    BUSCARTODOS("Nombre", "persona(Nombre, Edad), Edad MAYOR 18", $mayores)
    ESCRIBIR("Mayores de edad: $mayores") SALTO
    // Resultado: Mayores de edad: [juan, ana, maria]
FIN PRINCIPAL
FINAL
```

### Consideraciones Importantes

1. **Sin duplicados**: `BUSCARTODOS` elimina automáticamente los valores duplicados en la lista resultante.

2. **Lista vacía**: Si la consulta no tiene soluciones, `$resultado` queda como lista vacía `[]`.

3. **Orden**: Las soluciones se recolectan en el orden en que el motor las encuentra (orden de inserción de hechos/reglas).

4. **Variables libres**: Si la variable a recolectar no aparece en la consulta, el comportamiento es indefinido.

5. **Rendimiento**: Para consultas con muchas soluciones, puede ser más lento que `CONSULTAR` porque acumula todas las soluciones en memoria.

---

## 🔗 Integración con Variables Imperativas

Una de las características más poderosas del motor lógico de Nico es su capacidad de **integrarse con el paradigma imperativo**. Esto permite combinar lo mejor de ambos mundos: la lógica declarativa del motor Prolog con el control de flujo imperativo de Nico.

### Copiar Resultados a Variables Nico

Cuando usás `CONSULTAR`, las variables del motor lógico se copian **automáticamente** a variables imperativas de Nico (sin el prefijo `$`). Esto te permite usar los resultados de una consulta en cualquier parte de tu programa imperativo.

```nico
PROGRAMA Integracion
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    AGREGARHECHO("edad(juan, 25)")
    AGREGARHECHO("edad(ana, 30)")
    
    // Consultar edad de juan
    CONSULTAR("edad(juan, X)")
    
    // Ahora X está disponible como variable imperativa
    SI(X MAYOR 18) ENTONCES
        ESCRIBIR("Juan es mayor de edad") SALTO
    FIN SI
    
    ESCRIBIR("Edad de Juan: $X") SALTO
FIN PRINCIPAL
FINAL
```

**Salida esperada:**
```
X = 25
Juan es mayor de edad
Edad de Juan: 25
```

### Usar Variables Nico en Consultas

Podés usar variables imperativas (con `$`) dentro de consultas lógicas. El motor las resuelve antes de ejecutar la consulta.

```nico
PROGRAMA VariablesMixtas
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    VARIABLE ENTERA $edad_minima = 18
    
    AGREGARHECHO("edad(juan, 25)")
    AGREGARHECHO("edad(pedro, 15)")
    AGREGARHECHO("edad(ana, 20)")
    
    // Consultar personas mayores de edad
    CONSULTAR("edad(Nombre, Edad), Edad MAYOR $edad_minima")
FIN PRINCIPAL
FINAL
```

**Salida esperada:**
```
Nombre = juan, Edad = 25
Nombre = ana, Edad = 20
```

### Combinar BUSCARTODOS con Bucles Imperativos

Podés usar `BUSCARTODOS` para recolectar soluciones y luego procesarlas con bucles imperativos.

```nico
PROGRAMA ProcesarLista
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    AGREGARHECHO("color(rojo)")
    AGREGARHECHO("color(verde)")
    AGREGARHECHO("color(azul)")
    
    // Recolectar todos los colores
    BUSCARTODOS("X", "color(X)", $colores)
    
    // Procesar la lista con un bucle imperativo
    VARIABLE ENTERA $i = 0
    PARA $i DESDE 0 HASTA LONGITUDLISTA($colores) - 1 HACER
        ESCRIBIR("Color $i: $colores[$i]") SALTO
    FIN PARA
FIN PRINCIPAL
FINAL
```

**Salida esperada:**
```
Color 0: rojo
Color 1: verde
Color 2: azul
```

### Ejemplo Completo: Sistema de Recomendación

Este ejemplo muestra cómo combinar motor lógico con imperativo para crear un sistema de recomendación simple.

```nico
PROGRAMA Recomendacion
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    // Base de conocimiento: gustos de usuarios
    AGREGARHECHO("gusta(juan, rock)")
    AGREGARHECHO("gusta(juan, jazz)")
    AGREGARHECHO("gusta(ana, pop)")
    AGREGARHECHO("gusta(ana, rock)")
    AGREGARHECHO("gusta(pedro, jazz)")
    AGREGARHECHO("gusta(pedro, clasica)")
    
    // Regla: usuarios con gustos similares
    AGREGARREGLA("gustos_similares(X, Y) SI gusta(X, Genero), gusta(Y, Genero), X DIFERENTE Y")
    
    // Encontrar usuarios con gustos similares a juan
    BUSCARTODOS("Y", "gustos_similares(juan, Y)", $similares)
    
    ESCRIBIR("Usuarios con gustos similares a Juan:") SALTO
    VARIABLE ENTERA $i = 0
    PARA $i DESDE 0 HASTA LONGITUDLISTA($similares) - 1 HACER
        ESCRIBIR("  - $similares[$i]") SALTO
    FIN PARA
FIN PRINCIPAL
FINAL
```

**Salida esperada:**
```
Usuarios con gustos similares a Juan:
  - ana
  - pedro
```

### Consideraciones Importantes

1. **Ámbito de variables**: Las variables lógicas se copian al scope actual. Si la consulta se hace dentro de una función, las variables quedan en el scope de esa función.

2. **Tipos de datos**: Los valores copiados mantienen su tipo original:
   - Números enteros → `ENTERA`
   - Números decimales → `DECIMAL`
   - Textos/átomos → `TEXTO`

3. **Variables no resueltas**: Si una variable lógica no se resuelve (queda libre), no se copia a la tabla de símbolos imperativa.

4. **Múltiples soluciones**: `CONSULTAR` solo copia los valores de la **primera solución**. Para obtener todas las soluciones, usá `BUSCARTODOS`.

5. **Rendimiento**: Las consultas complejas con muchas soluciones pueden ser lentas. Usá el operador de corte `!` cuando solo necesitás la primera solución.

---

## 📝 Ejemplos Completos

### Ejemplo 1: Árbol Genealógico

Un ejemplo clásico de programación lógica: modelar relaciones familiares y hacer consultas sobre ellas.

```nico
PROGRAMA ArbolGenealogico
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    // Hechos: relaciones padre/madre
    AGREGARHECHO("padre(juan, pedro)")
    AGREGARHECHO("padre(juan, ana)")
    AGREGARHECHO("padre(pedro, luis)")
    AGREGARHECHO("padre(pedro, maria)")
    AGREGARHECHO("madre(ana, carlos)")
    
    // Reglas: abuelo y hermano
    AGREGARREGLA("abuelo(X, Y) SI padre(X, Z), padre(Z, Y)")
    AGREGARREGLA("abuelo(X, Y) SI padre(X, Z), madre(Z, Y)")
    AGREGARREGLA("hermano(X, Y) SI padre(Z, X), padre(Z, Y), X DIFERENTE Y")
    
    // Consultas
    ESCRIBIR("=== Abuelos ===") SALTO
    CONSULTAR("abuelo(X, Y)")
    
    ESCRIBIR("") SALTO
    ESCRIBIR("=== Hermanos ===") SALTO
    CONSULTAR("hermano(X, Y)")
    
    ESCRIBIR("") SALTO
    ESCRIBIR("=== Todos los abuelos ===") SALTO
    BUSCARTODOS("X", "abuelo(X, Y)", $abuelos)
    ESCRIBIR("Abuelos: $abuelos") SALTO
FIN PRINCIPAL
FINAL
```

**Salida esperada:**
```
=== Abuelos ===
X = juan, Y = luis
X = juan, Y = maria
X = juan, Y = carlos

=== Hermanos ===
X = pedro, Y = ana
X = ana, Y = pedro
X = luis, Y = maria
X = maria, Y = luis

=== Todos los abuelos ===
Abuelos: [juan]
```

---

### Ejemplo 2: Manipulación de Listas

Demostración de los built-ins del motor lógico para trabajar con listas.

```nico
PROGRAMA ListasAvanzadas
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    ESCRIBIR("=== JUNTAR (Concatenar) ===") SALTO
    CONSULTAR("juntar([1, 2], [3, 4], X)")
    
    ESCRIBIR("") SALTO
    ESCRIBIR("=== REVERSA (Invertir) ===") SALTO
    CONSULTAR("reversa([a, b, c, d], X)")
    
    ESCRIBIR("") SALTO
    ESCRIBIR("=== MIEMBRO ===") SALTO
    CONSULTAR("miembro(X, [rojo, verde, azul])")
    
    ESCRIBIR("") SALTO
    ESCRIBIR("=== LONGITUD ===") SALTO
    CONSULTAR("longitud([1, 2, 3, 4, 5], N)")
    
    ESCRIBIR("") SALTO
    ESCRIBIR("=== PRIMER y ULTIMO ===") SALTO
    CONSULTAR("primer([a, b, c], P)")
    CONSULTAR("ultimo([a, b, c], U)")
    
    ESCRIBIR("") SALTO
    ESCRIBIR("=== JUNTAR bidireccional ===") SALTO
    CONSULTAR("juntar(X, Y, [1, 2, 3])")
FIN PRINCIPAL
FINAL
```

**Salida esperada:**
```
=== JUNTAR (Concatenar) ===
X = [1,2,3,4]

=== REVERSA (Invertir) ===
X = [d,c,b,a]

=== MIEMBRO ===
X = rojo
X = verde
X = azul

=== LONGITUD ===
N = 5

=== PRIMER y ULTIMO ===
P = a
U = c

=== JUNTAR bidireccional ===
X = [], Y = [1,2,3]
X = [1], Y = [2,3]
X = [1,2], Y = [3]
X = [1,2,3], Y = []
```

---

### Ejemplo 3: Aritmética y Comparadores

El motor lógico soporta expresiones aritméticas y comparadores en las consultas.

```nico
PROGRAMA AritmeticaLogica
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    // Números del 1 al 10
    PARA $i DESDE 1 HASTA 10 HACER
        AGREGARHECHO("numero($i)")
    FIN PARA
    
    ESCRIBIR("=== Números mayores que 5 ===") SALTO
    CONSULTAR("numero(X), X MAYOR 5")
    
    ESCRIBIR("") SALTO
    ESCRIBIR("=== Números pares ===") SALTO
    CONSULTAR("numero(X), X MOD 2 IGUAL 0")
    
    ESCRIBIR("") SALTO
    ESCRIBIR("=== Cuadrados perfectos ===") SALTO
    CONSULTAR("numero(X), Y ES X * X, Y MENOR 50")
    
    ESCRIBIR("") SALTO
    ESCRIBIR("=== Recolectar pares ===") SALTO
    BUSCARTODOS("X", "numero(X), X MOD 2 IGUAL 0", $pares)
    ESCRIBIR("Pares: $pares") SALTO
FIN PRINCIPAL
FINAL
```

**Salida esperada:**
```
=== Números mayores que 5 ===
X = 6
X = 7
X = 8
X = 9
X = 10

=== Números pares ===
X = 2
X = 4
X = 6
X = 8
X = 10

=== Cuadrados perfectos ===
X = 1, Y = 1
X = 2, Y = 4
X = 3, Y = 9
X = 4, Y = 16
X = 5, Y = 25
X = 6, Y = 36
X = 7, Y = 49

=== Recolectar pares ===
Pares: [2, 4, 6, 8, 10]
```

### Ejemplo 4: Operador de Corte

El operador de corte `!` detiene el backtracking. Cuando el motor encuentra un `!`, descarta las alternativas pendientes y se queda con la solución actual. Es útil para optimizar consultas donde solo necesitás la primera solución.

```nico
PROGRAMA EjemploCorte
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    AGREGARHECHO("color(rojo)")
    AGREGARHECHO("color(verde)")
    AGREGARHECHO("color(azul)")
    
    // Sin corte: devuelve todos los colores
    ESCRIBIR("=== Sin corte ===") SALTO
    CONSULTAR("color(X)")
    
    // Con corte: solo devuelve el primer color
    ESCRIBIR("") SALTO
    ESCRIBIR("=== Con corte ===") SALTO
    CONSULTAR("color(X), !")
    
    // Ejemplo práctico: obtener el primer hijo
    ESCRIBIR("") SALTO
    ESCRIBIR("=== Primer hijo de Juan ===") SALTO
    AGREGARHECHO("hijo(juan, pedro)")
    AGREGARHECHO("hijo(juan, ana)")
    AGREGARHECHO("hijo(juan, luis)")
    
    CONSULTAR("hijo(juan, X), !")
FIN PRINCIPAL
FINAL
```

**Salida esperada:**
```
=== Sin corte ===
X = rojo
X = verde
X = azul

=== Con corte ===
X = rojo

=== Primer hijo de Juan ===
X = pedro
```

---

### Ejemplo 5: Negación por Fallo

El operador `NO` implementa **negación por fallo**: `NO predicado(X)` es verdadero si `predicado(X)` no puede probarse. Es importante notar que esto **no es negación lógica clásica**, sino "no se puede demostrar".

```nico
PROGRAMA EjemploNegacion
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    AGREGARHECHO("fruta(manzana)")
    AGREGARHECHO("fruta(pera)")
    AGREGARHECHO("fruta(banana)")
    
    AGREGARHECHO("verdura(zanahoria)")
    AGREGARHECHO("verdura(lechuga)")
    AGREGARHECHO("verdura(tomate)")
    
    // Listar todas las verduras
    ESCRIBIR("=== Verduras ===") SALTO
    CONSULTAR("verdura(X)")
    
    // Verificar si algo NO es fruta
    ESCRIBIR("") SALTO
    ESCRIBIR("=== ¿Es zanahoria una fruta? ===") SALTO
    CONSULTAR("NO fruta(zanahoria)")
    
    // Verificar si algo SÍ es fruta
    ESCRIBIR("") SALTO
    ESCRIBIR("=== ¿Es manzana una fruta? ===") SALTO
    CONSULTAR("NO NO fruta(manzana)")
    
    // Buscar verduras que NO sean tomate
    ESCRIBIR("") SALTO
    ESCRIBIR("=== Verduras que NO son tomate ===") SALTO
    CONSULTAR("verdura(X), NO X = tomate")
FIN PRINCIPAL
FINAL
```

**Salida esperada:**
```
=== Verduras ===
X = zanahoria
X = lechuga
X = tomate

=== ¿Es zanahoria una fruta? ===
VERDADERO

=== ¿Es manzana una fruta? ===
VERDADERO

=== Verduras que NO son tomate ===
X = zanahoria
X = lechuga
```

---

### Ejemplo 6: Disyunción en Reglas

La disyunción permite definir reglas con múltiples alternativas usando `O` o `;`. Internamente, el motor convierte una regla disyuntiva en múltiples reglas separadas.

```nico
PROGRAMA EjemploDisyuncion
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    // Regla con disyunción: X es par si X = 0 O X = 2 O X = 4 O X = 6 O X = 8
    AGREGARREGLA("es_par(X) SI X = 0 O X = 2 O X = 4 O X = 6 O X = 8")
    
    // Regla con disyunción usando punto y coma
    AGREGARREGLA("vocal(X) SI X = a ; X = e ; X = i ; X = o ; X = u")
    
    ESCRIBIR("=== Números pares (0-8) ===") SALTO
    CONSULTAR("es_par(X)")
    
    ESCRIBIR("") SALTO
    ESCRIBIR("=== Vocales ===") SALTO
    CONSULTAR("vocal(X)")
    
    ESCRIBIR("") SALTO
    ESCRIBIR("=== Recolectar pares ===") SALTO
    BUSCARTODOS("X", "es_par(X)", $pares)
    ESCRIBIR("Pares: $pares") SALTO
    
    ESCRIBIR("") SALTO
    ESCRIBIR("=== Recolectar vocales ===") SALTO
    BUSCARTODOS("X", "vocal(X)", $vocales)
    ESCRIBIR("Vocales: $vocales") SALTO
    
    // Ejemplo más complejo: clasificación de números
    ESCRIBIR("") SALTO
    ESCRIBIR("=== Clasificación ===") SALTO
    AGREGARREGLA("clasificacion(X, par) SI es_par(X)")
    AGREGARREGLA("clasificacion(X, impar) SI NO es_par(X), X MAYOR 0, X MENOR 9")
    
    CONSULTAR("clasificacion(4, C)")
    CONSULTAR("clasificacion(7, C)")
FIN PRINCIPAL
FINAL
```

**Salida esperada:**
```
=== Números pares (0-8) ===
X = 0
X = 2
X = 4
X = 6
X = 8

=== Vocales ===
X = a
X = e
X = i
X = o
X = u

=== Recolectar pares ===
Pares: [0,2,4,6,8]

=== Recolectar vocales ===
Vocales: [a,e,i,o,u]

=== Clasificación ===
C = par
C = impar
```

---

### Ejemplo 7: Combinación Avanzada - Sistema de Recomendación

Este ejemplo combina múltiples características del motor lógico: hechos, reglas, recursión, corte, negación y `BUSCARTODOS`.

```nico
PROGRAMA SistemaRecomendacion
BLOQUE PRINCIPAL
    LIMPIARLOGICA
    
    // Base de conocimiento: gustos de usuarios
    AGREGARHECHO("gusta(juan, rock)")
    AGREGARHECHO("gusta(juan, jazz)")
    AGREGARHECHO("gusta(ana, pop)")
    AGREGARHECHO("gusta(ana, rock)")
    AGREGARHECHO("gusta(pedro, jazz)")
    AGREGARHECHO("gusta(pedro, clasica)")
    AGREGARHECHO("gusta(maria, rock)")
    AGREGARHECHO("gusta(maria, pop)")
    
    // Regla: usuarios con al menos un gusto en común
    AGREGARREGLA("gustos_comunes(X, Y) SI gusta(X, Genero), gusta(Y, Genero), X DIFERENTE Y")
    
    // Regla: géneros populares (escuchados por más de 2 personas)
    AGREGARREGLA("genero_popular(G) SI gusta(P1, G), gusta(P2, G), gusta(P3, G), P1 DIFERENTE P2, P2 DIFERENTE P3")
    
    ESCRIBIR("=== Usuarios con gustos similares a Juan ===") SALTO
    BUSCARTODOS("Y", "gustos_comunes(juan, Y)", $similares)
    ESCRIBIR("Similares: $similares") SALTO
    
    ESCRIBIR("") SALTO
    ESCRIBIR("=== Géneros populares ===") SALTO
    BUSCARTODOS("G", "genero_popular(G)", $populares)
    ESCRIBIR("Populares: $populares") SALTO
    
    ESCRIBIR("") SALTO
    ESCRIBIR("=== Todos los géneros que le gustan a Juan ===") SALTO
    BUSCARTODOS("G", "gusta(juan, G)", $generos_juan)
    ESCRIBIR("Géneros de Juan: $generos_juan") SALTO
    
    ESCRIBIR("") SALTO
    ESCRIBIR("=== Recomendación para Maria ===") SALTO
    // Recomendar géneros que le gusten a otros pero no a Maria
    AGREGARREGLA("recomendar(U, G) SI gusta(Otro, G), Otro DIFERENTE U, NO gusta(U, G)")
    BUSCARTODOS("G", "recomendar(maria, G)", $recomendaciones)
    ESCRIBIR("Recomendaciones para Maria: $recomendaciones") SALTO
FIN PRINCIPAL
FINAL
```

**Salida esperada:**
```
=== Usuarios con gustos similares a Juan ===
Similares: [ana,pedro,maria]

=== Géneros populares ===
Populares: [rock]

=== Todos los géneros que le gustan a Juan ===
Géneros de Juan: [rock,jazz]

=== Recomendación para Maria ===
Recomendaciones para Maria: [jazz,clasica]
```

---

## ⚠️ Limitaciones y Consideraciones

### Límites del Motor

| Parámetro | Límite | Descripción |
|-----------|--------|-------------|
| Máximo de hechos | 256 | Cantidad máxima de hechos en la base de conocimiento |
| Máximo de reglas | 64 | Cantidad máxima de reglas en la base de conocimiento |
| Máximo de argumentos por predicado | 16 | Argumentos permitidos en un hecho o regla |
| Máximo de bindings por solución | 64 | Variables que pueden quedar ligadas en una solución |
| Máximo de condiciones en regla | 16 | Condiciones en el cuerpo de una regla |
| Máximo de soluciones por consulta | 64 | Soluciones que se recolectan en una sola consulta |
| Longitud máxima de nombre | 63 caracteres | Para predicados, variables y constantes |

### Consideraciones de Rendimiento

1. **Backtracking**: El motor explora todas las soluciones posibles. Para consultas con muchas soluciones, puede ser lento. Usá el operador de corte `!` cuando solo necesitás la primera solución.

2. **Recursión**: Soporta recursión con renombrado automático de variables, pero con límite de profundidad para evitar desbordamiento de stack.

3. **Occurs Check**: Se realiza automáticamente para evitar unificaciones circulares (`X = [X]`), lo que agrega overhead. Es necesario para corrección lógica.

4. **Renombrado de variables**: Cada vez que se aplica una regla, las variables se renombran para evitar colisiones entre diferentes aplicaciones de la misma regla.

5. **Unificación de listas**: Los patrones `[H|T]` con cabezas múltiples (ej: `[a, b|R]`) son soportados pero pueden ser más lentos que patrones simples.

### Buenas Prácticas

1. **Usar corte (`!`)** cuando solo necesitás la primera solución. Esto evita backtracking innecesario.

2. **Ordenar condiciones** de más restrictivas a menos restrictivas en el cuerpo de las reglas.

3. **Evitar recursión infinita** verificando condiciones de terminación. Siempre incluir un caso base.

4. **Usar `BUSCARTODOS`** en lugar de múltiples `CONSULTAR` cuando necesitás todas las soluciones en una lista.

5. **Limpiar la base** con `LIMPIARLOGICA` antes de cargar nuevos hechos/reglas si vas a reutilizar el motor.

6. **Usar la variable anónima `_`** cuando no necesitás el valor de un argumento (ej: `padre(_, Y)` para obtener todos los hijos).

7. **Aprovechar la bidireccionalidad** de `JUNTAR` y `REVERSA` para generar soluciones en múltiples direcciones.

### Diferencias con Prolog Estándar

| Característica | Nico | Prolog |
|----------------|------|--------|
| Sintaxis de reglas | `SI` o `:-` | `:-` |
| Negación | `NO` | `\+` |
| Corte | `!` | `!` |
| Listas | `[H\|T]` | `[H\|T]` |
| Variables | Mayúscula (`X`) | Mayúscula (`X`) |
| Constantes | Minúscula (`juan`) | Minúscula (`juan`) |
| Unificación | `=` | `=` |
| Built-ins | `MIEMBRO`, `LONGITUD`, etc. | `member`, `length`, etc. |
| Recolección | `BUSCARTODOS` | `findall` |
| Concatenación | `JUNTAR` | `append` |
| Inversión | `REVERSA` | `reverse` |
| Comillas en consultas | `"consulta"` | `consulta` |
| Disyunción | `O` o `;` | `;` |

### Integración con el Lenguaje Imperativo

El motor lógico de Nico se integra con el lenguaje imperativo de las siguientes formas:

1. **Variables lógicas → variables imperativas**: Los resultados de `CONSULTAR` se copian automáticamente a la tabla de símbolos imperativa.

2. **Variables imperativas → consultas**: Podés usar variables `$var` dentro de consultas lógicas.

3. **`BUSCARTODOS` → lista imperativa**: Las soluciones se guardan en una variable `LISTA TEXTO` que podés recorrer con bucles imperativos.

4. **Combinación libre**: Podés mezclar código imperativo y lógico en el mismo programa sin restricciones.

---

## 🎓 Recursos Adicionales

### Documentación Relacionada

- [02-sintaxis.md](02-sintaxis.md) - Sintaxis general de Nico
- [11-funciones.md](11-funciones.md) - Funciones y subprogramas
- [MANUAL.md](MANUAL.md) - Manual completo del lenguaje

### Ejemplos en el Repositorio

Los siguientes ejemplos están disponibles en la carpeta `ejemplos/logica/`:

- `test01_unificacion.nico` Prueba la unificación básica de términos y variables. 
- `test02_occurs_check.nico` Prueba el occurs check (verificación de ocurrencia). 
- `test03_listas.nico` Prueba manipulación de listas.
- `test04_backtracking.nico` Prueba el backtracking automático.
- `test05_recursion.nico` Prueba recursión con renombrado automático de variables. 
- `test06_corte.nico` Prueba el operador de corte ! .
- `test07_builtins.nico` Prueba los built-ins del motor lógico.
- `test08_aritmetica.nico` Prueba operadores aritméticos y comparadores en consultas lógicas. 
- `test08b_aritmetica.nico` Prueba la evaluación aritmética con ES.
- `test09_patrones_anidados.nico` Prueba patrones anidados como [[H|T]|R] y [a, b|R]. 
- `test10_buscartodos.nico` Prueba el comando BUSCARTODOS (estilo FINDALL de Prolog). 

### Lecturas Recomendadas

Si querés profundizar en programación lógica:

- **"Programming in Prolog"** de Clocksin & Mellish - Introducción clásica
- **"The Art of Prolog"** de Sterling & Shapiro - Enfoque más avanzado
- **"Logic, Programming and Prolog"** de Dahl, Flatman & Wirsing - Fundamentos teóricos

---

## 📄 Licencia

MIT License - Uso Educativo

**Autor**: Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)  
**Asistencia IA**: Qwen (Alibaba Cloud) (Implementación, Debugging y Optimización)  
**Versión**: 2.1.0 (Julio 2026)

---

## 🏁 Conclusión

El motor lógico de Nico v2.1.0 representa un hito importante en la evolución del lenguaje, permitiendo combinar dos paradigmas de programación en un solo entorno:

- **Programación imperativa**: Para algoritmos tradicionales, control de flujo explícito y manipulación directa de datos.
- **Programación lógica**: Para inferencia, búsqueda, patrones y relaciones declarativas.

Esta combinación hace de Nico una herramienta educativa única para aprender ambos paradigmas y entender cómo pueden coexistir y complementarse.

¡Disfrutá programando en Nico! 🚀