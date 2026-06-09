# 🌐 Servidor Web y Panel de Administración

Nico incluye un servidor HTTP embebido con panel SPA de administración, endpoint 
de renderizado dinámico y API JSON. No requiere frameworks externos, Node.js 
ni configuración de reverse proxies.

## 🚀 Iniciar el Servidor
El servidor corre en un hilo separado. Debés mantener el programa activo para 
atender peticiones:
```nico
INICIARSERVER(8080)
ESCRIBIR("🌐 Servidor activo en http://localhost:8080") SALTO

MIENTRAS(1) HACER
    ESPERAR(1, SEGUNDOS)
FIN MIENTRAS

DETENERSERVER()
```

## 📂 Endpoints Disponibles

| Ruta | Método | Función |
|------|--------|---------|
| `/admin` | GET | Panel SPA completo: CRUD, búsqueda, paginación, import/export CSV, detección visual de Foreign Keys. |
| `/render?archivo=ruta` | GET | Ejecuta un script .nico y devuelve su salida (ESCRIBIR) como text/html |
| `/api/tablas` | GET | Lista JSON de todas las tablas en la BD conectada. |
| `/api/schema?tabla=X` | GET | Estructura JSON de la tabla X (columnas, tipos, PKs, FKs). |

## 🔐 Panel de Administración (/admin)
- **Acceso:** `http://localhost:8080/admin`
- **Credenciales:** Usuario `admin` | Clave `nico2026`
- **Características:**
  - ✅ CRUD completo (Crear, Leer, Editar, Eliminar registros)
  - 🔍 Búsqueda por texto libre y filtros por columna
  - 📄 Paginación automática (configurable)
  - 📥📤 Import/Export de datos en formato `.csv`
  - 🔗 Detección automática de Foreign Keys y relaciones visuales
  - 🍪 Sesión basada en cookies HTTP (vencimiento: 24h o cierre de navegador)

> 💡 El panel se conecta a la BD que el script haya abierto con `CONECTARBD()`. Si no hay BD activa, mostrará un mensaje de configuración.

## 🎨 Renderizado Dinámico (/render)
Permite generar páginas HTML/CSS/JS desde scripts Nico. Ideal para prototipos, dashboards o generación de reportes.

**Ejemplo de script (`mi_pagina.nico`):**
```nico
ESCRIBIR("<!DOCTYPE html><html><head><meta charset='utf-8'><style>body{font-family:sans-serif;padding:30px}</style></head><body>") SALTO
ESCRIBIR("<h1>Reporte Generado</h1>") SALTO
ESCRIBIR("<p>¡Hola, mundo!</p>") SALTO
ESCRIBIR("</body></html>") SALTO
```
**Acceder:** `http://localhost:8080/render?archivo=mi_pagina.nico`

### ⚠️ Reglas Críticas para /render
- ❌ **NO** incluyas `INICIARSERVER` en el script renderizado. El endpoint ya corre en un proceso hijo; intentar iniciar otro servidor causará conflicto de puerto.
- ✅ Usá `ESCRIBIR` para todo el HTML/CSS/JS. El endpoint captura stdout, limpia secuencias ANSI y lo sirve como `text/html`.
- 📁 **Rutas relativas:** El `archivo=` se resuelve desde la carpeta donde ejecutaste `nico.exe` o `./nico`.
- 🔒 **Seguridad:** No validar parámetros de URL en el script renderizado. El endpoint solo acepta rutas de archivos locales.

## 📡 API JSON (/api/...)
Diseñada para integración con frontend externo o scripts de terceros.

```bash
# Listar tablas
curl http://localhost:8080/api/tablas

# Ver esquema de una tabla
curl http://localhost:8080/api/schema?tabla=usuarios
```

**Respuesta típica:**
```json
{
  "tablas": ["usuarios", "productos", "ventas"],
  "schema": {
    "id": "INTEGER PRIMARY KEY",
    "nombre": "TEXT",
    "email": "TEXT UNIQUE",
    "fk_categoria": "INTEGER REFERENCES categorias(id)"
  }
}
```

## 🛠️ Troubleshooting Rápido

| Síntoma | Causa probable | Solución |
|---------|---------------|----------|
| `/render` muestra página en blanco | Script no imprime HTML válido o ruta incorrecta | Verificá que el archivo exista y use `ESCRIBIR("<html>...")` |
| `bind falló: Address already in use` | Puerto 8080 ocupado | Cambiá a `INICIARSERVER(9090)` o cerrá el proceso anterior |
| Panel `/admin` no carga JS/CSS | Caché del navegador o bloqueo de recursos | Ctrl+F5 (hard refresh) o verificá que el servidor no esté filtrando `.css/.js` |
| Alta CPU/ventilador acelerado | Polling loop muy corto (`select` timeout 100ms) | Normal en desarrollo. En producción, aumentá `ESPERAR(1, SEGUNDOS)` |

## 📌 Notas de Arquitectura
- El servidor usa `select()` + hilos POSIX (`pthread`) para concurrencia ligera.
- Cada petición `/render` spawnea un nuevo proceso `nico` hijo, captura su salida y la devuelve.
- **Sin dependencias externas:** todo corre en un solo binario de ~200-500 KB.
- Diseñado para entornos educativos, IoT ligero y prototipado rápido. No reemplaza servidores de producción bajo carga.