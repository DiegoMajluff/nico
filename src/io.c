/*
 * Nico v1.1.1 - Intérprete Educativo de Scripting en Español
 * @file:         io.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Operaciones de entrada/salida y efectos. Implementa ESCRIBIR, LEER,
 *                manejo de archivos, cursor, colores y despacho de comandos de I/O.
 */
#define _DEFAULT_SOURCE
#include "nico.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h> 
#include <unistd.h>
#include <ctype.h>

// CROSS-PLATFORM 
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0A00
    #endif 
#else
    #include <poll.h>
    #include <unistd.h>
    #include <termios.h>
    #include <sys/ioctl.h>
#endif

#ifdef _WIN32
    static HANDLE h_stdin_win = INVALID_HANDLE_VALUE;
    static HANDLE h_stdout_win = INVALID_HANDLE_VALUE;
    static DWORD orig_mode_in = 0;
    static DWORD orig_mode_out = 0;
    static int win_console_saved = 0;
#else
    static struct termios orig_termios;
    static int termios_guardado = 0;
#endif
static int raw_init = 0;

void restaurar_terminal_completa(void);

// TIEMPOMS: Helper cross-platform para obtener milisegundos
static long long obtener_tiempo_ms(void) {
#ifdef _WIN32
    static LARGE_INTEGER freq = {0};
    LARGE_INTEGER count;
    
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }
    if (freq.QuadPart != 0 && QueryPerformanceCounter(&count)) {
        return (long long)((count.QuadPart * 1000LL) / freq.QuadPart);
    }
    return (long long)GetTickCount64();
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (long long)(ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL);
    }
    return (long long)time(NULL) * 1000LL;
#endif
}

int iniciar_terminal_raw(void);

// FUNCIONES DE ARCHIVOS
int procesar_usararchivo(const char *nombre_archivo, const char *modo_str, const char *nombre_variable) {
    char modo_c[5] = "";
    
    if (strcmp(modo_str, "ESCRITURA") == 0) strcpy(modo_c, "w");
    else if (strcmp(modo_str, "AGREGAR") == 0) strcpy(modo_c, "a");
    else if (strcmp(modo_str, "LECTURA") == 0) strcpy(modo_c, "r");
    else if (strcmp(modo_str, "LECTOESCRITURA") == 0) strcpy(modo_c, "r+");
    else {
        fprintf(stderr, "Error: Modo debe ser ESCRITURA, AGREGAR, LECTURA o LECTOESCRITURA.\n");
        return -1;
    }
    
    FILE *archivo = fopen(nombre_archivo, modo_c);
    if (archivo == NULL) {
        fprintf(stderr, "Error: No se pudo abrir '%s'.\n", nombre_archivo);
        return -1;
    }
    
    int idx = -1;
    if (nombre_variable && strlen(nombre_variable) > 0) {
        for (int k = 0; k < num_variables_archivo; k++) {
            if (strcmp(variables_archivo[k].nombre, nombre_variable) == 0) {
                if (variables_archivo[k].archivo != NULL) fclose(variables_archivo[k].archivo);
                idx = k;
                break;
            }
        }
    }
    
    if (idx < 0) {
        if (num_variables_archivo >= MAX_VARS_ARCHIVO) {
            fprintf(stderr, "Error: Máximo de archivos alcanzado.\n");
            fclose(archivo);
            return -1;
        }
        idx = num_variables_archivo;
        num_variables_archivo++;
    }
    
    if (nombre_variable && strlen(nombre_variable) > 0) {
        strncpy(variables_archivo[idx].nombre, nombre_variable, MAX_NOMBRE - 1);
        variables_archivo[idx].nombre[MAX_NOMBRE - 1] = '\0';
        variables_archivo[idx].nombre[MAX_NOMBRE - 1] = '\0';
    } else {
        strncpy(variables_archivo[idx].nombre, nombre_archivo, MAX_NOMBRE - 1);
        variables_archivo[idx].nombre[MAX_NOMBRE - 1] = '\0';
        variables_archivo[idx].nombre[MAX_NOMBRE - 1] = '\0';
    }
    
    variables_archivo[idx].archivo = archivo;
    variables_archivo[idx].modo = idx;
    return idx;
}

// ABRIRARCHIVO
void procesar_abrirarchivo(const char *argumento) {
    if (!argumento) return;
    
    char nombre_var[MAX_NOMBRE] = "";
    char nombre_archivo[MAX_LINEA] = "";
    char modo_str[20] = "";
    
    const char *ptr = argumento;
    int i = 0;
    
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    if (*ptr == '$') ptr++;
    while (*ptr && *ptr != ',' && i < MAX_NOMBRE - 1) nombre_var[i++] = *ptr++;
    nombre_var[i] = '\0';
    
    const char *coma = strchr(ptr, ',');
    if (!coma) {
        fprintf(stderr, "Error: ABRIRARCHIVO requiere (variable, \"ruta\", MODO).\n");
        return;
    }
    coma++;
    while (*coma == ' ' || *coma == '\t') coma++;
    
    if (*coma != '"') {
        fprintf(stderr, "Error: ABRIRARCHIVO requiere ruta entre comillas.\n");
        return;
    }
    coma++;
    i = 0;
    while (*coma && *coma != '"' && i < MAX_LINEA - 1) nombre_archivo[i++] = *coma++;
    nombre_archivo[i] = '\0';
    
    coma++;
    char *modo_ptr = strstr(coma, "ESCRITURA");
    if (modo_ptr) strcpy(modo_str, "ESCRITURA");
    else {
        modo_ptr = strstr(coma, "AGREGAR");
        if (modo_ptr) strcpy(modo_str, "AGREGAR");
        else {
            modo_ptr = strstr(coma, "LECTURA");
            if (modo_ptr) strcpy(modo_str, "LECTURA");
            else {
                modo_ptr = strstr(coma, "LECTOESCRITURA");
                if (modo_ptr) strcpy(modo_str, "LECTOESCRITURA");
            }
        }
    }
    
    if (strlen(modo_str) == 0) {
        fprintf(stderr, "Error: Modo debe ser ESCRITURA, AGREGAR, LECTURA o LECTOESCRITURA.\n");
        return;
    }
    
    procesar_usararchivo(nombre_archivo, modo_str, nombre_var);
}

// ESCRIBIRARCHIVO
void procesar_escribirarchivo(const char *argumento) {
    if (!argumento) return;
    
    char nombre_var[MAX_NOMBRE] = "";
    const char *ptr = argumento;
    int i = 0;
    
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    while (*ptr && *ptr != ',' && i < MAX_NOMBRE - 1) {
        if (*ptr == '$') ptr++;
        else nombre_var[i++] = *ptr++;
    }
    nombre_var[i] = '\0';
    
    const char *coma = strchr(argumento, ',');
    if (!coma) {
        fprintf(stderr, "Error: ESCRIBIRARCHIVO requiere contenido.\n");
        return;
    }
    coma++;
    while (*coma == ' ' || *coma == '\t') coma++;
    
    if (*coma != '"') {
        fprintf(stderr, "Error: ESCRIBIRARCHIVO requiere texto entre comillas.\n");
        return;
    }
    coma++;
    
    char contenido[MAX_LINEA] = "";
    i = 0;
    while (*coma && *coma != '"' && i < MAX_LINEA - 2) {
        if (*coma == '\\' && *(coma+1)) {
            coma++;
            switch (*coma) {
                case 'n': contenido[i++] = '\n'; break;
                case 't': contenido[i++] = '\t'; break;
                case '\\': contenido[i++] = '\\'; break;
                case '"': contenido[i++] = '"'; break;
                default: contenido[i++] = *coma; break;
            }
        } else {
            contenido[i++] = *coma;
        }
        coma++;
    }
    
    contenido[i++] = '\n';
    contenido[i] = '\0';
    
    for (int k = 0; k < MAX_VARS_ARCHIVO; k++) {
        if (variables_archivo[k].archivo != NULL && 
            strcmp(variables_archivo[k].nombre, nombre_var) == 0) {
            fprintf(variables_archivo[k].archivo, "%s", contenido);
            return;
        }
    }
    
    fprintf(stderr, "Error: Archivo '%s' no está abierto.\n", nombre_var);
}

// LEERARCHIVO
void procesar_leerarchivo(const char *argumento) {
    if (!argumento) return;
    
    char nombre_var_archivo[MAX_NOMBRE] = "";
    char nombre_var_destino[MAX_NOMBRE] = "";
    
    const char *ptr = argumento;
    int i = 0;
    
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    while (*ptr && *ptr != ',' && i < MAX_NOMBRE - 1) {
        if (*ptr == '$') ptr++;
        else nombre_var_archivo[i++] = *ptr++;
    }
    nombre_var_archivo[i] = '\0';
    limpiar_string(nombre_var_archivo);
    
    const char *coma = strchr(argumento, ',');
    if (!coma) {
        fprintf(stderr, "Error: LEERARCHIVO requiere variable destino.\n");
        return;
    }
    
    coma++;
    i = 0;
    while (*coma == ' ' || *coma == '\t') coma++;
    while (*coma && *coma != ')' && i < MAX_NOMBRE - 1) {
        if (*coma == '$') coma++;
        else nombre_var_destino[i++] = *coma++;
    }
    nombre_var_destino[i] = '\0';
    limpiar_string(nombre_var_destino);
    
    int idx_arch = -1;
    for (int k = 0; k < MAX_VARS_ARCHIVO; k++) {
        if (variables_archivo[k].archivo != NULL && 
            strcmp(variables_archivo[k].nombre, nombre_var_archivo) == 0) {
            idx_arch = k;
            break;
        }
    }
    
    if (idx_arch < 0) {
        fprintf(stderr, "Error: Archivo '%s' no está abierto.\n", nombre_var_archivo);
        return;
    }
    
    char buffer[MAX_LINEA];
    if (fgets(buffer, MAX_LINEA, variables_archivo[idx_arch].archivo) == NULL) {
        int idx_txt = buscar_texto_var(nombre_var_destino);
        if (idx_txt >= 0) texto_vars[idx_txt].valor[0] = '\0';
        return;
    }
    
    buffer[strcspn(buffer, "\n\r")] = '\0';
    
    int idx_txt = buscar_texto_var(nombre_var_destino);
    if (idx_txt >= 0) {
        strncpy(texto_vars[idx_txt].valor, buffer, MAX_TEXTO_LEN - 1);
        texto_vars[idx_txt].valor[MAX_TEXTO_LEN - 1] = '\0';
        texto_vars[idx_txt].valor[MAX_TEXTO_LEN - 1] = '\0';
    }
}

// LEERLINEA
void procesar_leerlinea(const char *argumento) {
    if (!argumento) return;
    
    char nombre_var[MAX_NOMBRE] = "";
    const char *ptr = argumento;
    int i = 0;
    
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    while (*ptr && *ptr != ',' && i < MAX_NOMBRE - 1) {
        if (*ptr == '$') ptr++;
        else nombre_var[i++] = *ptr++;
    }
    nombre_var[i] = '\0';
    limpiar_string(nombre_var);
    
    int idx = buscar_variable_archivo(nombre_var);
    if (idx < 0) {
        fprintf(stderr, "Error: Variable de archivo '$%s' no declarada.\n", nombre_var);
        return;
    }
    
    const char *destino = strchr(argumento, ',');
    if (!destino) {
        fprintf(stderr, "Error: LEERLINEA requiere variable destino.\n");
        return;
    }
    destino++;
    while (*destino == ' ' || *destino == '\t') destino++;
    
    char nombre_destino[MAX_NOMBRE] = "";
    i = 0;
    ptr = destino;
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    if (*ptr == '$') ptr++;
    while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) nombre_destino[i++] = *ptr++;
    nombre_destino[i] = '\0';
    
    char linea[MAX_TEXTO_LEN];
    if (!fgets(linea, MAX_TEXTO_LEN, variables_archivo[idx].archivo)) {
        int id = buscar_texto_var(nombre_destino);
        if (id >= 0) texto_vars[id].valor[0] = '\0';
        return;
    }
    
    size_t len = strlen(linea);
    if (len > 0 && linea[len-1] == '\n') linea[len-1] = '\0';
    
    int id = buscar_texto_var(nombre_destino);
    if (id >= 0) {
        strncpy(texto_vars[id].valor, linea, MAX_TEXTO_LEN - 1);
        texto_vars[id].valor[MAX_TEXTO_LEN - 1] = '\0';
        return;
    }
    agregar_texto_var(nombre_destino, linea);
}

// FINARCHIVO
int procesar_finarchivo(const char *argumento) {
    if (!argumento) return 0;
    
    char nombre_var[MAX_NOMBRE] = "";
    const char *ptr = argumento;
    int i = 0;
    
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    while (*ptr && *ptr != ')' && i < MAX_NOMBRE - 1) {
        if (*ptr == '$') ptr++;
        else nombre_var[i++] = *ptr++;
    }
    nombre_var[i] = '\0';
    limpiar_string(nombre_var);
    
    for (int k = 0; k < MAX_VARS_ARCHIVO; k++) {
        if (variables_archivo[k].archivo != NULL && 
            strcmp(variables_archivo[k].nombre, nombre_var) == 0) {
            return feof(variables_archivo[k].archivo) ? 1 : 0;
        }
    }
    
    return 0;
}

// CERRARARCHIVO
void procesar_cerrararchivo(const char *argumento) {
    if (!argumento) return;
    
    char nombre_var[MAX_NOMBRE] = "";
    const char *ptr = argumento;
    int i = 0;
    
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    while (*ptr && *ptr != ')' && i < MAX_NOMBRE - 1) {
        if (*ptr == '$') ptr++;
        else nombre_var[i++] = *ptr++;
    }
    nombre_var[i] = '\0';
    limpiar_string(nombre_var);
    
    int idx = buscar_variable_archivo(nombre_var);
    if (idx < 0) {
        fprintf(stderr, "Error: Variable de archivo '$%s' no declarada.\n", nombre_var);
        return;
    }
    
    if (variables_archivo[idx].archivo) {
        fclose(variables_archivo[idx].archivo);
        variables_archivo[idx].archivo = NULL;
    }
}

