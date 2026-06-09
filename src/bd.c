/*
 * Nico v1.1.1 - Intérprete Educativo de Scripting en Español
 * @file:         bd.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Módulo de integración con bases de datos SQLite. Gestiona el
 *                ciclo de vida de conexiones, ejecución segura de consultas SQL,
 *                bind de parámetros, manejo de transacciones y exposición de
 *                resultados al runtime de Nico.
 *                
 *                INCLUYE: Traductor Español Puro → SQL Estándar robusto con 
 *                tokenización segura, soporte completo de JOINs y ordenamiento 
 *                estricto de keywords.
 */
#include "nico.h"
#include <string.h>
#include <stdio.h>

// DECLARACIONES FORWARD
static void transformar_consulta_nico(const char *nico_sql, char *sql_std, size_t max_len);

ContextoBD ctx_bd = {0};

// COMANDOS BÁSICOS DE CONEXIÓN
int cmd_conectarbd(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea_actual;
    if (ctx_bd.conectado) { ctx->linea_num++; return 0; }
    
    const char *p = strchr(linea, '(');
    if (!p) { fprintf(stderr, "Error línea %d: CONECTARBD requiere paréntesis.\n", linea_actual); ctx->linea_num++; return -1; }
    p++; while (*p == ' ' || *p == '\t') p++;
    if (*p == '"') p++;
    
    char ruta[256] = ""; int i = 0;
    while (*p && *p != '"' && i < 255) ruta[i++] = *p++;
    ruta[i] = '\0';
    
    if (sqlite3_open(ruta, &ctx_bd.db) != SQLITE_OK) {
        snprintf(ctx_bd.error_msg, sizeof(ctx_bd.error_msg), "No se pudo abrir BD: %s", sqlite3_errmsg(ctx_bd.db));
        fprintf(stderr, "Error BD: %s.\n", ctx_bd.error_msg);
        ctx_bd.conectado = 0; ctx->linea_num++; return -1;
    }
    ctx_bd.conectado = 1; ctx_bd.consulta_abierta = 0; ctx_bd.stmt = NULL;
    ctx->linea_num++; return 0;
}

int cmd_cerrarbd(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea; (void)linea_actual;
    if (ctx_bd.stmt) sqlite3_finalize(ctx_bd.stmt);
    if (ctx_bd.db) sqlite3_close(ctx_bd.db);
    memset(&ctx_bd, 0, sizeof(ContextoBD));
    ctx->linea_num++; return 0;
}

