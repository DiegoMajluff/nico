# 🗄️ Motor SQL y Persistencia

Nico incluye un motor de base de datos basado en SQLite, con un traductor automático 
de sintaxis en español a SQL estándar. No requiere instalación externa, drivers 
ni configuración de rutas.

## 🔗 Conexión y Ejecución Básica
```nico
// Abrir o crear la base de datos en disco
CONECTARBD("tienda.db")

// Ejecutar comandos DDL/DML (CREATE, INSERT, UPDATE, DELETE)
EJECUTARBD("CREATE TABLE IF NOT EXISTS productos (id INTEGER PRIMARY KEY, nombre TEXT, precio REAL)")
EJECUTARBD("INSERT INTO productos (nombre, precio) VALUES ('Teclado', 1500.50)")

// Cerrar conexión (opcional, Nico la cierra automáticamente al finalizar)
CERRARBD
```

## 🔍 Consultas con Traductor Automático

| Sintaxis Nico | Equivalente SQL |
|---------------|-----------------|
| TRAER | SELECT |
| DE | FROM |
| DONDE | WHERE |
| COMO | AS |
| UNIR | INNER JOIN |
| UNIRIZQ | LEFT JOIN |
| SOBRE | ON |
| AGRUPAR POR | GROUP BY |
| ORDENAR POR | ORDER BY |

### Ejemplo Completo: JOIN con Alias
```nico
CONSULTARBD("TRAER c.nombre, p.precio DE categorias COMO c UNIR productos COMO p SOBRE c.id = p.id_categoria ORDENAR POR c.nombre", 1)

MIENTRAS(SIGUIENTEFILABD()) HACER
    ESCRIBIR("Categoría: $BDCOL1 | Precio: $BDCOL2") SALTO
FIN MIENTRAS

CERRARCONSULTABD
```
💡 `$BDCOL1`, `$BDCOL2`, etc. acceden a las columnas en el orden exacto de la consulta. El número `1` en `CONSULTARBD` identifica el cursor (útil si mantenés múltiples consultas abiertas).

## ⚠️ Persistencia y Buenas Prácticas
- ✅ Los archivos `.db` se guardan en disco y sobreviven entre ejecuciones.
- ✅ Usá `CREATE TABLE IF NOT EXISTS` para evitar errores en inicializaciones repetidas.
- ⚠️ Evitá `DELETE FROM tabla` o `DROP TABLE` en scripts de producción a menos que sea intencional.
- 🧪 Para entornos de prueba, podés usar `EJECUTARBD("DELETE FROM tabla")` para limpiar datos antes de ejecutar.

## 🔑 Foreign Keys (Claves Foráneas)
Nico respeta el esquema nativo de SQLite. Si definís `FOREIGN KEY`, el panel web las detecta automáticamente y las muestra como relaciones visuales entre tablas, sin configuración extra.

## 🧩 Múltiples Consultas (Opcional)
```nico
CONSULTARBD("TRAER nombre FROM usuarios", 1)
CONSULTARBD("TRAER titulo FROM libros", 2)

MIENTRAS(SIGUIENTEFILABD(1)) HACER
    ESCRIBIR("Usuario: $BDCOL1") SALTO
FIN MIENTRAS
CERRARCONSULTABD

MIENTRAS(SIGUIENTEFILABD(2)) HACER
    ESCRIBIR("Libro: $BDCOL1") SALTO
FIN MIENTRAS
CERRARCONSULTABD
```
(Si no se pasa ID, Nico usa el cursor por defecto `0`)