// FUNCIONES DE TEXTO
void procesar_funcion_texto(const char *linea) {
    char linea_local[MAX_LINEA];
    strncpy(linea_local, linea, MAX_LINEA - 1);
    linea_local[MAX_LINEA - 1] = '\0';
    limpiar_string(linea_local);
    
    char *apertura = strchr(linea_local, '(');
    char *cierre = strrchr(linea_local, ')');
    if (!apertura || !cierre) {
        fprintf(stderr, "Error: Función de texto requiere paréntesis.\n");
        return;
    }
    
    char nombre_func[MAX_NOMBRE];
    int i = 0;
    char *p = linea_local;
    while (*p && *p != '(' && i < MAX_NOMBRE - 1) {
        if (*p == '$') p++;
        else nombre_func[i++] = *p++;
    }
    nombre_func[i] = '\0';
    limpiar_string(nombre_func);
   
    char *args[MAX_PARAMETROS];
    int num_args = 0;
    p = apertura + 1;
    int dentro_comillas = 0;
    char tipo_comilla = 0;
    
    while (*p && *p != ')' && num_args < MAX_PARAMETROS) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == ')') break;
        
        char arg[MAX_LINEA] = "";
        i = 0;
        int nivel = 0;
        dentro_comillas = 0;
        tipo_comilla = 0;
        
        while (*p && (nivel > 0 || !((*p == ',' || *p == ')') && !dentro_comillas)) && i < MAX_LINEA - 1) {
            if ((*p == '"' || *p == '\'') && *(p-1) != '\\') {
                if (!dentro_comillas) { dentro_comillas = 1; tipo_comilla = *p; }
                else if (*p == tipo_comilla) { dentro_comillas = 0; }
            }
            else if (!dentro_comillas) {
                if (*p == '(') nivel++;
                else if (*p == ')') nivel--;
            }
            arg[i++] = *p++;
        }
        arg[i] = '\0';
        limpiar_string(arg);
        if (strlen(arg) > 0) args[num_args++] = strdup(arg);
        if (*p == ',') p++;
    } 
   
    if (ejecutar_comando_cadena(nombre_func, args, num_args) < 0) {
        fprintf(stderr, "Error: Función de texto '%s' no reconocida o argumentos inválidos.\n", nombre_func);
    }
    
    for (int k = 0; k < num_args; k++) {
        if (args[k]) free(args[k]);
    }
}