// EJECUTARBD con Bind Seguro y Transformación
int cmd_ejecutarbd(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea_actual;
    if (!ctx_bd.conectado) { fprintf(stderr, "Error: BD no conectada.\n"); ctx->linea_num++; return -1; }

    const char *p = strchr(linea, '(');
    if (!p) { fprintf(stderr, "Error línea %d: EJECUTARBD requiere paréntesis.\n", linea_actual); ctx->linea_num++; return -1; }
    p++; while (*p == ' ' || *p == '\t') p++;
    if (*p != '"') { fprintf(stderr, "Error: El SQL debe ir entre comillas.\n"); ctx->linea_num++; return -1; }
    p++;

    char sql[2048] = ""; int i = 0;
    while (*p && *p != '"' && i < 2047) sql[i++] = *p++;
    sql[i] = '\0';
    if (*p == '"') p++; 
    while (*p == ' ' || *p == '\t') p++;

    char sql_standard[4096] = "";
    transformar_consulta_nico(sql, sql_standard, sizeof(sql_standard));

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(ctx_bd.db, sql_standard, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error EJECUTARBD: %s.\n", sqlite3_errmsg(ctx_bd.db));
        ctx->linea_num++; return -1;
    }

    int idx = 1;
    while (*p && *p != ')') {
        while (*p == ' ' || *p == '\t' || *p == ',') p++;
        if (!*p || *p == ')') break;

        char val_str[MAX_LINEA] = ""; int vlen = 0;
        double val_num = 0.0; int es_texto = 0;

        if (*p == '"') {
            p++; while (*p && *p != '"') { if (vlen < MAX_LINEA-1) val_str[vlen++] = *p; p++; }
            if (*p == '"') { p++; es_texto = 1; }
        } else if (*p == '$') {
            p++; char vname[MAX_NOMBRE]; int vn=0;
            while (*p && !isspace((unsigned char)*p) && *p!=',' && *p!=')' && vn<MAX_NOMBRE-1) vname[vn++] = *p++;
            vname[vn]='\0';
            int tidx = buscar_texto_var(vname);
            if (tidx >= 0) { strncpy(val_str, texto_vars[tidx].valor, MAX_LINEA-1); es_texto = 1; }
            else { int nidx = buscar_variable(vname); if (nidx >= 0) val_num = (double)variables[nidx].valor; }
        } else {
            int n=0; while (*p && !isspace((unsigned char)*p) && *p!=',' && *p!=')' && n<MAX_LINEA-1) val_str[n++] = *p++;
            val_str[n]='\0'; val_num = atof(val_str);
        }

        if (es_texto) sqlite3_bind_text(stmt, idx, val_str, -1, SQLITE_TRANSIENT);
        else sqlite3_bind_double(stmt, idx, val_num);
        idx++;
    }

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
        fprintf(stderr, "Error al ejecutar: %s.\n", sqlite3_errmsg(ctx_bd.db));
        ctx->linea_num++; return -1;
    }
    ctx->linea_num++; return 0;
}