// PROCESAR CALCULAR
void procesar_calcular(const char *linea) {
    // Saltar espacios de indentación antes de comparar
    const char *inicio = linea;
    while (*inicio == ' ' || *inicio == '\t') inicio++;

    if (comienza_con(inicio, "COPIARTEXTO") || comienza_con(inicio, "CONCATENARTEXTO") ||
        comienza_con(inicio, "MAYUSCULAS") || comienza_con(inicio, "MINUSCULAS") ||
        comienza_con(inicio, "RECORTARTEXTO") || comienza_con(inicio, "REEMPLAZARTEXTO") ||
        comienza_con(inicio, "ENTEROATEXTO") || comienza_con(inicio, "DECIMALATEXTO") ||
        comienza_con(inicio, "CARACTERATEXTO") || comienza_con(inicio, "REPETIRTEXTO") ||
        comienza_con(inicio, "EXTRAERTEXTO") || comienza_con(inicio, "DIVIDIRTEXTO")) {
        procesar_funcion_texto(linea);
        return;
    }
    char linea_local[MAX_LINEA];
    strncpy(linea_local, linea, MAX_LINEA - 1);
    linea_local[MAX_LINEA - 1] = '\0';
    limpiar_string(linea_local);
    
    const char *ptr = strchr(linea_local, '$');
    if (!ptr) {
        fprintf(stderr, "Error: No se encontró variable '$' en la línea.\n");
        return;
    }

    const char *igual = strchr(ptr, '=');
    
    if (!igual) {
        fprintf(stderr, "Error: Formato de asignación inválido (falta '=').\n");
        return;
    }
    
    char lado_izq[MAX_LINEA];
    int len_izq = igual - ptr;
    if (len_izq <= 0 || len_izq >= MAX_LINEA) {
        fprintf(stderr, "Error: Formato de asignación inválido.\n");
        return;
    }
    strncpy(lado_izq, ptr, len_izq);
    lado_izq[len_izq] = '\0';

    if (strchr(lado_izq, '[')) {
        char *start = lado_izq;
        char *end = lado_izq + strlen(lado_izq) - 1;
        while (*start == ' ' || *start == '\t') start++;
        while (end > start && (*end == ' ' || *end == '\t')) *end-- = '\0';
        if (start != lado_izq) {
            memmove(lado_izq, start, strlen(start) + 1);
        }
    } else {
        limpiar_string(lado_izq);
    }

    char nombre_dest[MAX_NOMBRE] = "";
    const char *p_nombre = lado_izq + 1;
    int i = 0;
    while (es_alnum(*p_nombre) && i < MAX_NOMBRE - 1) nombre_dest[i++] = *p_nombre++;
    nombre_dest[i] = '\0';
   
    // Saltar '=' y espacios iniciales
    const char *rhs = igual + 1;
    while (*rhs == ' ' || *rhs == '\t') rhs++;

    // DETECCIÓN: ¿Es texto? (empieza con " o es $var de texto o es $matriz_texto[...])
    int es_texto = (*rhs == '"');
    if (!es_texto && *rhs == '$') {
        char var_check[MAX_NOMBRE] = "";
        int k = 0; const char *tmp = rhs + 1;
        while (es_alnum(*tmp) && k < MAX_NOMBRE - 1) var_check[k++] = *tmp++;
        var_check[k] = '\0';
    
        // Verificar si es variable de texto simple
        if (buscar_texto_var(var_check) >= 0) {
            es_texto = 1;
        }
        // Si tiene corchetes, verificar si es matriz de TEXTO
        else if (strchr(rhs, '[')) {
            // Buscar en matrices de texto globales
            for (int m = 0; m < num_matrices_texto; m++) {
                if (strcmp(matrices_texto[m].nombre, var_check) == 0) {
                    es_texto = 1;
                    break;
                }
            }
            // Buscar en matrices locales (tipo 6 = TEXTO)
            if (!es_texto && scope_actual >= 0) {
                for (int sc = scope_actual; sc >= 0 && !es_texto; sc--) {
                    ScopeLocal *scp = &scopes_locales[sc];
                    for (int m = 0; m < scp->num_matrices; m++) {
                        if (strcmp(scp->nombres_matrices[m], var_check) == 0 && 
                        scp->tipos_matrices[m] == 6) {
                            es_texto = 1;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (es_texto) {
        // Ruta TEXTO → Delegar a COPIARTEXTO
        char origen_raw[MAX_TEXTO_LEN] = "";
        int j = 0;
        while (*rhs && j < MAX_TEXTO_LEN - 1) origen_raw[j++] = *rhs++;
        origen_raw[j] = '\0';
        while (j > 0 && (origen_raw[j-1] == ' ' || origen_raw[j-1] == '\t')) origen_raw[--j] = '\0';
        
        // COPIARTEXTO espera "$lista[indice]" como primer argumento
        char *args[2] = { lado_izq, origen_raw }; 
        ejecutar_comando_cadena("COPIARTEXTO", args, 2);
        return; // Asignación completada, salir de la función
    }

    // Ruta NUMÉRICA
    char expresion[MAX_LINEA];
    strncpy(expresion, rhs, MAX_LINEA - 1);
    expresion[MAX_LINEA - 1] = '\0';
    limpiar_string(expresion);
    
    int exito_expr;
    double resultado = evaluar_expresion_completa(expresion, &exito_expr);
    if (!exito_expr) {
        fprintf(stderr, "Error: No se pudo evaluar '%s'.\n", expresion);
        return;
    }
    
    const char *corchete1 = strchr(lado_izq, '[');
    if (corchete1) {
        const char *cierre1 = strchr(corchete1 + 1, ']');
        const char *corchete2 = cierre1 ? strchr(cierre1 + 1, '[') : NULL;
        
        if (corchete2) {
            char fila_str[MAX_LINEA] = "", columna_str[MAX_LINEA] = "";
            
            const char *p_cor1 = corchete1 + 1;
            i = 0; int nivel = 1;
            while (*p_cor1 && nivel > 0 && i < MAX_LINEA - 1) {
                if (*p_cor1 == '[') nivel++; else if (*p_cor1 == ']') nivel--;
                if (nivel > 0) fila_str[i++] = *p_cor1;
                p_cor1++;
            }
            fila_str[i] = '\0'; limpiar_string(fila_str);
            
            const char *p_cor2 = corchete2 + 1;
            i = 0; nivel = 1;
            while (*p_cor2 && nivel > 0 && i < MAX_LINEA - 1) {
                if (*p_cor2 == '[') nivel++; else if (*p_cor2 == ']') nivel--;
                if (nivel > 0) columna_str[i++] = *p_cor2;
                p_cor2++;
            }
            columna_str[i] = '\0'; limpiar_string(columna_str);
            
            int efila, ecol;
            double vfila = evaluar_expresion_completa(fila_str, &efila);
            double vcol = evaluar_expresion_completa(columna_str, &ecol);
            int fila = efila ? (int)vfila : atoi(fila_str);
            int col = ecol ? (int)vcol : atoi(columna_str);

            double val_var;
                    
            for (int sc = scope_actual; sc >= 0; sc--) {
                ScopeLocal *scp = &scopes_locales[sc];
                for (int v = 0; v < scp->num_variables; v++) {
                    if (strcmp(scp->variables[v].nombre, nombre_dest) == 0) {
                        switch(scp->variables[v].tipo) {
                            case 0: val_var = (double)scp->variables[v].valor.valor_entero; break;
                            case 1: val_var = (double)scp->variables[v].valor.valor_sin_signo; break;
                            case 2: case 3: val_var = scp->variables[v].valor.valor_decimal; break;
                            case 4: val_var = (double)scp->variables[v].valor.valor_caracter; break;
                            case 5: val_var = (double)scp->variables[v].valor.valor_caracter_sin_signo; break;
                            default: val_var = 0; break;
                        }
                        
                        if (val_var >= 2000000.0 && val_var < 2060000.0) {
                            int ref = (int)(val_var - 2000000.0);
                            int tipo_mat = ref / 10000;
                            int idx_pool = ref % 10000;
                                                    
                            int filas_max = 0, cols_max = 0;
                            switch(tipo_mat) {
                                case 0: filas_max = matrices_enteras[idx_pool].filas; cols_max = matrices_enteras[idx_pool].columnas; break;
                                case 1: case 3: filas_max = matrices_decimales[idx_pool].filas; cols_max = matrices_decimales[idx_pool].columnas; break;
                                case 2: filas_max = matrices_enteras_sin_signo[idx_pool].filas; cols_max = matrices_enteras_sin_signo[idx_pool].columnas; break;
                                case 4: filas_max = matrices_caracter[idx_pool].filas; cols_max = matrices_caracter[idx_pool].columnas; break;
                                case 5: filas_max = matrices_caracter_sin_signo[idx_pool].filas; cols_max = matrices_caracter_sin_signo[idx_pool].columnas; break;
                            }
                            
                            if (fila < 0 || fila >= filas_max || col < 0 || col >= cols_max) {
                                return;
                            }
                            
                            switch(tipo_mat) {
                                case 0: 
                                    matrices_enteras[idx_pool].valores[fila][col] = (int)resultado; 
                                    break;
                                case 1: case 3: 
                                    matrices_decimales[idx_pool].valores[fila][col] = resultado; 
                                    break;
                                case 2: 
                                    matrices_enteras_sin_signo[idx_pool].valores[fila][col] = (unsigned int)resultado; 
                                    break;
                                case 4: 
                                    matrices_caracter[idx_pool].valores[fila][col] = (char)resultado; 
                                    break;
                                case 5: 
                                    matrices_caracter_sin_signo[idx_pool].valores[fila][col] = (unsigned char)resultado; 
                                    break;
                            }
                            return;
                        }
                        goto buscar_local_o_global_mat;
                    }
                }
            }
            
            buscar_local_o_global_mat:;
            int tipo_loc = -1, idx_loc = -1;
            // Limpiar nombre: registrar_matriz_local guarda SIN el '$'
            const char *clean_name = (nombre_dest[0] == '$') ? nombre_dest + 1 : nombre_dest;
            if (buscar_matriz_local(clean_name, &tipo_loc, &idx_loc)) {
                int filas_max = 0, cols_max = 0;
                switch(tipo_loc) {
                    case 0: filas_max = matrices_enteras[idx_loc].filas; cols_max = matrices_enteras[idx_loc].columnas; break;
                    case 1: filas_max = matrices_decimales[idx_loc].filas; cols_max = matrices_decimales[idx_loc].columnas; break;
                    case 3: filas_max = matrices_decimales_sin_signo[idx_loc].filas; cols_max = matrices_decimales_sin_signo[idx_loc].columnas; break;
                    case 2: filas_max = matrices_enteras_sin_signo[idx_loc].filas; cols_max = matrices_enteras_sin_signo[idx_loc].columnas; break;
                    case 4: filas_max = matrices_caracter[idx_loc].filas; cols_max = matrices_caracter[idx_loc].columnas; break;
                    case 5: filas_max = matrices_caracter_sin_signo[idx_loc].filas; cols_max = matrices_caracter_sin_signo[idx_loc].columnas; break;
                }
                if (fila < 0 || fila >= filas_max || col < 0 || col >= cols_max) {
                    return;
                }
    
                switch(tipo_loc) {
                    case 0: matrices_enteras[idx_loc].valores[fila][col] = (int)resultado; return;
                    case 1: matrices_decimales[idx_loc].valores[fila][col] = resultado; return;
                    case 3: matrices_decimales_sin_signo[idx_loc].valores[fila][col] = resultado; return;
                    case 2: matrices_enteras_sin_signo[idx_loc].valores[fila][col] = (unsigned int)resultado; return;
                    case 4: matrices_caracter[idx_loc].valores[fila][col] = (char)resultado; return;
                    case 5: matrices_caracter_sin_signo[idx_loc].valores[fila][col] = (unsigned char)resultado; return;
                }
            }
            if (buscar_matriz_entera(nombre_dest) >= 0) { 
                set_matriz_entera_valor(nombre_dest, fila, col, (int)resultado); return; 
            }

            if (buscar_matriz_decimal(nombre_dest) >= 0) { 
                set_matriz_decimal_valor(nombre_dest, fila, col, resultado); return; 
            }

            if (buscar_matriz_entera_sin_signo(nombre_dest) >= 0) { 
                set_matriz_entera_sin_signo_valor(nombre_dest, fila, col, (unsigned int)resultado); return; 
            }

            if (buscar_matriz_decimal_sin_signo(nombre_dest) >= 0) { 
                set_matriz_decimal_sin_signo_valor(nombre_dest, fila, col, resultado); return; 
            }

            if (buscar_matriz_caracter(nombre_dest) >= 0) { 
                set_matriz_caracter_valor(nombre_dest, fila, col, (char)resultado); return; 
            }

            if (buscar_matriz_caracter_sin_signo(nombre_dest) >= 0) { 
                set_matriz_caracter_sin_signo_valor(nombre_dest, fila, col, (unsigned char)resultado); return; 
            }

            fprintf(stderr, "Error: Matriz '$%s' no declarada.\n", nombre_dest);
            return;
        }
    }    

    corchete1 = strchr(lado_izq, '[');
    if (corchete1) {
        char indice_str[MAX_LINEA] = "";
        const char *p_cor = corchete1 + 1;
        i = 0; int nivel = 1;
        while (*p_cor && nivel > 0 && i < MAX_LINEA - 1) {
            if (*p_cor == '[') nivel++; else if (*p_cor == ']') nivel--;
            if (nivel > 0) indice_str[i++] = *p_cor;
            p_cor++;
        }
        indice_str[i] = '\0'; limpiar_string(indice_str);
        
        int eindice;
        double vindice = evaluar_expresion_completa(indice_str, &eindice);
        int indice = eindice ? (int)vindice : atoi(indice_str);
        
        double val_param = 0;
        
        for (int sc = scope_actual; sc >= 0; sc--) {
            ScopeLocal *scp = &scopes_locales[sc];
            for (int v = 0; v < scp->num_variables; v++) {
                if (strcmp(scp->variables[v].nombre, nombre_dest) == 0) {
                    switch(scp->variables[v].tipo) {
                        case 0: val_param = (double)scp->variables[v].valor.valor_entero; break;
                        case 1: val_param = (double)scp->variables[v].valor.valor_sin_signo; break;
                        case 2: case 3: val_param = scp->variables[v].valor.valor_decimal; break;
                        case 4: val_param = (double)scp->variables[v].valor.valor_caracter; break;
                        case 5: val_param = (double)scp->variables[v].valor.valor_caracter_sin_signo; break;
                        default: val_param = 0; break;
                    }
                    if (val_param >= 1000000.0 && val_param < 1060000.0) {
                        int ref = (int)(val_param - 1000000.0);
                        int tipo_list = ref / 10000;
                        int idx_pool = ref % 10000;
                        switch(tipo_list) {
                            case 0: listas_enteras[idx_pool].valores[indice] = (int)resultado; return;
                            case 1: case 3: listas_decimales[idx_pool].valores[indice] = resultado; return;
                            case 2: listas_enteras_sin_signo[idx_pool].valores[indice] = (unsigned int)resultado; return;
                            case 4: listas_caracter[idx_pool].valores[indice] = (char)resultado; return;
                            case 5: listas_caracter_sin_signo[idx_pool].valores[indice] = (unsigned char)resultado; return;
                        }
                    }
                    goto buscar_local_o_global_list;
                }
            }
        }
        
        buscar_local_o_global_list:;
        int tipo_loc = -1, idx_loc = -1;
        const char *clean_name = (nombre_dest[0] == '$') ? nombre_dest + 1 : nombre_dest;
        if (buscar_lista_local(clean_name, &tipo_loc, &idx_loc)) {  
            switch(tipo_loc) {
                case 0: listas_enteras[idx_loc].valores[indice] = (int)resultado; return;
                case 1: listas_decimales[idx_loc].valores[indice] = resultado; return;
                case 3: listas_decimales_sin_signo[idx_loc].valores[indice] = resultado; return;
                case 2: listas_enteras_sin_signo[idx_loc].valores[indice] = (unsigned int)resultado; return;
                case 4: listas_caracter[idx_loc].valores[indice] = (char)resultado; return;
                case 5: listas_caracter_sin_signo[idx_loc].valores[indice] = (unsigned char)resultado; return;
            }
        }
        
        // HANDLER MATRIZ TEXTO: $mat[f][c] = "texto"
        {
            // 1. Extraer nombre limpio (sin corchetes ni $)
            char nombre_limpio[MAX_NOMBRE] = "";
            int ni = 0;
            const char *src = nombre_dest;
            if (*src == '$') src++;
            while (*src && *src != '[' && ni < MAX_NOMBRE - 1) nombre_limpio[ni++] = *src++;
            nombre_limpio[ni] = '\0';

            // 2. Extraer fila y columna locales
            int fila_loc = 0, col_loc = 0;
            const char *br1 = strchr(nombre_dest, '[');
            if (br1) {
                fila_loc = atoi(br1 + 1);
                const char *br2 = strchr(br1 + 1, '[');
                if (br2) col_loc = atoi(br2 + 1);
            }

            // 3. Buscar matriz respetando scopes
            int idx_mat = buscar_matriz_texto_en_scope(nombre_limpio, scope_actual);
            if (idx_mat >= 0) {
                const char *igual = strchr(linea, '=');
                if (igual) {
                    const char *rhs = igual + 1;
                    while (*rhs == ' ' || *rhs == '\t') rhs++;

                    char valor_texto[MAX_TEXTO_LEN] = "";
                    if (*rhs == '"') {
                        rhs++; int i = 0;
                        while (*rhs && *rhs != '"' && i < MAX_TEXTO_LEN - 1) {
                            if (*rhs == '\\' && *(rhs+1)) {
                                rhs++;
                                switch(*rhs) {
                                    case 'n': valor_texto[i++] = '\n'; break;
                                    case 't': valor_texto[i++] = '\t'; break;
                                    case '\\': valor_texto[i++] = '\\'; break;
                                    case '"': valor_texto[i++] = '"'; break;
                                    case '$': valor_texto[i++] = '$'; break;
                                    default:  valor_texto[i++] = *rhs; break;
                                }
                                rhs++;
                            } else {
                                valor_texto[i++] = *rhs++;
                            }
                        }
                        valor_texto[i] = '\0';

                        // 4. Validar dimensiones y asignar
                        if (fila_loc >= 0 && fila_loc < matrices_texto[idx_mat].filas &&
                            col_loc >= 0 && col_loc < matrices_texto[idx_mat].columnas) {
                            set_matriz_texto_valor(nombre_limpio, fila_loc, col_loc, valor_texto);
                            return;
                        }
                    }
                }
            }
        }
        
        // ALIAS DIRECTO a $lista[indice] = "texto" o $var
        if (strstr(nombre_dest, "[") != NULL) {
            // Buscar '=' en la línea original para extraer el lado derecho
            const char *igual = strchr(linea, '=');
            if (igual) {
                const char *rhs = igual + 1;
                while (*rhs == ' ' || *rhs == '\t') rhs++;
        
                char origen_buf[MAX_TEXTO_LEN] = "";
                int es_texto = 0;
        
                // Caso 1: String literal "..."
                if (*rhs == '"') {
                    rhs++; int i = 0;
                    while (*rhs && *rhs != '"' && i < MAX_TEXTO_LEN - 1) {
                        if (*rhs == '\\' && *(rhs+1)) {
                            rhs++;
                            switch(*rhs) {
                                case 'n': origen_buf[i++] = '\n'; break;
                                case 't': origen_buf[i++] = '\t'; break;
                                case '\\': origen_buf[i++] = '\\'; break;
                                case '"': origen_buf[i++] = '"'; break;
                                case '$': origen_buf[i++] = '$'; break;
                                default:  origen_buf[i++] = *rhs; break;
                            }
                            rhs++;
                        } else {
                            origen_buf[i++] = *rhs++;
                        }
                    }
                    origen_buf[i] = '\0';
                    es_texto = 1;
                } 
                // Caso 2: Variable de texto $otra
                else if (*rhs == '$') {
                    char var_origen[MAX_NOMBRE] = "";
                    int j = 0; const char *tmp = rhs + 1;
                    while (es_alnum(*tmp) && j < MAX_NOMBRE - 1) var_origen[j++] = *tmp++;
                    var_origen[j] = '\0';
            
                    // Buscar en variables globales de texto
                    int idx_var = buscar_texto_var(var_origen);
                    if (idx_var >= 0) {
                        strncpy(origen_buf, texto_vars[idx_var].valor, MAX_TEXTO_LEN - 1);
                        origen_buf[MAX_TEXTO_LEN - 1] = '\0';
                        es_texto = 1;
                    } 
                    // Buscar en scopes locales si aplica
                    else if (scope_actual >= 0) {
                        for (int s = scope_actual; s >= 0; s--) {
                            for (int v = 0; v < scopes_locales[s].num_textos; v++) {
                                if (strcmp(scopes_locales[s].nombres_textos[v], var_origen) == 0) {
                                    int idx_pool = scopes_locales[s].indices_textos[v];
                                    strncpy(origen_buf, texto_vars[idx_pool].valor, MAX_TEXTO_LEN - 1);
                                    origen_buf[MAX_TEXTO_LEN - 1] = '\0';
                                    es_texto = 1; break;
                                }
                            }
                            if (es_texto) break;
                        }
                    }
                }
        
                // Delegar a COPIARTEXTO si es texto
                if (es_texto) {
                    char args_mutables[2][MAX_NOMBRE];
                    strncpy(args_mutables[0], nombre_dest, MAX_NOMBRE - 1); args_mutables[0][MAX_NOMBRE - 1] = '\0';
                    char *args[2] = { args_mutables[0], origen_buf };
                    ejecutar_comando_cadena("COPIARTEXTO", args, 2);
                    return;
                }
            }
        }
        
        if (buscar_lista_entera(nombre_dest) >= 0) { set_lista_entera_valor(nombre_dest, indice, (int)resultado); return; }
        if (buscar_lista_decimal(nombre_dest) >= 0) { set_lista_decimal_valor(nombre_dest, indice, resultado); return; }
        if (buscar_lista_entera_sin_signo(nombre_dest) >= 0) { set_lista_entera_sin_signo_valor(nombre_dest, indice, (unsigned int)resultado); return; }
        if (buscar_lista_decimal_sin_signo(nombre_dest) >= 0) { set_lista_decimal_sin_signo_valor(nombre_dest, indice, resultado); return; }
        if (buscar_lista_caracter(nombre_dest) >= 0) { set_lista_caracter_valor(nombre_dest, indice, (char)resultado); return; }
        if (buscar_lista_caracter_sin_signo(nombre_dest) >= 0) { set_lista_caracter_sin_signo_valor(nombre_dest, indice, (unsigned char)resultado); return; }
        
        fprintf(stderr, "Error: Lísta '$%s' no declarada.\n", nombre_dest);
        return;
    }
    
    if (scope_actual >= 0) {
        for (int sc = scope_actual; sc >= 0; sc--) {
            ScopeLocal *scp = &scopes_locales[sc];
            for (int j = 0; j < scp->num_variables; j++) {
                if (strcmp(scp->variables[j].nombre, nombre_dest) == 0) {
                    switch (scp->variables[j].tipo) {
                        case 0: scp->variables[j].valor.valor_entero = (int)resultado; break;
                        case 1: scp->variables[j].valor.valor_sin_signo = (unsigned int)resultado; break;
                        case 2: case 3: scp->variables[j].valor.valor_decimal = resultado; break;
                        case 4: scp->variables[j].valor.valor_caracter = (char)resultado; break;
                        case 5: scp->variables[j].valor.valor_caracter_sin_signo = (unsigned char)resultado; break;
                    }
                    return;
                }
            }
        }
    }
    
    for (int k = 0; k < num_variables; k++) if (strcmp(variables[k].nombre, nombre_dest) == 0) { variables[k].valor = (int)resultado; return; }
    for (int k = 0; k < num_variables_decimal; k++) if (strcmp(variables_decimal[k].nombre, nombre_dest) == 0) { variables_decimal[k].valor = resultado; return; }
    for (int k = 0; k < num_variables_sin_signo; k++) if (strcmp(variables_sin_signo[k].nombre, nombre_dest) == 0) { variables_sin_signo[k].valor = (unsigned int)resultado; return; }
    for (int k = 0; k < num_variables_decimal_sin_signo; k++) if (strcmp(variables_decimal_sin_signo[k].nombre, nombre_dest) == 0) { variables_decimal_sin_signo[k].valor = resultado; return; }
    for (int k = 0; k < num_variables_caracter; k++) if (strcmp(variables_caracter[k].nombre, nombre_dest) == 0) { variables_caracter[k].valor = (char)resultado; return; }
    for (int k = 0; k < num_variables_caracter_sin_signo; k++) if (strcmp(variables_caracter_sin_signo[k].nombre, nombre_dest) == 0) { variables_caracter_sin_signo[k].valor = (unsigned char)resultado; return; }
    
    fprintf(stderr, "Error: Variable '$%s' no declarada.\n", nombre_dest);
}

// PROCESAR ESCRIBIR
void procesar_escribir(const char *texto) {
    if (!texto) return;
    int dec_precisions[MAX_VARS];
    int dec_count = 0;
    int dec_actual = 0;

    char buffer[MAX_LINEA];
    strncpy(buffer, texto, MAX_LINEA - 1);
    buffer[MAX_LINEA - 1] = '\0';
    buffer[MAX_LINEA - 1] = '\0';
    
    char *ptr = buffer;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    if (*ptr != '(') {
        fprintf(stderr, "Error: ESCRIBIR requiere paréntesis de apertura.\n");
        return;
    }
    ptr++;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    if (*ptr != '"') {
        fprintf(stderr, "Error: ESCRIBIR requiere texto entre comillas.\n");
        return;
    }
    ptr++;
    
    char texto_final[MAX_LINEA] = "";
    int i = 0;
    while (*ptr && i < MAX_LINEA - 1) {
        if (*ptr == '\\' && *(ptr+1) == '"') { texto_final[i++] = *ptr++; texto_final[i++] = *ptr++; continue; }
        if (*ptr == '\\' && *(ptr+1) == '\\') { texto_final[i++] = *ptr++; texto_final[i++] = *ptr++; continue; }
        if (*ptr == '"') break;
        texto_final[i++] = *ptr++;
    }
    texto_final[i] = '\0';
    
    if (*ptr != '"') {
        fprintf(stderr, "Error: ESCRIBIR requiere comilla de cierre.\n");
        return;
    }
    ptr++;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    if (*ptr == ',') {
        char *dec_ptr = ptr + 1;
        while (*dec_ptr == ' ' || *dec_ptr == '\t') dec_ptr++;
        if (strncmp(dec_ptr, "DECIMALES", 9) == 0) {
            dec_ptr += 9;
            while (*dec_ptr == ' ' || *dec_ptr == '\t') dec_ptr++;
            if (*dec_ptr == '(') {
                dec_ptr++;
                while (*dec_ptr && *dec_ptr != ')' && dec_count < MAX_VARS) {
                    while (*dec_ptr == ' ' || *dec_ptr == '\t' || *dec_ptr == ',') dec_ptr++;
                    if (*dec_ptr == ')') break;
                    char num_str[16] = "";
                    int i = 0;
                    while (*dec_ptr >= '0' && *dec_ptr <= '9' && i < 15) num_str[i++] = *dec_ptr++;
                    num_str[i] = '\0';
                    if (i > 0) dec_precisions[dec_count++] = atoi(num_str);
                }
                if (*dec_ptr == ')') dec_ptr++;
                ptr = dec_ptr;
                while (*ptr == ' ' || *ptr == '\t') ptr++;
            }
        }
    }    
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr != ')') {
        fprintf(stderr, "Error: ESCRIBIR requiere paréntesis de cierre.\n");
        return;
    }
    ptr++;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    int tiene_salto = (strncmp(ptr, "SALTO", 5) == 0) ? 1 : 0;
    
    // PROCESAR OUTPUT
    char resultado[MAX_LINEA] = "";
    ptr = texto_final;
    
    while (*ptr) {
        if (*ptr == '[') {
            const char *cierre = NULL;
            int nivel = 1;
            const char *temp_scan = ptr + 1;
            
            while (*temp_scan) {
                if (*temp_scan == '[') nivel++;
                else if (*temp_scan == ']') {
                    nivel--;
                    if (nivel == 0) {
                        cierre = temp_scan;
                        break;
                    }
                }
                temp_scan++;
            }

            if (cierre) {
                char contenido[MAX_LINEA] = "";
                int j = 0;
                const char *temp = ptr + 1;
                while (temp < cierre && j < MAX_LINEA - 1) contenido[j++] = *temp++;
                contenido[j] = '\0';
                limpiar_string(contenido);
                
                int parece_expresion = 0;
                for (int k = 0; contenido[k]; k++) {
                    if ((contenido[k] >= '0' && contenido[k] <= '9') ||
                        contenido[k] == '$' ||
                        contenido[k] == '+' || contenido[k] == '-' ||
                        contenido[k] == '*' || contenido[k] == '/' ||
                        contenido[k] == '(' || contenido[k] == ')') {
                        parece_expresion = 1;
                        break;
                    }
                }
                if (parece_expresion) {
                    int exito;
                    double valor = evaluar_expresion_completa(contenido, &exito);
                    if (exito) {
                        if (valor >= 3000000.0 && valor < 3100000.0) {
                            int idx_txt = (int)(valor - 3000000.0);
                            if (idx_txt >= 0 && idx_txt < num_texto_vars) {
                                strcat(resultado, texto_vars[idx_txt].valor);
                            }
                            ptr = (char *)cierre + 1;
                            continue;
                        }
                        char buf[64];
                        int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 2;
                        dec_actual++;
                        // Formateo inteligente: si es entero y no se pidió precisión explícita, imprime como entero
                        if (valor == (long long)valor && p <= 2) {
                            sprintf(buf, "[%lld]", (long long)valor);
                        } else {
                            sprintf(buf, "[%.*f]", p, valor);
                        }
                        strcat(resultado, buf);
                        ptr = (char *)cierre + 1;
                        continue;
                    }
                }
                
            } else {
                char temp_char[2] = {'[', '\0'};
                strcat(resultado, temp_char);
            }
            ptr++;
            continue;
        }
        
        if (*ptr == '\\' && *(ptr+1) == '$') { strcat(resultado, "$"); ptr += 2; continue; }
        if (*ptr == '\\' && *(ptr+1) == 'n') { strcat(resultado, "\n"); ptr += 2; continue; }
        if (*ptr == '\\' && *(ptr+1) == 't') { strcat(resultado, "\t"); ptr += 2; continue; }
        if (*ptr == '\\' && *(ptr+1) == '\\') { strcat(resultado, "\\"); ptr += 2; continue; }
        if (*ptr == '\\' && *(ptr+1) == '[') { strcat(resultado, "["); ptr += 2; continue; }
        if (*ptr == '\\' && *(ptr+1) == ']') { strcat(resultado, "]"); ptr += 2; continue; }
        if (*ptr == '\\' && *(ptr+1) == '"') { strcat(resultado, "\""); ptr += 2; continue; }
        
        if (*ptr == '$') {
            ptr++;
            char nombre[MAX_NOMBRE];
            int j = 0;
            while (es_alnum(*ptr) && j < MAX_NOMBRE - 1) nombre[j++] = *ptr++;
            nombre[j] = '\0';
            int encontrado = 0;
            char buf_num[64] = {0};
            int idx;

            char *corchete = strchr(ptr, '[');
            if (corchete) {
                char indice_str[MAX_LINEA];
                int k = 0;
                char *p = corchete + 1;
                int nivel = 1;
                while (*p && nivel > 0 && k < MAX_LINEA - 1) {
                    if (*p == '[') nivel++;
                    else if (*p == ']') nivel--;
                    if (nivel > 0) indice_str[k++] = *p;
                    p++;
                }
                indice_str[k] = '\0';
                limpiar_string(indice_str);
                            
                int idx_lista = -1;    

                int tipo_local = -1, idx_local = -1;
                int encontrado_local = (scope_actual >= 0) ? buscar_lista_local(nombre, &tipo_local, &idx_local) : 0;

                if (encontrado_local) {
                    int exito_indice;
                    double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                    int indice = exito_indice ? (int)val_indice : atoi(indice_str);

                    switch(tipo_local) {
                        case 0: sprintf(buf_num, "%lld", (long long)listas_enteras[idx_local].valores[indice]); break;
                        case 1: { 
                            int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 2; 
                            if (dec_actual < dec_count) dec_actual++; 
                            sprintf(buf_num, "%.*f", p, listas_decimales[idx_local].valores[indice]); 
                            break; 
                        }
                        case 2: sprintf(buf_num, "%llu", (unsigned long long)listas_enteras_sin_signo[idx_local].valores[indice]); break;
                        case 3: { 
                            int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 2; 
                            if (dec_actual < dec_count) dec_actual++; 
                            sprintf(buf_num, "%.*f", p, listas_decimales_sin_signo[idx_local].valores[indice]); 
                            break; 
                        }
                        case 4: sprintf(buf_num, "%c", (unsigned char)listas_caracter[idx_local].valores[indice]); break;
                        case 5: sprintf(buf_num, "%c", listas_caracter_sin_signo[idx_local].valores[indice]); break;
                        case 6: sprintf(buf_num, "%s", listas_texto[idx_local].valores[indice]);break;
                        default: sprintf(buf_num, "?"); break;
                    }
                    encontrado = 1;
                }

                else if (buscar_lista_entera(nombre) >= 0) {
                    int exito_indice; double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                    int indice = exito_indice ? (int)val_indice : atoi(indice_str);
                    sprintf(buf_num, "%lld", (long long)get_lista_entera_valor(nombre, indice)); encontrado = 1;
                }

               else if ((idx_lista = buscar_lista_decimal(nombre)) >= 0) {
                    int exito_indice; double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                    int indice = exito_indice ? (int)val_indice : atoi(indice_str);
                    int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 2;
                    if (dec_actual < dec_count) dec_actual++;
                    sprintf(buf_num, "%.*f", p, get_lista_decimal_valor(nombre, indice)); 
                    encontrado = 1;
                }

                else if ((idx_lista = buscar_lista_entera_sin_signo(nombre)) >= 0) {
                    int exito_indice; double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                    int indice = exito_indice ? (int)val_indice : atoi(indice_str);
                    sprintf(buf_num, "%llu", (unsigned long long)get_lista_entera_sin_signo_valor(nombre, indice)); encontrado = 1;
                }

                else if ((idx_lista = buscar_lista_decimal_sin_signo(nombre)) >= 0) {
                    int exito_indice; double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                    int indice = exito_indice ? (int)val_indice : atoi(indice_str);
                    int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6; dec_actual++;
                    sprintf(buf_num, "%.*f", p, get_lista_decimal_sin_signo_valor(nombre, indice)); encontrado = 1;
                }

                else if ((idx_lista = buscar_lista_caracter(nombre)) >= 0) {
                    int exito_indice; double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                    int indice = exito_indice ? (int)val_indice : atoi(indice_str);
                    sprintf(buf_num, "%c", (unsigned char)get_lista_caracter_valor(nombre, indice)); encontrado = 1;
                }

                else if ((idx_lista = buscar_lista_caracter_sin_signo(nombre)) >= 0) {
                    int exito_indice; double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                    int indice = exito_indice ? (int)val_indice : atoi(indice_str);
                    sprintf(buf_num, "%c", get_lista_caracter_sin_signo_valor(nombre, indice)); encontrado = 1;
                }
                
                else {
                    // Limpiar nombre para búsqueda en scope local y global
                    const char *clean_name = (nombre[0] == '$') ? nombre + 1 : nombre;
    
                    // 1. Buscar en scope local PRIMERO (shadowing)
                    int tipo_loc, idx_loc;
                    if (buscar_lista_local(clean_name, &tipo_loc, &idx_loc) && tipo_loc == 6) {
                        int exito_indice; double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                        int indice = exito_indice ? (int)val_indice : atoi(indice_str);
                        if (indice < 0 || indice >= listas_texto[idx_loc].longitud) {
                            sprintf(buf_num, "[FUERA_DE_RANGO]");
                        } else {
                            sprintf(buf_num, "%s", listas_texto[idx_loc].valores[indice]);
                        }
                        encontrado = 1;
                    }
                    // 2. Fallback a pool global
                    else if ((idx_lista = buscar_lista_texto(clean_name)) >= 0) {  // ← ✅ clean_name, no nombre
                        int exito_indice; double val_indice = evaluar_expresion_completa(indice_str, &exito_indice);
                        int indice = exito_indice ? (int)val_indice : atoi(indice_str);
                        if (indice < 0 || indice >= listas_texto[idx_lista].longitud) {
                            sprintf(buf_num, "[FUERA_DE_RANGO]");
                        } else {
                            sprintf(buf_num, "%s", listas_texto[idx_lista].valores[indice]);
                        }
                        encontrado = 1;
                    }
                }

                if (!encontrado) {
                    char *cierre1 = strchr(ptr, ']');
                    char *corchete2 = cierre1 ? strchr(cierre1 + 1, '[') : NULL;
        
                    if (corchete2) {
                        char fila_str[MAX_LINEA] = "", col_str[MAX_LINEA] = "";
                        int k = 0, nivel = 1;
                        const char *p1 = ptr + 1;
                        while (*p1 && nivel > 0 && k < MAX_LINEA - 1) {
                            if (*p1 == '[') nivel++; else if (*p1 == ']') nivel--;
                            if (nivel > 0) fila_str[k++] = *p1;
                            p1++;
                        }
                        fila_str[k] = '\0'; limpiar_string(fila_str);
        
                        k = 0; nivel = 1;
                        const char *p2 = corchete2 + 1;
                        while (*p2 && nivel > 0 && k < MAX_LINEA - 1) {
                            if (*p2 == '[') nivel++; else if (*p2 == ']') nivel--;
                            if (nivel > 0) col_str[k++] = *p2;
                            p2++;
                        }
                        col_str[k] = '\0'; limpiar_string(col_str);
        
                        nivel = 1; p2 = corchete2 + 1;
                        while (*p2 && nivel > 0) { if (*p2=='[') nivel++; else if (*p2==']') nivel--; p2++; }
        
                        int tipo_var; double val_var;
                        int found_as_param = buscar_variable_local(nombre, &tipo_var, &val_var);
        
                        if (found_as_param && val_var >= 2000000.0 && val_var < 2060000.0) {
                            int ref = (int)(val_var - 2000000.0);
                            int tipo_mat = ref / 10000;
                            int idx_pool = ref % 10000;
            
                            int ef, ec; 
                            double vf = evaluar_expresion_completa(fila_str, &ef); 
                            double vc = evaluar_expresion_completa(col_str, &ec);
                            int f = ef ? (int)vf : atoi(fila_str); 
                            int c = ec ? (int)vc : atoi(col_str);
            
                            switch(tipo_mat) {
                                case 0: sprintf(buf_num, "%lld", (long long)matrices_enteras[idx_pool].valores[f][c]); break;
                                case 1: case 3: { 
                                    int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6; 
                                    dec_actual++; 
                                    sprintf(buf_num, "%.*f", p, matrices_decimales[idx_pool].valores[f][c]); 
                                    break; 
                                }
                                case 2: sprintf(buf_num, "%llu", (unsigned long long)matrices_enteras_sin_signo[idx_pool].valores[f][c]); break;
                                case 4: sprintf(buf_num, "%c", (unsigned char)matrices_caracter[idx_pool].valores[f][c]); break;
                                case 5: sprintf(buf_num, "%c", matrices_caracter_sin_signo[idx_pool].valores[f][c]); break;
                            }
                            encontrado = 1; 
                            ptr = (char *)p2; 
                            strcat(resultado, buf_num); 
                            continue;
                        }
        
                        int tipo_loc = -1, idx_loc = -1;
                        // Limpiar nombre: registrar_matriz_local guarda SIN el '$'
                        const char *clean_name_read = (nombre[0] == '$') ? nombre + 1 : nombre;
                        int found_loc = buscar_matriz_local(clean_name_read, &tipo_loc, &idx_loc);

                        if (found_loc) {
                            int ef, ec; 
                            double vf = evaluar_expresion_completa(fila_str, &ef); 
                            double vc = evaluar_expresion_completa(col_str, &ec);
                            int f = ef ? (int)vf : atoi(fila_str); 
                            int c = ec ? (int)vc : atoi(col_str);

                            switch(tipo_loc) {
                                case 0: sprintf(buf_num, "%lld", (long long)matrices_enteras[idx_loc].valores[f][c]); break;
                                case 1: { 
                                    int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6; 
                                    dec_actual++; 
                                    sprintf(buf_num, "%.*f", p, matrices_decimales[idx_loc].valores[f][c]); 
                                    break; 
                                }
                                case 3: { 
                                    int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6; 
                                    dec_actual++; 
                                    sprintf(buf_num, "%.*f", p, matrices_decimales_sin_signo[idx_loc].valores[f][c]); 
                                    break; 
                                }
                                case 2: sprintf(buf_num, "%llu", (unsigned long long)matrices_enteras_sin_signo[idx_loc].valores[f][c]); break;
                                case 4: sprintf(buf_num, "%c", (unsigned char)matrices_caracter[idx_loc].valores[f][c]); break;
                                case 5: sprintf(buf_num, "%c", matrices_caracter_sin_signo[idx_loc].valores[f][c]); break;
                                case 6: sprintf(buf_num, "%s", get_matriz_texto_valor(matrices_texto[idx_loc].nombre, f, c)); break;
                            }
                            encontrado = 1; 
                            ptr = (char *)p2; 
                            strcat(resultado, buf_num); 
                            continue;
                        }
        
                        if (buscar_matriz_entera(nombre) >= 0) {
                            int ef, ec; double vf = evaluar_expresion_completa(fila_str, &ef); double vc = evaluar_expresion_completa(col_str, &ec);
                            int f = ef ? (int)vf : atoi(fila_str); int c = ec ? (int)vc : atoi(col_str);
                            sprintf(buf_num, "%lld", (long long)get_matriz_entera_valor(nombre, f, c)); 
                            encontrado = 1; ptr = (char *)p2; strcat(resultado, buf_num); continue;
                        }

                        else if (buscar_matriz_decimal(nombre) >= 0) {
                            int ef, ec; double vf = evaluar_expresion_completa(fila_str, &ef); double vc = evaluar_expresion_completa(col_str, &ec);
                            int f = ef ? (int)vf : atoi(fila_str); int c = ec ? (int)vc : atoi(col_str);
                            int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6; dec_actual++;
                            sprintf(buf_num, "%.*f", p, get_matriz_decimal_valor(nombre, f, c)); 
                            encontrado = 1; ptr = (char *)p2; strcat(resultado, buf_num); continue;
                        }

                        else if (buscar_matriz_entera_sin_signo(nombre) >= 0) {
                            int ef, ec; double vf = evaluar_expresion_completa(fila_str, &ef); double vc = evaluar_expresion_completa(col_str, &ec);
                            int f = ef ? (int)vf : atoi(fila_str); int c = ec ? (int)vc : atoi(col_str);
                            sprintf(buf_num, "%llu", (unsigned long long)get_matriz_entera_sin_signo_valor(nombre, f, c)); 
                            encontrado = 1; ptr = (char *)p2; strcat(resultado, buf_num); continue;
                        }

                        else if (buscar_matriz_decimal_sin_signo(nombre) >= 0) {
                            int ef, ec; double vf = evaluar_expresion_completa(fila_str, &ef); double vc = evaluar_expresion_completa(col_str, &ec);
                            int f = ef ? (int)vf : atoi(fila_str); int c = ec ? (int)vc : atoi(col_str);
                            int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 6; dec_actual++;
                            sprintf(buf_num, "%.*f", p, get_matriz_decimal_sin_signo_valor(nombre, f, c)); 
                            encontrado = 1; ptr = (char *)p2; strcat(resultado, buf_num); continue;
                        }

                        else if (buscar_matriz_caracter(nombre) >= 0) {
                            int ef, ec; double vf = evaluar_expresion_completa(fila_str, &ef); double vc = evaluar_expresion_completa(col_str, &ec);
                            int f = ef ? (int)vf : atoi(fila_str); int c = ec ? (int)vc : atoi(col_str);
                            sprintf(buf_num, "%c", (unsigned char)get_matriz_caracter_valor(nombre, f, c)); 
                            encontrado = 1; ptr = (char *)p2; strcat(resultado, buf_num); continue;
                        }

                        else if (buscar_matriz_caracter_sin_signo(nombre) >= 0) {
                            int ef, ec; double vf = evaluar_expresion_completa(fila_str, &ef); double vc = evaluar_expresion_completa(col_str, &ec);
                            int f = ef ? (int)vf : atoi(fila_str); int c = ec ? (int)vc : atoi(col_str);
                            sprintf(buf_num, "%c", get_matriz_caracter_sin_signo_valor(nombre, f, c)); 
                            encontrado = 1; ptr = (char *)p2; strcat(resultado, buf_num); continue;
                        }

                        else {
                            // Limpiar nombre para búsqueda en scope local y global
                            const char *clean_name = (nombre[0] == '$') ? nombre + 1 : nombre;
    
                            // 1. Buscar en scope local PRIMERO (shadowing)
                            int tipo_loc, idx_loc;
                            if (buscar_matriz_local(clean_name, &tipo_loc, &idx_loc) && tipo_loc == 6) {
                                int ef, ec; 
                                double vf = evaluar_expresion_completa(fila_str, &ef); 
                                double vc = evaluar_expresion_completa(col_str, &ec);
                                int f = ef ? (int)vf : atoi(fila_str); 
                                int c = ec ? (int)vc : atoi(col_str);
                                sprintf(buf_num, "%s", get_matriz_texto_valor(matrices_texto[idx_loc].nombre, f, c));
                                encontrado = 1; 
                                ptr = (char *)p2; 
                                strcat(resultado, buf_num); 
                                continue;
                            }
                            // 2. Fallback a pool global
                            else if (buscar_matriz_texto_en_scope(clean_name, scope_actual) >= 0) {
                                int ef, ec; 
                                double vf = evaluar_expresion_completa(fila_str, &ef); 
                                double vc = evaluar_expresion_completa(col_str, &ec);
                                int f = ef ? (int)vf : atoi(fila_str); 
                                int c = ec ? (int)vc : atoi(col_str);
                                sprintf(buf_num, "%s", get_matriz_texto_valor(clean_name, f, c));
                                encontrado = 1; 
                                ptr = (char *)p2; 
                                strcat(resultado, buf_num); 
                                continue;
                            }
                        }
                    }
                }           
                if (encontrado) {
                    strcat(resultado, buf_num);
                    ptr = p;
                    if (*ptr == ']') ptr++;
                    continue;
                }
            }

            int idx_txt = -1;
            if (scope_actual >= 0) {
                for (int s = scope_actual; s >= 0; s--) {
                    for (int v = 0; v < scopes_locales[s].num_textos; v++) {
                        if (strcmp(scopes_locales[s].nombres_textos[v], nombre) == 0) {
                            idx_txt = scopes_locales[s].indices_textos[v];
                            break;
                        }
                    }
                    if (idx_txt >= 0) break;
                }
            }
            if (idx_txt < 0) idx_txt = buscar_texto_var(nombre);

            if (idx_txt >= 0) {
                strcat(resultado, texto_vars[idx_txt].valor);
                encontrado = 1;
            }
            else if ((idx_txt = buscar_texto_constante(nombre)) >= 0) {
                strcat(resultado, texto_constantes[idx_txt].valor);
                encontrado = 1;
            }

            if (!encontrado) {
                for (int s = MAX_SCOPES - 1; s >= 0 && !encontrado; s--) {
                    if (scopes_locales[s].num_variables == 0) continue;
                    for (int v = 0; v < scopes_locales[s].num_variables && !encontrado; v++) {
                        if (strcmp(scopes_locales[s].variables[v].nombre, nombre) == 0) {
                            switch(scopes_locales[s].variables[v].tipo) {
                                case 0: {
                                        double val = (double)scopes_locales[s].variables[v].valor.valor_entero;
                                        if (val >= 3000000.0 && val < 3100000.0) {
                                            int idx_txt = (int)(val - 3000000.0);
                                            if (idx_txt >= 0 && idx_txt < num_texto_vars) {
                                                strcat(resultado, texto_vars[idx_txt].valor);
                                                encontrado = 1;
                                            }
                                        } else {
                                            sprintf(buf_num, "%lld", (long long)val);
                                        }
                                        break;
                                    }
                                case 1: {
                                        double val = (double)scopes_locales[s].variables[v].valor.valor_sin_signo;
                                        if (val >= 3000000.0 && val < 3100000.0) {
                                            int idx_txt = (int)(val - 3000000.0);
                                            if (idx_txt >= 0 && idx_txt < num_texto_vars) {
                                                strcat(resultado, texto_vars[idx_txt].valor);
                                                encontrado = 1;
                                            }
                                        } else {
                                            sprintf(buf_num, "%llu", (unsigned long long)val);
                                        }
                                        break;
                                    } 
                                case 2: case 3: {
                                        double val = scopes_locales[s].variables[v].valor.valor_decimal;
                                        if (val >= 3000000.0 && val < 3100000.0) {
                                            int idx_txt = (int)(val - 3000000.0);
                                            if (idx_txt >= 0 && idx_txt < num_texto_vars) {
                                                strcat(resultado, texto_vars[idx_txt].valor);
                                                encontrado = 1;
                                            }
                                        } else {
                                            int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 2;  // ← Default 2 decimales
                                            dec_actual++;
                                            sprintf(buf_num, "%.*f", p, val);
                                        }
                                    } break;
                                case 4: sprintf(buf_num, "%c", (char)scopes_locales[s].variables[v].valor.valor_caracter); break;
                                case 5: sprintf(buf_num, "%c", (unsigned char)scopes_locales[s].variables[v].valor.valor_caracter_sin_signo); break;
                                default: sprintf(buf_num, "%g", scopes_locales[s].variables[v].valor.valor_decimal); break;
                            }
                            encontrado = 1;
                        }
                    }
                }
            }

            if (!encontrado) {
                if ((idx = buscar_variable_decimal(nombre)) >= 0) { 
                    double val = variables_decimal[idx].valor;
                    int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 2;
                    dec_actual++;
                    sprintf(buf_num, "%.*f", p, val);
                    encontrado = 1; 
                }
                else if ((idx = buscar_variable(nombre)) >= 0) { sprintf(buf_num, "%lld", (long long)variables[idx].valor); encontrado = 1; }
                else if ((idx = buscar_variable_sin_signo(nombre)) >= 0) { sprintf(buf_num, "%llu", (unsigned long long)variables_sin_signo[idx].valor); encontrado = 1; }
                else if ((idx = buscar_variable_decimal_sin_signo(nombre)) >= 0) { 
                    double val = variables_decimal_sin_signo[idx].valor;
                    int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 2;
                    dec_actual++;
                    sprintf(buf_num, "%.*f", p, val);
                    encontrado = 1; 
                }
                else if ((idx = buscar_variable_caracter(nombre)) >= 0) { sprintf(buf_num, "%c", (char)variables_caracter[idx].valor); encontrado = 1; }
                else if ((idx = buscar_variable_caracter_sin_signo(nombre)) >= 0) { sprintf(buf_num, "%c", (unsigned char)variables_caracter_sin_signo[idx].valor); encontrado = 1; }
            }
            
            // BUSCAR EN POOLS DE CONSTANTES (si no se encontró en variables)
            if (!encontrado) {
                if ((idx = buscar_constante_decimal(nombre)) >= 0) { 
                    double val = constantes_decimal[idx].valor;
                    int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 2;
                    dec_actual++;
                    sprintf(buf_num, "%.*f", p, val);
                    encontrado = 1; 
                }
                else if ((idx = buscar_constante(nombre)) >= 0) { sprintf(buf_num, "%lld", (long long)constantes[idx].valor); encontrado = 1; }
                else if ((idx = buscar_constante_sin_signo(nombre)) >= 0) { sprintf(buf_num, "%llu", (unsigned long long)constantes_sin_signo[idx].valor); encontrado = 1; }
                else if ((idx = buscar_constante_decimal_sin_signo(nombre)) >= 0) { 
                    double val = constantes_decimal_sin_signo[idx].valor;
                    int p = (dec_actual < dec_count) ? dec_precisions[dec_actual] : 2;
                    dec_actual++;
                    sprintf(buf_num, "%.*f", p, val);
                    encontrado = 1; 
                }
                else if ((idx = buscar_constante_caracter(nombre)) >= 0) { sprintf(buf_num, "%c", (char)constantes_caracter[idx].valor); encontrado = 1; }
                else if ((idx = buscar_constante_caracter_sin_signo(nombre)) >= 0) { sprintf(buf_num, "%s", constantes_caracter_sin_signo[idx].valor); encontrado = 1; }
            }

            if (!encontrado) {
                int es_ext = 0, idx_ext = -1, scope_ext = -1;
                if (buscar_texto_extenso(nombre, &es_ext, &idx_ext, &scope_ext) >= 0) {
                    char *val = es_ext ? scopes_locales[scope_ext].textos_ext[idx_ext].valor 
                                       : textos_ext_globales[idx_ext].valor;
                    if (val) {
                        strncat(resultado, val, sizeof(resultado) - strlen(resultado) - 1);
                    }
                    encontrado = 1;
                }
            }

            if (encontrado) {
                if (buf_num[0] != '\0') {
                    strcat(resultado, buf_num);
                }
                continue; 
            }
            
            fprintf(stderr, "Error: Variable '$%s' no declarada.\n", nombre);
            return;
        }

        else {
            char temp[2] = {*ptr, '\0'};
            strcat(resultado, temp);
            ptr++;
        } 
    }
    
    printf("%s", resultado);
    if (tiene_salto) printf("\n");
    fflush(stdout);
}

// PROCESAR LEER
void procesar_leer(const char *argumento) {
    if (!argumento) return;
    
    char nombre_var[MAX_NOMBRE] = "";
    const char *ptr = argumento;
    
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr == '(') {
        ptr++;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
    }
    
    int i = 0;
    if (*ptr == '$') ptr++;
    while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) nombre_var[i++] = *ptr++;
    nombre_var[i] = '\0';
    limpiar_string(nombre_var);
    
    if (strlen(nombre_var) == 0) {
        fprintf(stderr, "Error: LEER requiere una variable válida.\n");
        return;
    }
    
    char entrada[MAX_LINEA];
    if (!fgets(entrada, MAX_LINEA, stdin)) return;
    limpiar_string(entrada);
    
    char *fin;
    double valor_num = strtod(entrada, &fin);
    int es_numero = (fin != entrada && *fin == '\0');
    
    int idx_pool = -1;
    if (scope_actual >= 0) {
        for (int s = scope_actual; s >= 0; s--) {
            for (int v = 0; v < scopes_locales[s].num_textos; v++) {
                if (strcmp(scopes_locales[s].nombres_textos[v], nombre_var) == 0) {
                    idx_pool = scopes_locales[s].indices_textos[v];
                    break;
                }
            }
            if (idx_pool != -1) break;
        }
    }
    
    if (idx_pool >= 0) {
        strncpy(texto_vars[idx_pool].valor, entrada, MAX_TEXTO_LEN - 1);
        texto_vars[idx_pool].valor[MAX_TEXTO_LEN - 1] = '\0';
        return;
    }
    
    int idx = buscar_texto_var(nombre_var);
    if (idx >= 0) {
        strncpy(texto_vars[idx].valor, entrada, MAX_TEXTO_LEN - 1);
        return;
    }
    
    if (es_numero) {
        int idx = buscar_variable(nombre_var);
        if (idx >= 0) { variables[idx].valor = (int)valor_num; return; }
        idx = buscar_variable_decimal(nombre_var);
        if (idx >= 0) { variables_decimal[idx].valor = valor_num; return; }
        idx = buscar_variable_sin_signo(nombre_var);
        if (idx >= 0) { variables_sin_signo[idx].valor = (unsigned int)valor_num; return; }
        idx = buscar_variable_decimal_sin_signo(nombre_var);
        if (idx >= 0) { variables_decimal_sin_signo[idx].valor = valor_num; return; }
    }

    if (scope_actual >= 0 && en_funcion) {
        agregar_texto_local(nombre_var, entrada);
        return;
    }
    
    agregar_texto_var(nombre_var, entrada);
     
    if (idx >= 0) {
        strncpy(texto_vars[idx].valor, entrada, MAX_TEXTO_LEN - 1);
        texto_vars[idx].valor[MAX_TEXTO_LEN - 1] = '\0';
        return;
    }

    if (strlen(entrada) > 0) {
        idx = buscar_variable_caracter(nombre_var);
        if (idx >= 0) { variables_caracter[idx].valor = entrada[0]; return; }
    }
    
    if (strlen(entrada) > 0) {
        idx = buscar_variable_caracter_sin_signo(nombre_var);
        if (idx >= 0) { variables_caracter_sin_signo[idx].valor = (unsigned char)entrada[0]; return; }
    }
    
    if (es_numero) {
        idx = buscar_variable(nombre_var);
        if (idx >= 0) { 
            variables[idx].valor = (int)valor_num; 
            return; 
        }
    }
    
    if (es_numero) {
        idx = buscar_variable_sin_signo(nombre_var);
        if (idx >= 0) { variables_sin_signo[idx].valor = (unsigned int)valor_num; return; }
    }
    
    idx = buscar_variable_decimal(nombre_var);
    if (idx >= 0) { variables_decimal[idx].valor = valor_num; return; }
    
    idx = buscar_variable_decimal_sin_signo(nombre_var);
    if (idx >= 0) { variables_decimal_sin_signo[idx].valor = valor_num; return; }
    
    if (en_funcion) {
        int local_idx = -1;
        if (scope_actual >= 0) {
            for (int s = scope_actual; s >= 0; s--) {
                for (int v = 0; v < scopes_locales[s].num_textos; v++) {
                    if (strcmp(scopes_locales[s].nombres_textos[v], nombre_var) == 0) {
                        local_idx = scopes_locales[s].indices_textos[v];
                        break;
                    }
                }
                if (local_idx >= 0) break;
            }
        }
        if (local_idx >= 0) {
            strncpy(texto_vars[local_idx].valor, entrada, MAX_TEXTO_LEN - 1);
            texto_vars[local_idx].valor[MAX_TEXTO_LEN - 1] = '\0';
            return;
        }

        if (en_funcion) {
            int local_idx = -1;
            if (scope_actual >= 0 && scope_actual < MAX_SCOPES) {
                for (int s = scope_actual; s >= 0; s--) {
                    for (int v = 0; v < scopes_locales[s].num_textos; v++) {
                        const char *n1 = scopes_locales[s].nombres_textos[v];
                        const char *n2 = nombre_var;
                        if (n1[0] == '$') n1++;
                        if (n2[0] == '$') n2++;
                        if (strcmp(n1, n2) == 0) {
                            local_idx = scopes_locales[s].indices_textos[v];
                            break;
                        }
                    }
                    if (local_idx != -1) break;
                }
            }
            if (local_idx >= 0) {
                strncpy(texto_vars[local_idx].valor, entrada, MAX_TEXTO_LEN - 1);
                texto_vars[local_idx].valor[MAX_TEXTO_LEN - 1] = '\0';
                return;
            }

            if (es_numero) agregar_variable(nombre_var, (int)valor_num);
            else agregar_texto_var(nombre_var, entrada);
            return;
        }       
    }
    fprintf(stderr, "Error: Variable '$%s' no declarada.\n", nombre_var);
}

// PROCESAR LEERCARACTER
void procesar_leercaracter(const char *argumento) {
    if (!argumento) return;
    
    char nombre_var[MAX_NOMBRE] = "";
    const char *ptr = argumento;
    
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr == '(') {
        ptr++;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
    }
    
    int i = 0;
    if (*ptr == '$') ptr++;
    while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) nombre_var[i++] = *ptr++;
    nombre_var[i] = '\0';
    limpiar_string(nombre_var);
    
    if (strlen(nombre_var) == 0) {
        fprintf(stderr, "Error: LEERCARACTER requiere una variable válida.\n");
        return;
    }
    
    int c = getchar();
    
    int idx = buscar_variable_caracter(nombre_var);
    if (idx >= 0) { variables_caracter[idx].valor = (char)c; while (getchar() != '\n' && !feof(stdin)); return; }
    
    idx = buscar_variable_caracter_sin_signo(nombre_var);
    if (idx >= 0) { variables_caracter_sin_signo[idx].valor = (unsigned char)c; while (getchar() != '\n' && !feof(stdin)); return; }
    
    if (en_funcion) { agregar_variable_caracter(nombre_var, (char)c); while (getchar() != '\n' && !feof(stdin)); return; }
    
    fprintf(stderr, "Error: Variable CARACTER '$%s' no declarada.\n", nombre_var);
}

// PROCESAR LEERHASTA
void procesar_leerhasta(const char *argumento) {
    if (!argumento) return;
    
    char nombre_var[MAX_NOMBRE] = "";
    const char *ptr = argumento;
    int i = 0;
    
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    if (*ptr == '$') ptr++;
    while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) nombre_var[i++] = *ptr++;
    nombre_var[i] = '\0';
    limpiar_string(nombre_var);
    
    if (strlen(nombre_var) == 0) {
        fprintf(stderr, "Error: LEERHASTA requiere variable TEXTO destino.\n");
        return;
    }
    
    const char *coma = strchr(argumento, ',');
    if (!coma) {
        fprintf(stderr, "Error: LEERHASTA requiere terminador.\n");
        return;
    }
    
    const char *terminador_ptr = coma + 1;
    while (*terminador_ptr == ' ' || *terminador_ptr == '\t') terminador_ptr++;
    
    char terminador[MAX_TEXTO_LEN] = "";
    i = 0;
    if (*terminador_ptr == '"') {
        terminador_ptr++;
        while (*terminador_ptr && *terminador_ptr != '"' && i < MAX_TEXTO_LEN - 1) {
            terminador[i++] = *terminador_ptr++;
        }
    } else {
        while (*terminador_ptr && *terminador_ptr != ')' && i < MAX_TEXTO_LEN - 1) {
            terminador[i++] = *terminador_ptr++;
        }
    }
    terminador[i] = '\0';
    limpiar_string(terminador);
    
    if (strlen(terminador) == 0) {
        fprintf(stderr, "Error: LEERHASTA requiere terminador válido.\n");
        return;
    }
    
    int idx = buscar_texto_var(nombre_var);
    if (idx < 0) {
        fprintf(stderr, "Error: Texto '$%s' no declarado.\n", nombre_var);
        return;
    }
    
    texto_vars[idx].valor[0] = '\0';
    
    char linea[MAX_LINEA];
    int primera_linea = 1;
    
    while (fgets(linea, MAX_LINEA, stdin)) {
        size_t len = strlen(linea);
        if (len > 0 && linea[len-1] == '\n') {
            linea[len-1] = '\0';
            len--;
        }
        
        if (strcmp(linea, terminador) == 0) break;
        
        if (!primera_linea) {
            if (strlen(texto_vars[idx].valor) + 1 < MAX_TEXTO_LEN) strcat(texto_vars[idx].valor, "\n");
        }
        
        if (strlen(texto_vars[idx].valor) + len < MAX_TEXTO_LEN) strcat(texto_vars[idx].valor, linea);
        primera_linea = 0;
    }
}

// FUNCIONES DE COLORES
int obtener_codigo_color_texto(const char *color) {
    if (strcmp(color, "negro") == 0) return 30;
    if (strcmp(color, "rojo") == 0) return 31;
    if (strcmp(color, "verde") == 0) return 32;
    if (strcmp(color, "amarillo") == 0) return 33;
    if (strcmp(color, "azul") == 0) return 34;
    if (strcmp(color, "magenta") == 0) return 35;
    if (strcmp(color, "cyan") == 0) return 36;
    if (strcmp(color, "blanco") == 0) return 37;
    if (strcmp(color, "gris") == 0) return 90;
    if (strcmp(color, "rojoclaro") == 0) return 91;
    if (strcmp(color, "verdeclaro") == 0) return 92;
    if (strcmp(color, "amarilloclaro") == 0) return 93;
    if (strcmp(color, "azulclaro") == 0) return 94;
    if (strcmp(color, "magentaclaro") == 0) return 95;
    if (strcmp(color, "cyanclaro") == 0) return 96;
    if (strcmp(color, "blancoclaro") == 0) return 97;
    return -1;
}

int obtener_codigo_color_fondo(const char *color) {
    if (strcmp(color, "negro") == 0) return 40;
    if (strcmp(color, "rojo") == 0) return 41;
    if (strcmp(color, "verde") == 0) return 42;
    if (strcmp(color, "amarillo") == 0) return 43;
    if (strcmp(color, "azul") == 0) return 44;
    if (strcmp(color, "magenta") == 0) return 45;
    if (strcmp(color, "cyan") == 0) return 46;
    if (strcmp(color, "blanco") == 0) return 47;
    if (strcmp(color, "gris") == 0) return 100;
    if (strcmp(color, "rojoclaro") == 0) return 101;
    if (strcmp(color, "verdeclaro") == 0) return 102;
    if (strcmp(color, "amarilloclaro") == 0) return 103;
    if (strcmp(color, "azulclaro") == 0) return 104;
    if (strcmp(color, "magentaclaro") == 0) return 105;
    if (strcmp(color, "cyanclaro") == 0) return 106;
    if (strcmp(color, "blancoclaro") == 0) return 107;
    return -1;
}