// CONSULTARBD con Bind Seguro y Transformación
int cmd_consultarbd(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea_actual;
    if (!ctx_bd.conectado) { fprintf(stderr, "Error: BD no conectada.\n"); ctx->linea_num++; return -1; }
    if (ctx_bd.stmt) { fprintf(stderr, "Error: Hay consulta abierta. Usá CERRARCONSULTABD.\n"); ctx->linea_num++; return -1; }

    const char *p = strchr(linea, '(');
    if (!p) { fprintf(stderr, "Error línea %d: CONSULTARBD requiere paréntesis.\n", linea_actual); ctx->linea_num++; return -1; }
    p++; while (*p == ' ' || *p == '\t') p++;
    if (*p != '"') { fprintf(stderr, "Error: El SQL debe ir entre comillas.\n"); ctx->linea_num++; return -1; }
    p++;

    char sql[2048] = ""; int i = 0;
    while (*p && *p != '"' && i < 2047) sql[i++] = *p++;
    sql[i] = '\0';
    if (*p == '"') p++; 
    while (*p == ' ' || *p == '\t') p++;

    char sql_standard[4096] = "";
    transformar_consulta_nico(sql, sql_standard, sizeof(sql_standard));

    if (sqlite3_prepare_v2(ctx_bd.db, sql_standard, -1, &ctx_bd.stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error CONSULTARBD: %s.\n", sqlite3_errmsg(ctx_bd.db));
        ctx_bd.stmt = NULL; ctx->linea_num++; return -1;
    }

    int idx = 1;
    while (*p && *p != ')') {
        while (*p == ' ' || *p == '\t' || *p == ',') p++;
        if (!*p || *p == ')') break;

        char val_str[MAX_LINEA] = ""; int vlen = 0;
        double val_num = 0.0; int es_texto = 0;

        if (*p == '"') {
            p++; while (*p && *p != '"') { if (vlen < MAX_LINEA-1) val_str[vlen++] = *p; p++; }
            if (*p == '"') { p++; es_texto = 1; }
        } else if (*p == '$') {
            p++; char vname[MAX_NOMBRE]; int vn=0;
            while (*p && !isspace((unsigned char)*p) && *p!=',' && *p!=')' && vn<MAX_NOMBRE-1) vname[vn++] = *p++;
            vname[vn]='\0';
            int tidx = buscar_texto_var(vname);
            if (tidx >= 0) { strncpy(val_str, texto_vars[tidx].valor, MAX_LINEA-1); es_texto = 1; }
            else { int nidx = buscar_variable(vname); if (nidx >= 0) val_num = (double)variables[nidx].valor; }
        } else {
            int n=0; while (*p && !isspace((unsigned char)*p) && *p!=',' && *p!=')' && n<MAX_LINEA-1) val_str[n++] = *p++;
            val_str[n]='\0'; val_num = atof(val_str);
        }

        if (es_texto) sqlite3_bind_text(ctx_bd.stmt, idx, val_str, -1, SQLITE_TRANSIENT);
        else sqlite3_bind_double(ctx_bd.stmt, idx, val_num);
        idx++;
    }

    ctx_bd.consulta_abierta = 1;
    ctx->linea_num++; return 0;
}

int cmd_cerrarconsultabd(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea; (void)linea_actual;
    if (ctx_bd.stmt) { sqlite3_finalize(ctx_bd.stmt); ctx_bd.stmt = NULL; }
    ctx_bd.consulta_abierta = 0;
    ctx->linea_num++; return 0;
}

// TRANSACCIONES
static int bd_exec_sql(const char *sql) {
    char *err = NULL;
    int rc = sqlite3_exec(ctx_bd.db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) { fprintf(stderr, "Error BD Transacción: %s.\n", err); sqlite3_free(err); return -1; }
    return 0;
}
int cmd_iniciartransaccion(const char *l, CtxBloque *ctx, int la) { (void)l; (void)la; if (!ctx_bd.conectado) { ctx->linea_num++; return -1; } int r = bd_exec_sql("BEGIN"); ctx->linea_num++; return r; }
int cmd_confirmartransaccion(const char *l, CtxBloque *ctx, int la) { (void)l; (void)la; if (!ctx_bd.conectado) { ctx->linea_num++; return -1; } int r = bd_exec_sql("COMMIT"); ctx->linea_num++; return r; }
int cmd_deshacertransaccion(const char *l, CtxBloque *ctx, int la) { (void)l; (void)la; if (!ctx_bd.conectado) { ctx->linea_num++; return -1; } int r = bd_exec_sql("ROLLBACK"); ctx->linea_num++; return r; }

// ACCESO A RESULTADOS (SIGUIENTEFILABD)
double func_siguientefilabd(int *exito) {
    if (!ctx_bd.stmt || !ctx_bd.consulta_abierta) { *exito = 0; return 0.0; }
    
    int rc = sqlite3_step(ctx_bd.stmt);
    if (rc == SQLITE_ROW) {
        ctx_bd.col_count = sqlite3_column_count(ctx_bd.stmt);
        
        int idx_c = buscar_variable("BDCOL_CANT");
        if (idx_c >= 0) variables[idx_c].valor = ctx_bd.col_count;
        else agregar_variable("BDCOL_CANT", ctx_bd.col_count);

        for (int c = 0; c < ctx_bd.col_count; c++) {
            char var_name[32];
            snprintf(var_name, sizeof(var_name), "BDCOL%d", c + 1);
            int type = sqlite3_column_type(ctx_bd.stmt, c);

            if (type == SQLITE_TEXT) {
                const unsigned char *txt = sqlite3_column_text(ctx_bd.stmt, c);
                int tidx = buscar_texto_var(var_name);
                if (tidx >= 0) {
                    strncpy(texto_vars[tidx].valor, (const char*)txt, MAX_TEXTO_LEN - 1);
                    texto_vars[tidx].valor[MAX_TEXTO_LEN - 1] = '\0';
                } else {
                    agregar_texto_var(var_name, (const char*)txt);
                }
                int nidx = buscar_variable(var_name);
                if (nidx >= 0) variables[nidx].valor = 0.0;
            } else {
                double val = sqlite3_column_double(ctx_bd.stmt, c);
                int nidx = buscar_variable(var_name);
                if (nidx >= 0) variables[nidx].valor = (int)val;
                else agregar_variable(var_name, (int)val);
                
                int tidx = buscar_texto_var(var_name);
                if (tidx >= 0) {
                    texto_vars[tidx].valor[0] = '\0';
                    texto_vars[tidx].nombre[0] = '\0';
                }
            }
        }
        *exito = 1; return 1.0;
    }
    *exito = 1; return 0.0;
}

// =========================================================
//  TRADUCTOR DE CONSULTAS: Español Puro → SQL Estándar
//   - Verifica límites de palabra estrictos
//   - Mantiene espacios intactos
//   - Orden crítico: de más específico a general
// =========================================================
static void reemplazar_seguro(char *str, const char *vieja, const char *nueva) {
    if (!str || !vieja || !nueva) return;
    size_t lv = strlen(vieja), ln = strlen(nueva);
    if (lv == 0) return;

    char *p = str;
    while ((p = strstr(p, vieja)) != NULL) {
        int antes_ok = (p == str) || 
                       (*(p-1) == ' ' || *(p-1) == '\t' || *(p-1) == '\n' || 
                        *(p-1) == '(' || *(p-1) == ',' || *(p-1) == ';');
        int despues_ok = (*(p+lv) == '\0') || 
                         (*(p+lv) == ' ' || *(p+lv) == '\t' || *(p+lv) == '\n' || 
                          *(p+lv) == ')' || *(p+lv) == ',' || *(p+lv) == ';');

        if (antes_ok && despues_ok) {
            memmove(p + ln, p + lv, strlen(p + lv) + 1);
            memcpy(p, nueva, ln);
            p += ln;
        } else {
            p++;
        }
    }
}

static void transformar_consulta_nico(const char *nico_sql, char *sql_std, size_t max_len) {
    if (!nico_sql || !sql_std || max_len == 0) { sql_std[0] = '\0'; return; }
    
    strncpy(sql_std, nico_sql, max_len - 1); sql_std[max_len-1] = '\0';
    char *src = sql_std, *dst = sql_std;
    int prev_space = 1;
    while (*src) {
        if (*src == ' ' || *src == '\t' || *src == '\n' || *src == '\r') {
            if (!prev_space) { *dst++ = ' '; prev_space = 1; }
        } else {
            *dst++ = *src; prev_space = 0;
        }
        src++;
    }
    *dst = '\0';
    while (dst > sql_std && *(dst-1) == ' ') *(--dst) = '\0';

    reemplazar_seguro(sql_std, "ORDENAR POR", "ORDER BY");
    reemplazar_seguro(sql_std, "AGRUPAR POR", "GROUP BY");
    reemplazar_seguro(sql_std, "DISTINTO", "DISTINCT");
    reemplazar_seguro(sql_std, "UNIRIZQ", "LEFT JOIN");
    reemplazar_seguro(sql_std, "UNIRDER", "RIGHT JOIN");
    reemplazar_seguro(sql_std, "UNIR", "INNER JOIN");
    reemplazar_seguro(sql_std, "COMO", "AS");
    reemplazar_seguro(sql_std, "CON", "ON");
    reemplazar_seguro(sql_std, "SOBRE", "ON");
    reemplazar_seguro(sql_std, "DONDE", "WHERE");
    reemplazar_seguro(sql_std, "DE", "FROM");
    reemplazar_seguro(sql_std, "Y", "AND");
    reemplazar_seguro(sql_std, "O", "OR");
    reemplazar_seguro(sql_std, "TRAER", "SELECT");

    src = sql_std; dst = sql_std; prev_space = 1;
    while (*src) {
        if (*src == ' ') { if (!prev_space) { *dst++ = ' '; prev_space = 1; } }
        else { *dst++ = *src; prev_space = 0; }
        src++;
    }
    *dst = '\0';
}