void procesar_colortexto(const char *argumento) {
    if (!argumento) return;
    const char *ptr = argumento;
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    char color[MAX_NOMBRE] = "";
    int i = 0;
    while (*ptr && *ptr != ')' && *ptr != '"' && i < MAX_NOMBRE - 1) color[i++] = *ptr++;
    color[i] = '\0'; limpiar_string(color);
    int codigo = obtener_codigo_color_texto(color);
    if (codigo >= 0) { printf("\033[%dm", codigo); fflush(stdout); }
    else fprintf(stderr, "Error: Color de texto '%s' no válido.\n", color);
}

void procesar_colorfondo(const char *argumento) {
    if (!argumento) return;
    const char *ptr = argumento;
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    char color[MAX_NOMBRE] = "";
    int i = 0;
    while (*ptr && *ptr != ')' && *ptr != '"' && i < MAX_NOMBRE - 1) color[i++] = *ptr++;
    color[i] = '\0'; limpiar_string(color);
    int codigo = obtener_codigo_color_fondo(color);
    if (codigo >= 0) { printf("\033[%dm", codigo); fflush(stdout); }
    else fprintf(stderr, "Error: Color de fondo '%s' no válido.\n", color);
}

void procesar_resetcolor(const char *argumento) {
    (void)argumento; printf("\033[0m"); fflush(stdout); 
}

void procesar_textonegrita(const char *argumento) {
    (void)argumento; printf("\033[1m"); fflush(stdout); 
}

void procesar_textocursiva(const char *argumento) {
    (void)argumento; printf("\033[3m"); fflush(stdout); 
}

void procesar_textosubrayado(const char *argumento) {
    (void)argumento; printf("\033[4m"); fflush(stdout); 
}

void procesar_textoreset(const char *argumento) {
    (void)argumento; printf("\033[0m"); fflush(stdout); 
}

static int extraer_nombre_var_texto(const char *linea, char *nombre) {
    const char *p = strchr(linea, '$');
    if (!p) return -1;
    p++;
    int i = 0;
    while (es_alnum(*p) && i < MAX_NOMBRE - 1) nombre[i++] = *p++;
    nombre[i] = '\0';
    return (i > 0) ? 0 : -1;
}

int cmd_hora_actual(const char *linea, CtxBloque *ctx, int linea_actual) {
    char nombre[MAX_NOMBRE];
    if (extraer_nombre_var_texto(linea, nombre) < 0) {
        fprintf(stderr, "Error línea %d: HORAACTUAL requiere variable de texto ($var).\n", linea_actual);
        ctx->linea_num++; return 0;
    }

    int idx = buscar_texto_var(nombre);
    if (idx < 0) {
        fprintf(stderr, "Error línea %d: Variable TEXTO '$%s' no declarada.\n", linea_actual, nombre);
        ctx->linea_num++; return 0;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char buffer[MAX_LINEA];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", t);

    strncpy(texto_vars[idx].valor, buffer, MAX_TEXTO_LEN - 1);
    texto_vars[idx].valor[MAX_TEXTO_LEN - 1] = '\0';
    ctx->linea_num++; return 0;
}

int cmd_fecha_actual(const char *linea, CtxBloque *ctx, int linea_actual) {
    char nombre[MAX_NOMBRE];
    if (extraer_nombre_var_texto(linea, nombre) < 0) {
        fprintf(stderr, "Error línea %d: FECHAACTUAL requiere variable de texto ($var).\n", linea_actual);
        ctx->linea_num++; return 0;
    }

    int idx = buscar_texto_var(nombre);
    if (idx < 0) {
        fprintf(stderr, "Error línea %d: Variable TEXTO '$%s' no declarada.\n", linea_actual, nombre);
        ctx->linea_num++; return 0;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char buffer[MAX_LINEA];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y", t);

    strncpy(texto_vars[idx].valor, buffer, MAX_TEXTO_LEN - 1);
    texto_vars[idx].valor[MAX_TEXTO_LEN - 1] = '\0';
    ctx->linea_num++; return 0;
}

// OTRAS FUNCIONES
void procesar_limpiarpantalla(void) {
#ifdef _WIN32
    (void)system("cls");
#else
    (void)system("clear");
#endif
}

void procesar_esperar(const char *argumento) {
    const char *ptr = argumento;
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    
    const char *cierre = strrchr(ptr, ')');
    if (!cierre) {
        fprintf(stderr, "Error: ESPERAR requiere paréntesis de cierre.\n");
        return;
    }
    
    char contenido[MAX_LINEA];
    int len = (int)(cierre - ptr);
    if (len <= 0 || len >= MAX_LINEA - 1) {
        fprintf(stderr, "Error: Argumento de ESPERAR inválido.\n");
        return;
    }
    strncpy(contenido, ptr, len);
    contenido[len] = '\0';
    
    char *coma = strchr(contenido, ',');
    if (!coma) {
        fprintf(stderr, "Error: Formato inválido. Uso: ESPERAR(valor, UNIDAD)\n");
        return;
    }
    *coma = '\0';
    char *arg_valor = contenido;
    char *arg_unidad = coma + 1;
    
    // Limpieza robusta de unidad
    char unidad_limpia[64] = "";
    int k = 0;
    for (int i = 0; arg_unidad[i] && k < 63; i++) {
        char c = arg_unidad[i];
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n' && c != ')') {
            unidad_limpia[k++] = (c >= 'a' && c <= 'z') ? (c - 32) : c;
        }
    }
    unidad_limpia[k] = '\0';
    
    int exito = 0;
    double valor = evaluar_expresion_completa(arg_valor, &exito);
    if (!exito || valor < 0) {
        fprintf(stderr, "Error: ESPERAR requiere un valor numérico positivo.\n");
        return;
    }
    
    long long tiempo_us = 0;
    if (strcmp(unidad_limpia, "MICROSEGUNDOS") == 0 || strcmp(unidad_limpia, "MICROS") == 0 || strcmp(unidad_limpia, "US") == 0) {
        tiempo_us = (long long)valor;
    } else if (strcmp(unidad_limpia, "MILISEGUNDOS") == 0 || strcmp(unidad_limpia, "MS") == 0) {
        tiempo_us = (long long)(valor * 1000.0);
    } else if (strcmp(unidad_limpia, "SEGUNDOS") == 0 || strcmp(unidad_limpia, "S") == 0) {
        tiempo_us = (long long)(valor * 1000000.0);
    } else if (strcmp(unidad_limpia, "MINUTOS") == 0 || strcmp(unidad_limpia, "MIN") == 0) {
        tiempo_us = (long long)(valor * 60000000.0);
    } else {
        fprintf(stderr, "Error: Unidad inválida '%s'. Use MICROS, MILISEGUNDOS, SEGUNDOS o MINUTOS.\n", unidad_limpia);
        return;
    }
    
    if (tiempo_us > 0) {
#ifdef _WIN32
        Sleep((DWORD)(tiempo_us / 1000));
#else
        usleep((useconds_t)tiempo_us);
#endif
    }
}

void procesar_sistema(const char *argumento) {
    if (!argumento) return;
    char cmd[MAX_LINEA];
    strncpy(cmd, argumento, MAX_LINEA - 1);
    cmd[MAX_LINEA - 1] = '\0';
    cmd[MAX_LINEA - 1] = '\0';
    limpiar_string(cmd);
    (void)system(cmd);
}

// POSICIONAR CURSOR EN CONSOLA - CROSS-PLATFORM
void nico_posicionar_cursor(int fila, int columna) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) return;
    COORD coord = { (SHORT)(columna - 1), (SHORT)(fila - 1) };
    SetConsoleCursorPosition(hConsole, coord);
#else
    printf("\033[%d;%dH", fila, columna);
#endif
    fflush(stdout);
}

// OCULTARCURSOR / MOSTRARCURSOR
int cmd_ocultarcursor(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea; (void)linea_actual;
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        CONSOLE_CURSOR_INFO cci;
        GetConsoleCursorInfo(hConsole, &cci);
        cci.bVisible = FALSE;
        SetConsoleCursorInfo(hConsole, &cci);
    }
#else
    printf("\033[?25l");
    fflush(stdout);
#endif
    ctx->linea_num++;
    return 0;
}

int cmd_mostrarcursor(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea; (void)linea_actual;
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        CONSOLE_CURSOR_INFO cci;
        GetConsoleCursorInfo(hConsole, &cci);
        cci.bVisible = TRUE;
        SetConsoleCursorInfo(hConsole, &cci);
    }
#else
    printf("\033[?25h");
    fflush(stdout);
#endif
    restaurar_terminal_completa();
    ctx->linea_num++;
    return 0;
}

// COMANDO NATIVO: TIEMPOMS($var)
int cmd_tiempoms(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea; (void)linea_actual;
    
    const char *ptr = linea + 8;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    if (*ptr != '(') {
        fprintf(stderr, "Error línea %d: Sintaxis inválida. Uso: TIEMPOMS($variable).\n", linea_actual);
        return -1;
    }
    ptr++;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    if (*ptr != '$') {
        fprintf(stderr, "Error línea %d: El signo '$' es obligatorio. Ej: TIEMPOMS($tiempo).\n", linea_actual);
        return -1;
    }
    ptr++;
    
    char nombre[MAX_NOMBRE] = {0};
    int i = 0;
    while (*ptr && *ptr != ')' && *ptr != ' ' && *ptr != '\t' && i < MAX_NOMBRE - 1) {
        nombre[i++] = *ptr++;
    }
    nombre[i] = '\0';
    
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr != ')') {
        fprintf(stderr, "Error línea %d: Falta paréntesis de cierre en TIEMPOMS.\n", linea_actual);
        return -1;
    }
    
    if (!strlen(nombre)) {
        fprintf(stderr, "Error línea %d: Variable vacía en TIEMPOMS.\n", linea_actual);
        return -1;
    }
    
    long long ms = obtener_tiempo_ms();
    int encontrado = 0;

    if (scope_actual >= 0) {
        for (int s = scope_actual; s >= 0; s--) {
            ScopeLocal *scp = &scopes_locales[s];
            for (int v = 0; v < scp->num_variables; v++) {
                if (strcmp(scp->variables[v].nombre, nombre) == 0) {
                    switch (scp->variables[v].tipo) {
                        case 0: scp->variables[v].valor.valor_entero = (int)ms; break;
                        case 1: scp->variables[v].valor.valor_sin_signo = (unsigned int)ms; break;
                        case 2:
                        case 3: scp->variables[v].valor.valor_decimal = (double)ms; break;
                        case 4: scp->variables[v].valor.valor_caracter = (char)ms; break;
                        case 5: scp->variables[v].valor.valor_caracter_sin_signo = (unsigned char)ms; break;
                    }
                    encontrado = 1;
                    goto fin_asignacion_tiempoms;
                }
            }
        }
    }
    
    int idx = buscar_variable(nombre);
    if (idx >= 0) { variables[idx].valor = (int)ms; encontrado = 1; }
    else if ((idx = buscar_variable_sin_signo(nombre)) >= 0) {
        variables_sin_signo[idx].valor = (unsigned int)ms; encontrado = 1;
    }
    else if ((idx = buscar_variable_decimal(nombre)) >= 0) {
        variables_decimal[idx].valor = (double)ms; encontrado = 1;
    }
    else if ((idx = buscar_variable_caracter(nombre)) >= 0) {
        variables_caracter[idx].valor = (char)ms; encontrado = 1;
    }
    
fin_asignacion_tiempoms:
    if (!encontrado) {
        fprintf(stderr, "Error línea %d: Variable '$%s' no declarada en scope local ni global.\n", linea_actual, nombre);
    }
    
    ctx->linea_num++;
    return 0;
}

// ANCHOTERMINAL / ALTOTERMINAL
static int obtener_dim_terminal(int tipo) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return (tipo == 0) ? (csbi.srWindow.Right - csbi.srWindow.Left + 1)
                           : (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
    }
#else
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        return (tipo == 0) ? ws.ws_col : ws.ws_row;
    }
#endif
    return 80;
}

static int cmd_dim_terminal(const char *linea, CtxBloque *ctx, int linea_actual, int tipo_dim) {
    const char *cmd_name = (tipo_dim == 0) ? "ANCHOTERMINAL" : "ALTOTERMINAL";
    int cmd_len = strlen(cmd_name); 

    const char *ptr = linea + cmd_len;
    while (*ptr == ' ' || *ptr == '\t') ptr++;

    if (*ptr != '(') {
        fprintf(stderr, "Error línea %d: Sintaxis inválida. Uso: %s($variable).\n", linea_actual, cmd_name);
        return -1;
    }
    ptr++; while (*ptr == ' ' || *ptr == '\t') ptr++;

    if (*ptr != '$') {
        fprintf(stderr, "Error línea %d: El signo '$' es obligatorio. Ej: %s($ancho).\n", linea_actual, cmd_name);
        return -1;
    }
    ptr++;

    char nombre[MAX_NOMBRE] = {0};
    int i = 0;
    while (*ptr && *ptr != ')' && !isspace((unsigned char)*ptr) && i < MAX_NOMBRE - 1) {
        nombre[i++] = *ptr++;
    }
    nombre[i] = '\0';

    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr != ')') {
        fprintf(stderr, "Error línea %d: Falta paréntesis de cierre en %s.\n", linea_actual, cmd_name);
        return -1;
    }
    if (!strlen(nombre)) {
        fprintf(stderr, "Error línea %d: Variable vacía en %s.\n", linea_actual, cmd_name);
        return -1;
    }

    int valor = obtener_dim_terminal(tipo_dim);
    int encontrado = 0;

    if (scope_actual >= 0) {
        for (int s = scope_actual; s >= 0; s--) {
            ScopeLocal *scp = &scopes_locales[s];
            for (int v = 0; v < scp->num_variables; v++) {
                if (strcmp(scp->variables[v].nombre, nombre) == 0) {
                    switch (scp->variables[v].tipo) {
                        case 0: scp->variables[v].valor.valor_entero = valor; break;
                        case 1: scp->variables[v].valor.valor_sin_signo = (unsigned int)valor; break;
                        case 2: case 3: scp->variables[v].valor.valor_decimal = (double)valor; break;
                        case 4: scp->variables[v].valor.valor_caracter = (char)valor; break;
                        case 5: scp->variables[v].valor.valor_caracter_sin_signo = (unsigned char)valor; break;
                    }
                    encontrado = 1;
                    goto fin_asignacion_dim;
                }
            }
        }
    }

    int idx = buscar_variable(nombre);
    if (idx >= 0) { variables[idx].valor = valor; encontrado = 1; }
    else if ((idx = buscar_variable_sin_signo(nombre)) >= 0) { variables_sin_signo[idx].valor = (unsigned int)valor; encontrado = 1; }
    else if ((idx = buscar_variable_decimal(nombre)) >= 0) { variables_decimal[idx].valor = (double)valor; encontrado = 1; }
    else if ((idx = buscar_variable_caracter(nombre)) >= 0) { variables_caracter[idx].valor = (char)valor; encontrado = 1; }

fin_asignacion_dim:
    if (!encontrado) {
        fprintf(stderr, "Error línea %d: Variable '$%s' no declarada.\n", linea_actual, nombre);
    }

    ctx->linea_num++;
    return 0;
}

int cmd_anchoterminal(const char *linea, CtxBloque *ctx, int linea_actual) {
    return cmd_dim_terminal(linea, ctx, linea_actual, 0);
}

int cmd_altoterminal(const char *linea, CtxBloque *ctx, int linea_actual) {
    return cmd_dim_terminal(linea, ctx, linea_actual, 1);
}

// DIBUJARLINEA / DIBUJARCIRCULO: Primitivas ASCII
static const char* obtener_patron_dibujo(const char *token) {
    static char buf[8] = {0};
    memset(buf, 0, sizeof(buf));
    while (*token == ' ' || *token == '\t') token++;
    if (*token == '\0') return " ";

    if (*token == '"' || *token == '\'') {
        char q = *token++;
        int i = 0;
        while (*token && *token != q && i < 6) buf[i++] = *token++;
        return buf;
    }

    if (*token == '$') {
        token++;
        char nombre[MAX_NOMBRE] = {0};
        int i = 0;
        while (*token && *token != ' ' && *token != '\t' && *token != ')' && *token != ',' && 
               *token != '\r' && *token != '\n' && i < MAX_NOMBRE-1) nombre[i++] = *token++;

        char c = '?';
        if (scope_actual >= 0) {
            for (int s = scope_actual; s >= 0; s--) {
                for (int v = 0; v < scopes_locales[s].num_variables; v++) {
                    if (strcmp(scopes_locales[s].variables[v].nombre, nombre) == 0) {
                        switch(scopes_locales[s].variables[v].tipo) {
                            case 4: c = scopes_locales[s].variables[v].valor.valor_caracter; break;
                            case 5: c = (char)scopes_locales[s].variables[v].valor.valor_caracter_sin_signo; break;
                            default: c = (char)scopes_locales[s].variables[v].valor.valor_entero; break;
                        }
                        buf[0] = c; buf[1] = '\0';
                        return buf;
                    }
                }
            }
        }
        int idx = buscar_variable(nombre);
        if (idx >= 0) c = (char)variables[idx].valor;
        else if ((idx = buscar_variable_caracter(nombre)) >= 0) c = variables_caracter[idx].valor;
        
        buf[0] = c; buf[1] = '\0';
        return buf;
    }

    int i = 0;
    while (*token && *token != ',' && *token != ')' && *token != '\r' && *token != '\n' && i < 6)
        buf[i++] = *token++;
    buf[i] = '\0';
    return buf;
}

// DIBUJARLINEA($col1, $fil1, $col2, $fil2, "patrón")
int cmd_dibujarlinea(const char *linea, CtxBloque *ctx, int linea_actual) {
    const char *p = linea + 12;
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '(') { fprintf(stderr, "Error línea %d: Uso: DIBUJARLINEA($x1, $y1, $x2, $y2, \"char\").\n", linea_actual); ctx->linea_num++; return 0; }
    p++;

    const char *fin = strrchr(linea, ')');
    if (!fin || fin <= p) { fprintf(stderr, "Error línea %d: Falta paréntesis de cierre.\n", linea_actual); ctx->linea_num++; return 0; }

    char buf[MAX_LINEA];
    int len = (int)(fin - p);
    if (len >= MAX_LINEA) len = MAX_LINEA - 1;
    strncpy(buf, p, len); buf[len] = '\0';

    char *token = strtok(buf, ",");
    double args[4];
    int i = 0;
    while (token && i < 4) {
        int exito = 0;
        args[i] = evaluar_expresion_completa(token, &exito);
        if (!exito) { fprintf(stderr, "Error línea %d: Coordenada inválida.\n", linea_actual); ctx->linea_num++; return 0; }
        i++; token = strtok(NULL, ",");
    }
    if (i != 4 || !token) { fprintf(stderr, "Error línea %d: Se esperan 4 coordenadas y un patrón.\n", linea_actual); ctx->linea_num++; return 0; }

    const char *patron = obtener_patron_dibujo(token);
    int x1 = (int)args[0], y1 = (int)args[1];
    int x2 = (int)args[2], y2 = (int)args[3];
    if (x1 <= 0 || y1 <= 0 || x2 <= 0 || y2 <= 0) {
        fprintf(stderr, "Error línea %d: Coordenadas deben ser positivas.\n", linea_actual);
        ctx->linea_num++; return 0;
    }

    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1, sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    while (1) {
        nico_posicionar_cursor(y1, x1);
        printf("%s", patron);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
    fflush(stdout);
    ctx->linea_num++; return 0;
}

// DIBUJARCIRCULO($col_centro, $fil_centro, $radio, "patrón")
int cmd_dibujarcirculo(const char *linea, CtxBloque *ctx, int linea_actual) {
    const char *p = linea + 14;
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '(') { fprintf(stderr, "Error línea %d: Uso: DIBUJARCIRCULO($xc, $yc, $r, \"char\").\n", linea_actual); ctx->linea_num++; return 0; }
    p++;

    const char *fin = strrchr(linea, ')');
    if (!fin || fin <= p) { fprintf(stderr, "Error línea %d: Falta paréntesis de cierre.\n", linea_actual); ctx->linea_num++; return 0; }

    char buf[MAX_LINEA];
    int len = (int)(fin - p);
    if (len >= MAX_LINEA) len = MAX_LINEA - 1;
    strncpy(buf, p, len); buf[len] = '\0';

    char *token = strtok(buf, ",");
    double args[3];
    int i = 0;
    while (token && i < 3) {
        int exito = 0;
        args[i] = evaluar_expresion_completa(token, &exito);
        if (!exito) { fprintf(stderr, "Error línea %d: Valor inválido.\n", linea_actual); ctx->linea_num++; return 0; }
        i++; token = strtok(NULL, ",");
    }
    if (i != 3 || !token) { fprintf(stderr, "Error línea %d: Se esperan centro, radio y patrón.\n", linea_actual); ctx->linea_num++; return 0; }

    const char *patron = obtener_patron_dibujo(token);
    int xc = (int)args[0], yc = (int)args[1], r = (int)args[2];
    if (r <= 0) { fprintf(stderr, "Error línea %d: Radio debe ser positivo.\n", linea_actual); ctx->linea_num++; return 0; }

    int x = 0, y = r;
    int d = 1 - r;
    while (y >= x) {
        nico_posicionar_cursor(yc + y, xc + x); printf("%s", patron);
        nico_posicionar_cursor(yc + x, xc + y); printf("%s", patron);
        nico_posicionar_cursor(yc - x, xc + y); printf("%s", patron);
        nico_posicionar_cursor(yc - y, xc + x); printf("%s", patron);
        nico_posicionar_cursor(yc - y, xc - x); printf("%s", patron);
        nico_posicionar_cursor(yc - x, xc - y); printf("%s", patron);
        nico_posicionar_cursor(yc + x, xc - y); printf("%s", patron);
        nico_posicionar_cursor(yc + y, xc - x); printf("%s", patron);
        x++;
        if (d < 0) d += 2*x + 1;
        else { y--; d += 2*(x-y) + 1; }
    }
    fflush(stdout);
    ctx->linea_num++; return 0;
}

// RELLENARRECTANGULO($col1, $fil1, $col2, $fil2, "patrón")
int cmd_rellenarrectangulo(const char *linea, CtxBloque *ctx, int linea_actual) {
    const char *cmd = "RELLENARRECTANGULO";
    const char *p = linea + strlen(cmd);
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '(') { fprintf(stderr, "Error línea %d: Uso: RELLENARRECTANGULO($x1, $y1, $x2, $y2, \"char\").\n", linea_actual); ctx->linea_num++; return 0; }
    p++;

    const char *fin = strrchr(linea, ')');
    if (!fin || fin <= p) { fprintf(stderr, "Error línea %d: Falta paréntesis de cierre.\n", linea_actual); ctx->linea_num++; return 0; }

    char buf[MAX_LINEA];
    int len = (int)(fin - p);
    if (len >= MAX_LINEA) len = MAX_LINEA - 1;
    strncpy(buf, p, len); buf[len] = '\0';

    char *token = strtok(buf, ",");
    double args[4];
    int i = 0;
    while (token && i < 4) {
        int exito = 0;
        args[i] = evaluar_expresion_completa(token, &exito);
        if (!exito) { fprintf(stderr, "Error línea %d: Coordenada inválida.\n", linea_actual); ctx->linea_num++; return 0; }
        i++; token = strtok(NULL, ",");
    }
    if (i != 4 || !token) { fprintf(stderr, "Error línea %d: Se esperan 4 coordenadas y un patrón.\n", linea_actual); ctx->linea_num++; return 0; }

    const char *patron = obtener_patron_dibujo(token);
    int x1 = (int)args[0], y1 = (int)args[1];
    int x2 = (int)args[2], y2 = (int)args[3];

    int min_x = (x1 < x2) ? x1 : x2;
    int max_x = (x1 > x2) ? x1 : x2;
    int min_y = (y1 < y2) ? y1 : y2;
    int max_y = (y1 > y2) ? y1 : y2;

    if (min_x <= 0 || min_y <= 0) {
        fprintf(stderr, "Error línea %d: Coordenadas deben ser positivas.\n", linea_actual);
        ctx->linea_num++; return 0;
    }

    for (int y = min_y; y <= max_y; y++) {
        nico_posicionar_cursor(y, min_x);
        for (int x = min_x; x <= max_x; x++) {
            printf("%s", patron);
        }
    }
    fflush(stdout);
    ctx->linea_num++; return 0;
}

// TECLAMANTENIDA($codigo, $resultado)
int cmd_teclamantenida(const char *linea, CtxBloque *ctx, int linea_actual) {
    const char *ap = strchr(linea, '(');
    const char *cp = strrchr(linea, ')');
    if (!ap || !cp || cp <= ap) {
        fprintf(stderr, "Error línea %d: Uso: TECLAMANTENIDA($codigo, $resultado).\n", linea_actual);
        ctx->linea_num++; return 0;
    }

    char contenido[MAX_LINEA];
    int len = (int)(cp - ap - 1);
    if (len <= 0 || len >= MAX_LINEA - 1) {
        fprintf(stderr, "Error línea %d: Argumentos inválidos.\n", linea_actual);
        ctx->linea_num++; return 0;
    }
    strncpy(contenido, ap + 1, len);
    contenido[len] = '\0';

    char *coma = strchr(contenido, ',');
    if (!coma) {
        fprintf(stderr, "Error línea %d: Falta coma separadora.\n", linea_actual);
        ctx->linea_num++; return 0;
    }
    *coma = '\0';
    char *arg_cod = contenido;
    char *arg_res = coma + 1;
    
    while(*arg_cod == ' ' || *arg_cod == '\t') arg_cod++;
    while(*arg_res == ' ' || *arg_res == '\t') arg_res++;

    if (*arg_res != '$') {
        fprintf(stderr, "Error línea %d: Variable resultado debe empezar con '$'.\n", linea_actual);
        ctx->linea_num++; return 0;
    }
    arg_res++;
    char nombre_dest[MAX_NOMBRE] = {0};
    int i = 0;
    while (*arg_res && *arg_res != ' ' && *arg_res != '\t' && i < MAX_NOMBRE - 1) nombre_dest[i++] = *arg_res++;
    if (!strlen(nombre_dest)) {
        fprintf(stderr, "Error línea %d: Variable destino vacía.\n", linea_actual);
        ctx->linea_num++; return 0;
    }

    int exito = 0;
    double val_cod = evaluar_expresion_completa(arg_cod, &exito);
    if (!exito) {
        fprintf(stderr, "Error línea %d: Código inválido.\n", linea_actual);
        ctx->linea_num++; return 0;
    }
    int key_code = (int)val_cod;

    // Inicialización de modo raw / no-bloqueante CROSS-PLATFORM
    if (!raw_init) {
#ifdef _WIN32
        // Windows
        if (!win_console_saved) {
            h_stdin_win = GetStdHandle(STD_INPUT_HANDLE);
            GetConsoleMode(h_stdin_win, &orig_mode_in);
            SetConsoleMode(h_stdin_win, orig_mode_in & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT));
            
            h_stdout_win = GetStdHandle(STD_OUTPUT_HANDLE);
            GetConsoleMode(h_stdout_win, &orig_mode_out);
            SetConsoleMode(h_stdout_win, orig_mode_out | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            
            win_console_saved = 1;
        }
#else
        // Linux
        if (!termios_guardado) {
            tcgetattr(STDIN_FILENO, &orig_termios);
            termios_guardado = 1;
        }
        struct termios t = orig_termios;
        t.c_lflag &= ~(ICANON | ECHO);
        t.c_cc[VMIN] = 1;
        t.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
#endif
        raw_init = 1;
    }

    static char kb_buf[64];
    static int kb_len = 0;

    if (kb_len > 32) kb_len = 0;

#ifdef _WIN32
    #include <conio.h>
    if (_kbhit()) {
        if (kb_len < (int)sizeof(kb_buf) - 4) {
            int ch = _getch();
            kb_buf[kb_len++] = (char)ch;
            if (ch == 0 || ch == 224) {
                kb_buf[kb_len++] = (char)_getch();
            }
        }
    }
#else
    struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };
    if (poll(&pfd, 1, 0) > 0) {
        if (kb_len < (int)sizeof(kb_buf) - 4) {
            ssize_t n = read(STDIN_FILENO, kb_buf + kb_len, sizeof(kb_buf) - kb_len - 4);
            if (n > 0) kb_len += n;
        }
    }
#endif

#ifdef _WIN32
    if (kb_len > 16) {
        memmove(kb_buf, kb_buf + (kb_len - 16), 16);
        kb_len = 16;
    }
#endif
       int estado = 0;

    // ASCII (32-126): Búsqueda lineal para multi-key polling (juegos)
    if (key_code >= 32 && key_code <= 126) {
        for (int i = 0; i < kb_len; i++) {
            if ((unsigned char)kb_buf[i] == key_code) {
                estado = 1;
                memmove(&kb_buf[i], &kb_buf[i+1], kb_len - i - 1);
                kb_len--;
                break;
            }
        }
    }
    // ESCAPE y FLECHAS: Requieren prefijo '\x1B' al inicio del buffer
    else {
        while (kb_len > 0 && (unsigned char)kb_buf[0] != '\x1B') {
            memmove(kb_buf, kb_buf + 1, kb_len - 1);
            kb_len--;
        }

        if (kb_len > 0 && kb_buf[0] == '\x1B') {
            if (key_code == 27) {
                if (kb_len > 1 && kb_buf[1] == '[') {
                    int to_consume = (kb_len >= 3) ? 3 : kb_len;
                    memmove(kb_buf, kb_buf + to_consume, kb_len - to_consume);
                    kb_len -= to_consume;
                } else {
                    estado = 1;
                    memmove(kb_buf, kb_buf + 1, kb_len - 1); kb_len--;
                }
            }
            else if (key_code == 1001 && kb_len >= 3 && kb_buf[1]=='[' && kb_buf[2]=='A') {
                estado = 1; memmove(kb_buf, kb_buf+3, kb_len-3); kb_len-=3;
            }
            else if (key_code == 1002 && kb_len >= 3 && kb_buf[1]=='[' && kb_buf[2]=='B') {
                estado = 1; memmove(kb_buf, kb_buf+3, kb_len-3); kb_len-=3;
            }
            else if (key_code == 1003 && kb_len >= 3 && kb_buf[1]=='[' && kb_buf[2]=='C') {
                estado = 1; memmove(kb_buf, kb_buf+3, kb_len-3); kb_len-=3;
            }
            else if (key_code == 1004 && kb_len >= 3 && kb_buf[1]=='[' && kb_buf[2]=='D') {
                estado = 1; memmove(kb_buf, kb_buf+3, kb_len-3); kb_len-=3;
            }
        }
    }

    if (kb_len > 32) {
        memmove(kb_buf, kb_buf + (kb_len - 32), 32);
        kb_len = 32;
    }
    int encontrado = 0;
 
    if (scope_actual >= 0) {
        for (int s = scope_actual; s >= 0; s--) {
            for (int v = 0; v < scopes_locales[s].num_variables; v++) {
                if (strcmp(scopes_locales[s].variables[v].nombre, nombre_dest) == 0) {
                    switch (scopes_locales[s].variables[v].tipo) {
                        case 0: scopes_locales[s].variables[v].valor.valor_entero = estado; break;
                        case 1: scopes_locales[s].variables[v].valor.valor_sin_signo = (unsigned int)estado; break;
                        case 2: case 3: scopes_locales[s].variables[v].valor.valor_decimal = (double)estado; break;
                        case 4: scopes_locales[s].variables[v].valor.valor_caracter = (char)estado; break;
                        case 5: scopes_locales[s].variables[v].valor.valor_caracter_sin_signo = (unsigned char)estado; break;
                    }
                    encontrado = 1; goto fin_asig_tcla;
                }
            }
        }
    }
    int idx = buscar_variable(nombre_dest);
    if (idx >= 0) { variables[idx].valor = estado; encontrado = 1; }
    else if ((idx = buscar_variable_sin_signo(nombre_dest)) >= 0) { variables_sin_signo[idx].valor = (unsigned int)estado; encontrado = 1; }
    else if ((idx = buscar_variable_decimal(nombre_dest)) >= 0) { variables_decimal[idx].valor = (double)estado; encontrado = 1; }
    else if ((idx = buscar_variable_caracter(nombre_dest)) >= 0) { variables_caracter[idx].valor = (char)estado; encontrado = 1; }

fin_asig_tcla:
    if (!encontrado) {
        fprintf(stderr, "Error línea %d: Variable '$%s' no declarada.\n", linea_actual, nombre_dest);
    }

    ctx->linea_num++;
    return 0;
}

// COLISIONRECTANGULOS($x1,$y1,$w1,$h1, $x2,$y2,$w2,$h2, $resultado)
int cmd_colisionrectangulos(const char *linea, CtxBloque *ctx, int linea_actual) {
    const char *ap = strchr(linea, '(');
    const char *cp = strrchr(linea, ')');
    if (!ap || !cp || cp <= ap) {
        fprintf(stderr, "Error línea %d: Uso: COLISIONRECTANGULOS($x1,$y1,$w1,$h1, $x2,$y2,$w2,$h2, $resultado).\n", linea_actual);
        ctx->linea_num++; return 0;
    }

    char contenido[MAX_LINEA];
    int len = (int)(cp - ap - 1);
    if (len <= 0 || len >= MAX_LINEA - 1) {
        fprintf(stderr, "Error línea %d: Argumentos inválidos en COLISIONRECTANGULOS.\n", linea_actual);
        ctx->linea_num++; return 0;
    }
    strncpy(contenido, ap + 1, len); contenido[len] = '\0';

    char *tokens[9];
    char *p = contenido;
    int idx = 0;
    while (idx < 8 && p) {
        tokens[idx] = p;
        while (*p && *p != ',') p++;
        if (*p == ',') { *p = '\0'; p++; while(*p == ' ' || *p == '\t') p++; }
        idx++;
    }
    tokens[8] = p;

    double vals[8]; int exito;
    for (int i = 0; i < 8; i++) {
        vals[i] = evaluar_expresion_completa(tokens[i], &exito);
        if (!exito) {
            fprintf(stderr, "Error línea %d: Valor numérico inválido en coordenada %d.\n", linea_actual, i+1);
            ctx->linea_num++; return 0;
        }
    }

    char *res_ptr = tokens[8];
    while (*res_ptr == ' ' || *res_ptr == '\t') res_ptr++;
    if (*res_ptr != '$') {
        fprintf(stderr, "Error línea %d: El último argumento debe ser una variable ($resultado).\n", linea_actual);
        ctx->linea_num++; return 0;
    }
    res_ptr++;
    char nombre_dest[MAX_NOMBRE] = {0}; int i = 0;
    while (*res_ptr && *res_ptr != ' ' && *res_ptr != '\t' && i < MAX_NOMBRE - 1) nombre_dest[i++] = *res_ptr++;
    if (!strlen(nombre_dest)) {
        fprintf(stderr, "Error línea %d: Variable destino vacía.\n", linea_actual);
        ctx->linea_num++; return 0;
    }

    // Detección AABB (Axis-Aligned Bounding Box)
    int colision = (vals[0] < vals[4] + vals[6]) && (vals[0] + vals[2] > vals[4]) &&
                   (vals[1] < vals[5] + vals[7]) && (vals[1] + vals[3] > vals[5]) ? 1 : 0;

    int encontrado = 0;
    if (scope_actual >= 0) {
        for (int s = scope_actual; s >= 0; s--) {
            for (int v = 0; v < scopes_locales[s].num_variables; v++) {
                if (strcmp(scopes_locales[s].variables[v].nombre, nombre_dest) == 0) {
                    switch (scopes_locales[s].variables[v].tipo) {
                        case 0: scopes_locales[s].variables[v].valor.valor_entero = colision; break;
                        case 1: scopes_locales[s].variables[v].valor.valor_sin_signo = (unsigned int)colision; break;
                        case 2: case 3: scopes_locales[s].variables[v].valor.valor_decimal = (double)colision; break;
                        case 4: scopes_locales[s].variables[v].valor.valor_caracter = (char)colision; break;
                        case 5: scopes_locales[s].variables[v].valor.valor_caracter_sin_signo = (unsigned char)colision; break;
                    }
                    encontrado = 1; goto fin_asig_col;
                }
            }
        }
    }
    int idx_var = buscar_variable(nombre_dest);
    if (idx_var >= 0) { variables[idx_var].valor = colision; encontrado = 1; }
    else if ((idx_var = buscar_variable_sin_signo(nombre_dest)) >= 0) { variables_sin_signo[idx_var].valor = (unsigned int)colision; encontrado = 1; }
    else if ((idx_var = buscar_variable_decimal(nombre_dest)) >= 0) { variables_decimal[idx_var].valor = (double)colision; encontrado = 1; }
    else if ((idx_var = buscar_variable_caracter(nombre_dest)) >= 0) { variables_caracter[idx_var].valor = (char)colision; encontrado = 1; }

fin_asig_col:
    if (!encontrado) {
        fprintf(stderr, "Error línea %d: Variable '$%s' no declarada.\n", linea_actual, nombre_dest);
    }
    ctx->linea_num++; return 0;
}

void restaurar_terminal_completa(void) {
#ifdef _WIN32
    if (win_console_saved) {
        if (h_stdin_win != INVALID_HANDLE_VALUE) {
            SetConsoleMode(h_stdin_win, orig_mode_in);
        }
        if (h_stdout_win != INVALID_HANDLE_VALUE) {
            SetConsoleMode(h_stdout_win, orig_mode_out);
        }
        win_console_saved = 0;
    }
    printf("\033[0m\033[?25h\r\n");
    fflush(stdout);
#else
    if (termios_guardado) {
        tcsetattr(STDIN_FILENO, TCSADRAIN, &orig_termios);
        termios_guardado = 0;
    } else {
        struct termios sane;
        if (tcgetattr(STDIN_FILENO, &sane) == 0) {
            sane.c_lflag |= (ECHO | ICANON);
            sane.c_cc[VMIN] = 1;
            sane.c_cc[VTIME] = 0;
            tcsetattr(STDIN_FILENO, TCSADRAIN, &sane);
        }
    }
    printf("\033[0m\033[?25h\r\n");
    fflush(stdout);
    tcdrain(STDOUT_FILENO);
#endif
    raw_init = 0;
}
