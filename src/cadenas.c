/*
 * Nico v1.1.1 - Intérprete Educativo de Scripting en Español
 * @file:         cadenas.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Biblioteca de manipulación de texto. Provee operaciones nativas
 *                (COPIARTEXTO, LONGITUD, MAYUSCULAS, REEMPLAZAR, DIVIDIR, etc.).
 */
#define _DEFAULT_SOURCE
#include "nico.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

extern int buscar_matriz_texto_en_scope(const char *nombre, int scope_id);

// ELIMINAR COMENTARIOS
void eliminar_comentarios(char *linea) {
    if (!linea) return;
    int dentro_comillas = 0;
    char comilla_tipo = 0;
    for (int i = 0; linea[i] != '\0'; i++) {
        if (linea[i] == '"' || linea[i] == '\'') {
            if (i > 0 && linea[i-1] == '\\') continue;
            if (!dentro_comillas) { dentro_comillas = 1; comilla_tipo = linea[i]; }
            else if (linea[i] == comilla_tipo) { dentro_comillas = 0; comilla_tipo = 0; }
            continue;
        }
        if (dentro_comillas) continue;
        if (linea[i] == '/' && linea[i+1] == '*') {
            fprintf(stderr, "\nERROR: Comentarios de bloque no permitidos.\n");
            exit(1);
        }
        if (linea[i] == '/' && linea[i+1] == '/') { linea[i] = '\0'; return; }
    }
}

// IMPLEMENTACIONES C DE BAJO NIVEL
double nico_longitud_texto(const char *texto) {
    if (!texto) return 0;
    return (double)strlen(texto);
}

int nico_copiar_texto(char *destino, const char *origen, int max_len) {
    if (!origen) return -1;
    if (!destino) return -1;
    strncpy(destino, origen, max_len - 1);
    destino[max_len - 1] = '\0';
    return strlen(destino);
}

int nico_concatenar_texto(char *destino, const char *origen, int max_len) {
    if (!destino || !origen) return -1;
    int len_dest = strlen(destino);
    int len_orig = strlen(origen);
    if (len_dest + len_orig >= max_len) len_orig = max_len - len_dest - 1;
    strncat(destino, origen, len_orig);
    destino[max_len - 1] = '\0';
    return strlen(destino);
}

int nico_comparar_texto(const char *texto1, const char *texto2) {
    if (!texto1 || !texto2) return 1;
    return strcmp(texto1, texto2);
}

int nico_buscar_caracter(const char *texto, char caracter) {
    if (!texto) return -1;
    char *ptr = strchr(texto, caracter);
    if (!ptr) return -1;
    return (int)(ptr - texto);
}

int nico_buscar_texto(const char *texto, const char *busqueda) {
    if (!texto || !busqueda) return -1;
    char *ptr = strstr(texto, busqueda);
    if (!ptr) return -1;
    return (int)(ptr - texto);
}

int nico_extraer_texto(const char *texto, int inicio, int longitud, char *destino, int max_len) {
    if (!texto || !destino) return -1;
    int len = strlen(texto);
    if (inicio < 0 || inicio >= len) return -1;
    if (longitud < 0 || inicio + longitud > len) longitud = len - inicio;
    if (longitud >= max_len) longitud = max_len - 1;
    strncpy(destino, texto + inicio, longitud);
    destino[longitud] = '\0';
    return strlen(destino);
}

int nico_reemplazar_texto(char *texto, const char *busqueda, const char *reemplazo, int max_len) {
    if (!texto || !busqueda || !reemplazo) return -1;
    char *ptr = strstr(texto, busqueda);
    if (!ptr) return -1;
    int len_texto = strlen(texto);
    int len_busqueda = strlen(busqueda);
    int len_reemplazo = strlen(reemplazo);
    int nueva_longitud = len_texto - len_busqueda + len_reemplazo;
    if (nueva_longitud >= max_len) len_reemplazo = max_len - (ptr - texto) - 1;
    memmove(ptr + len_reemplazo, ptr + len_busqueda, strlen(ptr + len_busqueda) + 1);
    memcpy(ptr, reemplazo, len_reemplazo);
    texto[max_len - 1] = '\0';
    return strlen(texto);
}

int nico_a_mayusculas(char *texto) {
    if (!texto) return -1;
    for (int i = 0; texto[i]; i++) texto[i] = toupper((unsigned char)texto[i]);
    return strlen(texto);
}

int nico_a_minusculas(char *texto) {
    if (!texto) return -1;
    for (int i = 0; texto[i]; i++) texto[i] = tolower((unsigned char)texto[i]);
    return strlen(texto);
}

int nico_texto_a_entero(const char *texto) {
    if (!texto) return 0;
    return atoi(texto);
}

double nico_texto_a_decimal(const char *texto) {
    if (!texto) return 0.0;
    return atof(texto);
}

int nico_entero_a_texto(int numero, char *texto, int max_len) {
    if (!texto) return -1;
    return snprintf(texto, max_len, "%d", numero);
}

int nico_decimal_a_texto(double numero, char *texto, int max_len) {
    if (!texto) return -1;
    return snprintf(texto, max_len, "%g", numero);
}

int nico_texto_vacio(const char *texto) {
    if (!texto || strlen(texto) == 0) return 1;
    return 0;
}

int nico_recortar_texto(char *texto) {
    if (!texto) return -1;
    char *inicio = texto;
    while (*inicio == ' ' || *inicio == '\t') inicio++;
    char *fin = inicio + strlen(inicio) - 1;
    while (fin > inicio && (*fin == ' ' || *fin == '\t' || *fin == '\n' || *fin == '\r')) { *fin = '\0'; fin--; }
    if (inicio != texto) memmove(texto, inicio, strlen(inicio) + 1);
    return strlen(texto);
}

int nico_caracter_a_texto(char caracter, char *texto, int max_len) {
    if (!texto || max_len < 2) return -1;
    texto[0] = caracter;
    texto[1] = '\0';
    return 1;
}

int nico_texto_a_caracter(const char *texto) {
    if (!texto || strlen(texto) == 0) return 0;
    return (int)texto[0];
}

int nico_repetir_texto(const char *texto, int veces, char *destino, int max_len) {
    if (!texto || !destino || veces < 0) return -1;
    destino[0] = '\0';
    int len_texto = strlen(texto);
    for (int i = 0; i < veces; i++) {
        if ((size_t)strlen(destino) + (size_t)len_texto >= (size_t)max_len) break;
        strcat(destino, texto);
    }
    return strlen(destino);
}

int nico_dividir_texto(const char *texto, char separador, int indice, char *destino, int max_len) {
    if (!texto || !destino) return -1;
    int count = 0;
    const char *inicio = texto;
    const char *ptr = texto;
    while (*ptr) {
        if (*ptr == separador) {
            if (count == indice) {
                int len = ptr - inicio;
                if (len >= max_len) len = max_len - 1;
                strncpy(destino, inicio, len);
                destino[len] = '\0';
                return len;
            }
            count++;
            inicio = ptr + 1;
        }
        ptr++;
    }
    if (count == indice) {
        strncpy(destino, inicio, max_len - 1);
        destino[max_len - 1] = '\0';
        return strlen(destino);
    }
    destino[0] = '\0';
    return -1;
}

int obtener_texto_para_cadena(const char *arg, char *buffer, int max_len) {
    if (!arg || !buffer) return 0;
    
    // Caso 1: String literal entre comillas
    if (arg[0] == '"') {
        int i = 1, j = 0;
        while (arg[i] && arg[i] != '"' && j < max_len - 1) {
            if (arg[i] == '\\' && arg[i+1]) {
                i++;
                switch(arg[i]) {
                    case 'n': buffer[j++] = '\n'; break;
                    case 't': buffer[j++] = '\t'; break;
                    case '\\': buffer[j++] = '\\'; break;
                    case '"': buffer[j++] = '"'; break;
                    default: buffer[j++] = arg[i]; break;
                }
            } else {
                buffer[j++] = arg[i];
            }
            i++;
        }
        buffer[j] = '\0';
        return 1;
    }
    
    // Caso 2: Variable de texto simple o lista/matriz indexada
    char nombre[MAX_NOMBRE] = "";
    int i = 0;
    const char *ptr = arg;
    
    // Saltar $ si existe
    if (*ptr == '$') ptr++;
    
    // Extraer nombre base (sin corchetes)
    while (*ptr && *ptr != '[' && es_alnum(*ptr) && i < MAX_NOMBRE - 1) {
        nombre[i++] = *ptr++;
    }
    nombre[i] = '\0';
    
    // ¿Es una lista/matriz indexada?
    if (*ptr == '[') {
        ptr++; // saltar primer '['
    
        // Extraer primer índice (fila para matrices, índice para listas)
        char idx1_str[32] = "";
        int k = 0;
        while (*ptr && *ptr != ']' && k < 31) {
            idx1_str[k++] = *ptr++;
        }
        idx1_str[k] = '\0';
        int idx1 = atoi(idx1_str);
    
        if (*ptr == ']') ptr++; // saltar primer ']'
    
        // ¿Hay segundo corchete? (es matriz)
        if (*ptr == '[') {
            ptr++; // saltar segundo '['
            char idx2_str[32] = "";
            k = 0;
            while (*ptr && *ptr != ']' && k < 31) {
                idx2_str[k++] = *ptr++;
            }
            idx2_str[k] = '\0';
            int idx2 = atoi(idx2_str);
        
            if (*ptr == ']') ptr++; // saltar segundo ']'
        
            // Es MATRIZ de texto - buscar en scope local primero
            if (scope_actual >= 0) {
                for (int s = scope_actual; s >= 0; s--) {
                    ScopeLocal *scp = &scopes_locales[s];
                    for (int m = 0; m < scp->num_matrices; m++) {
                        if (strcmp(scp->nombres_matrices[m], nombre) == 0 && 
                        scp->tipos_matrices[m] == 6) { // tipo 6 = MATRIZ TEXTO
                            int idx_pool = scp->indices_matrices[m];
                            if (idx1 >= 0 && idx1 < scp->filas_matrices[m] && 
                            idx2 >= 0 && idx2 < scp->cols_matrices[m]) {
                                strncpy(buffer, matrices_texto[idx_pool].valores[idx1][idx2], max_len - 1);
                                buffer[max_len - 1] = '\0';
                                return 1;
                            }
                        }
                    }
                }
            }
        
            // Fallback a pool global
            int idx_mat = buscar_matriz_texto(nombre);
            if (idx_mat >= 0 && idx1 >= 0 && idx1 < matrices_texto[idx_mat].filas && 
            idx2 >= 0 && idx2 < matrices_texto[idx_mat].columnas) {
                strncpy(buffer, matrices_texto[idx_mat].valores[idx1][idx2], max_len - 1);
                buffer[max_len - 1] = '\0';
                return 1;
            }
        
            return 0; // Matriz no encontrada o índices fuera de rango
        }
    
        // Es LISTA de texto (un solo índice)
        int indice = idx1;
    
        // Buscar en scope local PRIMERO (shadowing)
        if (scope_actual >= 0) {
            for (int s = scope_actual; s >= 0; s--) {
                for (int l = 0; l < scopes_locales[s].num_listas; l++) {
                    if (strcmp(scopes_locales[s].nombres_listas[l], nombre) == 0 &&
                    scopes_locales[s].tipos_listas[l] == 6) { // tipo 6 = LISTA TEXTO
                        int idx_pool = scopes_locales[s].indices_listas[l];
                        if (indice >= 0 && indice < listas_texto[idx_pool].longitud) {
                            strncpy(buffer, listas_texto[idx_pool].valores[indice], max_len - 1);
                            buffer[max_len - 1] = '\0';
                            return 1;
                        }
                    }
                }
            }
        }
    
        // Fallback a pool global
        int idx_lista = buscar_lista_texto(nombre);
        if (idx_lista >= 0 && indice >= 0 && indice < listas_texto[idx_lista].longitud) {
            strncpy(buffer, listas_texto[idx_lista].valores[indice], max_len - 1);
            buffer[max_len - 1] = '\0';
            return 1;
        }
    
        return 0; // Lista no encontrada
    }
 
    // Caso 3: Variable de texto simple (sin índices)
    int idx = buscar_texto_var(nombre);
    if (idx >= 0) { 
        strncpy(buffer, texto_vars[idx].valor, max_len - 1); 
        buffer[max_len - 1] = '\0'; 
        return 1; 
    }
    
    idx = buscar_texto_constante(nombre);
    if (idx >= 0) { 
        strncpy(buffer, texto_constantes[idx].valor, max_len - 1); 
        buffer[max_len - 1] = '\0'; 
        return 1; 
    }
    
    // Buscar en TEXTO EXTENSO (global y local)
    int es_local_ext, idx_ext, scope_ext;
    if (buscar_texto_extenso(nombre, &es_local_ext, &idx_ext, &scope_ext) >= 0) {
        const char *valor = NULL;
        if (es_local_ext) {
            valor = scopes_locales[scope_ext].textos_ext[idx_ext].valor;
        } else {
            valor = textos_ext_globales[idx_ext].valor;
        }
        if (valor) {
            strncpy(buffer, valor, max_len - 1);
            buffer[max_len - 1] = '\0';
            return 1;
        }
    }

    // Buscar en scopes locales para variables simples
    if (scope_actual >= 0 && buscar_texto_local(nombre, buffer)) return 1;
    
    return 0;
}

double ejecutar_funcion_cadena(const char *nombre_func, char *args[], int num_args, int *exito) {
    *exito = 0;
    
    if (strcmp(nombre_func, "LONGITUDTEXTO") == 0 && num_args == 1) {
        char texto[MAX_TEXTO_LEN] = "";
        if (!obtener_texto_para_cadena(args[0], texto, MAX_TEXTO_LEN)) return 0;
        *exito = 1;
        return nico_longitud_texto(texto);
    }
    else if (strcmp(nombre_func, "BUSCARTEXTO") == 0 && num_args == 2) {
        char t[MAX_TEXTO_LEN] = "", b[MAX_TEXTO_LEN] = "";
        if (!obtener_texto_para_cadena(args[0], t, MAX_TEXTO_LEN)) return 0;
        if (!obtener_texto_para_cadena(args[1], b, MAX_TEXTO_LEN)) return 0;
        *exito = 1;
        return (double)nico_buscar_texto(t, b);
    }
    else if (strcmp(nombre_func, "BUSCARCARACTER") == 0 && num_args == 2) {
        char texto[MAX_TEXTO_LEN] = "";
        if (!obtener_texto_para_cadena(args[0], texto, MAX_TEXTO_LEN)) return 0;
        char caracter = args[1][0];
        if (args[1][0] == '\'' && args[1][1]) caracter = args[1][1];
        *exito = 1;
        return (double)nico_buscar_caracter(texto, caracter);
    }
    else if (strcmp(nombre_func, "COMPARARTEXTO") == 0 && num_args == 2) {
        char t1[MAX_TEXTO_LEN] = "", t2[MAX_TEXTO_LEN] = "";
        if (!obtener_texto_para_cadena(args[0], t1, MAX_TEXTO_LEN)) return 0;
        if (!obtener_texto_para_cadena(args[1], t2, MAX_TEXTO_LEN)) return 0;
        *exito = 1;
        return (double)nico_comparar_texto(t1, t2);
    }
    else if (strcmp(nombre_func, "TEXTOVACIO") == 0 && num_args == 1) {
        char texto[MAX_TEXTO_LEN] = "";
        if (!obtener_texto_para_cadena(args[0], texto, MAX_TEXTO_LEN)) return 0;
        *exito = 1;
        return (double)nico_texto_vacio(texto);
    }
    else if (strcmp(nombre_func, "TEXTOAENTERO") == 0 && num_args == 1) {
        char texto[MAX_TEXTO_LEN] = "";
        if (!obtener_texto_para_cadena(args[0], texto, MAX_TEXTO_LEN)) return 0;
        *exito = 1;
        return (double)nico_texto_a_entero(texto);
    }
    else if (strcmp(nombre_func, "TEXTOADECIMAL") == 0 && num_args == 1) {
        char texto[MAX_TEXTO_LEN] = "";
        if (!obtener_texto_para_cadena(args[0], texto, MAX_TEXTO_LEN)) return 0;
        *exito = 1;
        return nico_texto_a_decimal(texto);
    }
    else if (strcmp(nombre_func, "TEXTOACARACTER") == 0 && num_args == 1) {
        char texto[MAX_TEXTO_LEN] = "";
        if (!obtener_texto_para_cadena(args[0], texto, MAX_TEXTO_LEN)) return 0;
        *exito = 1;
        return (double)nico_texto_a_caracter(texto);
    }
    return 0;
}

int ejecutar_comando_cadena(const char *nombre_cmd, char *args[], int num_args) {
    if (strcmp(nombre_cmd, "COPIARTEXTO") == 0 && num_args == 2) {
        // RUTA MATRIZ TEXTO
        const char *raw_dest = args[0];
        const char *b1 = strchr(raw_dest, '[');
        const char *b2 = b1 ? strchr(b1 + 1, '[') : NULL;
        
        if (b1 && b2) {
            char nom_mat[MAX_NOMBRE] = "";
            int mi = 0;
            const char *src = raw_dest;
            if (*src == '$') src++;
            while (*src && *src != '[' && mi < MAX_NOMBRE - 1) nom_mat[mi++] = *src++;
            nom_mat[mi] = '\0';
            
            char idx_f[32] = "", idx_c[32] = "";
            int k = 0; const char *t = b1 + 1;
            while (*t && *t != ']' && k < 31) { idx_f[k++] = *t++; }
            idx_f[k] = '\0';
            k = 0; t = b2 + 1;
            while (*t && *t != ']' && k < 31) { idx_c[k++] = *t++; }
            idx_c[k] = '\0'; 

            int f = atoi(idx_f);
            int c = atoi(idx_c);
            
            char origen_buf[MAX_TEXTO_LEN];
            if (!obtener_texto_para_cadena(args[1], origen_buf, MAX_TEXTO_LEN)) return -1;
            
            int idx_mat = buscar_matriz_texto_en_scope(nom_mat, scope_actual);
            if (idx_mat >= 0 && f >= 0 && f < matrices_texto[idx_mat].filas && c >= 0 && c < matrices_texto[idx_mat].columnas) {
                nico_copiar_texto(matrices_texto[idx_mat].valores[f][c], origen_buf, MAX_TEXTO_LEN);
                return 0;
            }
            return -1;
        }
        
        char destino_nombre[MAX_NOMBRE] = "", origen_buf[MAX_TEXTO_LEN];
        int es_lista = 0, indice_lista = -1;
        
        const char *dest_raw = args[0];
        if (*dest_raw == '$') dest_raw++;
        
        int i = 0;
        while (es_alnum(*dest_raw) && i < MAX_NOMBRE - 1) {
            destino_nombre[i++] = *dest_raw++;
        }
        destino_nombre[i] = '\0';
        
        if (*dest_raw == '[') {
            dest_raw++;
            indice_lista = atoi(dest_raw);
            es_lista = 1;
        }
        
        if (!obtener_texto_para_cadena(args[1], origen_buf, MAX_TEXTO_LEN)) return -1;
        
        // Ruta A: Lista de texto
        if (es_lista) {
            if (scope_actual >= 0) {
                for (int s = scope_actual; s >= 0; s--) {
                    for (int l = 0; l < scopes_locales[s].num_listas; l++) {
                        if (strcmp(scopes_locales[s].nombres_listas[l], destino_nombre) == 0 && scopes_locales[s].tipos_listas[l] == 6) {
                           int idx_pool = scopes_locales[s].indices_listas[l];
                            if (indice_lista >= 0 && indice_lista < listas_texto[idx_pool].longitud) {
                                nico_copiar_texto(listas_texto[idx_pool].valores[indice_lista], origen_buf, MAX_TEXTO_LEN);
                                return 0;
                            }
                        }
                    }
                }
            }
            int idx_lista = buscar_lista_texto(destino_nombre);
            if (idx_lista >= 0 && indice_lista >= 0 && indice_lista < listas_texto[idx_lista].longitud) {
                nico_copiar_texto(listas_texto[idx_lista].valores[indice_lista], origen_buf, MAX_TEXTO_LEN);
                return 0;
            }
            return -1;
        }
        
        // Ruta B: Variable de texto simple
        int idx = buscar_texto_var(destino_nombre);
        if (idx >= 0) {
            nico_copiar_texto(texto_vars[idx].valor, origen_buf, MAX_TEXTO_LEN);
            return 0;
        }
        
        // Ruta C: Texto local en scope
        if (scope_actual >= 0) {
            int idx_pool = -1;
            for (int s = scope_actual; s >= 0; s--) {
                for (int v = 0; v < scopes_locales[s].num_textos; v++) {
                    if (strcmp(scopes_locales[s].nombres_textos[v], destino_nombre) == 0) {
                        idx_pool = scopes_locales[s].indices_textos[v];
                        break;
                    }
                }
                if (idx_pool != -1) break;
            }
            if (idx_pool != -1) {
                nico_copiar_texto(texto_vars[idx_pool].valor, origen_buf, MAX_TEXTO_LEN);
                return 0;
            }
        }

        // ✅ NUEVO: Ruta D: TEXTO EXTENSO
        int es_local_ext, idx_ext, scope_ext;
        if (buscar_texto_extenso(destino_nombre, &es_local_ext, &idx_ext, &scope_ext) >= 0) {
            return asignar_texto_extenso_valor(es_local_ext, idx_ext, scope_ext, origen_buf);
        }
        
        return -1;
    }

    else if (strcmp(nombre_cmd, "CONCATENARTEXTO") == 0 && num_args == 2) {
        char destino_nombre[MAX_NOMBRE] = "", origen_buf[MAX_TEXTO_LEN];
        const char *dest_raw = args[0];
        if (*dest_raw == '$') dest_raw++;
        int i = 0;
        while (es_alnum(*dest_raw) && i < MAX_NOMBRE - 1) destino_nombre[i++] = *dest_raw++;
        destino_nombre[i] = '\0';

        if (!obtener_texto_para_cadena(args[1], origen_buf, MAX_TEXTO_LEN)) return -1;
        
        int idx = buscar_texto_var(destino_nombre);
        if (idx >= 0) {
            return nico_concatenar_texto(texto_vars[idx].valor, origen_buf, MAX_TEXTO_LEN) != -1 ? 0 : -1;
        }
        
        int es_local_ext, idx_ext, scope_ext;
        if (buscar_texto_extenso(destino_nombre, &es_local_ext, &idx_ext, &scope_ext) >= 0) {
            const char *valor_actual = es_local_ext ? scopes_locales[scope_ext].textos_ext[idx_ext].valor : textos_ext_globales[idx_ext].valor;
            if (valor_actual) {
                size_t len_total = strlen(valor_actual) + strlen(origen_buf) + 1;
                char *buffer_concat = malloc(len_total);
                if (buffer_concat) {
                    strcpy(buffer_concat, valor_actual);
                    if (nico_concatenar_texto(buffer_concat, origen_buf, len_total) != -1) {
                        int res = asignar_texto_extenso_valor(es_local_ext, idx_ext, scope_ext, buffer_concat);
                        free(buffer_concat);
                        return res;
                    }
                    free(buffer_concat);
                }
            }
        }
        return -1;
    }
    
    else if (strcmp(nombre_cmd, "MAYUSCULAS") == 0 && num_args == 1) {
        char nombre[MAX_NOMBRE] = "";
        const char *dest_raw = args[0];
        if (*dest_raw == '$') dest_raw++;
        int i = 0;
        while (es_alnum(*dest_raw) && i < MAX_NOMBRE - 1) nombre[i++] = *dest_raw++;
        nombre[i] = '\0';
        
        int idx = buscar_texto_var(nombre);
        if (idx >= 0) { nico_a_mayusculas(texto_vars[idx].valor); return 0; }
        
        int es_local_ext, idx_ext, scope_ext;
        if (buscar_texto_extenso(nombre, &es_local_ext, &idx_ext, &scope_ext) >= 0) {
            const char *valor_actual = es_local_ext ? scopes_locales[scope_ext].textos_ext[idx_ext].valor : textos_ext_globales[idx_ext].valor;
            if (valor_actual) {
                char *buffer = malloc(strlen(valor_actual) + 1);
                if (buffer) {
                    strcpy(buffer, valor_actual);
                    nico_a_mayusculas(buffer);
                    int res = asignar_texto_extenso_valor(es_local_ext, idx_ext, scope_ext, buffer);
                    free(buffer);
                    return res;
                }
            }
        }
        return -1;
    }
    
    else if (strcmp(nombre_cmd, "MINUSCULAS") == 0 && num_args == 1) {
        char nombre[MAX_NOMBRE] = "";
        const char *dest_raw = args[0];
        if (*dest_raw == '$') dest_raw++;
        int i = 0;
        while (es_alnum(*dest_raw) && i < MAX_NOMBRE - 1) nombre[i++] = *dest_raw++;
        nombre[i] = '\0';
        
        int idx = buscar_texto_var(nombre);
        if (idx >= 0) { nico_a_minusculas(texto_vars[idx].valor); return 0; }
        
        int es_local_ext, idx_ext, scope_ext;
        if (buscar_texto_extenso(nombre, &es_local_ext, &idx_ext, &scope_ext) >= 0) {
            const char *valor_actual = es_local_ext ? scopes_locales[scope_ext].textos_ext[idx_ext].valor : textos_ext_globales[idx_ext].valor;
            if (valor_actual) {
                char *buffer = malloc(strlen(valor_actual) + 1);
                if (buffer) {
                    strcpy(buffer, valor_actual);
                    nico_a_minusculas(buffer);
                    int res = asignar_texto_extenso_valor(es_local_ext, idx_ext, scope_ext, buffer);
                    free(buffer);
                    return res;
                }
            }
        }
        return -1;
    }
    
    else if (strcmp(nombre_cmd, "RECORTARTEXTO") == 0 && num_args == 1) {
        char nombre[MAX_NOMBRE] = "";
        const char *dest_raw = args[0];
        if (*dest_raw == '$') dest_raw++;
        int i = 0;
        while (es_alnum(*dest_raw) && i < MAX_NOMBRE - 1) nombre[i++] = *dest_raw++;
        nombre[i] = '\0';
        
        int idx = buscar_texto_var(nombre);
        if (idx >= 0) { nico_recortar_texto(texto_vars[idx].valor); return 0; }
        
        int es_local_ext, idx_ext, scope_ext;
        if (buscar_texto_extenso(nombre, &es_local_ext, &idx_ext, &scope_ext) >= 0) {
            const char *valor_actual = es_local_ext ? scopes_locales[scope_ext].textos_ext[idx_ext].valor : textos_ext_globales[idx_ext].valor;
            if (valor_actual) {
                char *buffer = malloc(strlen(valor_actual) + 1);
                if (buffer) {
                    strcpy(buffer, valor_actual);
                    nico_recortar_texto(buffer);
                    int res = asignar_texto_extenso_valor(es_local_ext, idx_ext, scope_ext, buffer);
                    free(buffer);
                    return res;
                }
            }
        }
        return -1;
    }
    
    else if (strcmp(nombre_cmd, "REEMPLAZARTEXTO") == 0 && num_args == 3) {
        char destino_nombre[MAX_NOMBRE] = "", busqueda[MAX_TEXTO_LEN], reemplazo[MAX_TEXTO_LEN];
        const char *dest_raw = args[0];
        if (*dest_raw == '$') dest_raw++;
        int i = 0;
        while (es_alnum(*dest_raw) && i < MAX_NOMBRE - 1) destino_nombre[i++] = *dest_raw++;
        destino_nombre[i] = '\0';
        
        if (!obtener_texto_para_cadena(args[1], busqueda, MAX_TEXTO_LEN)) return -1;
        if (!obtener_texto_para_cadena(args[2], reemplazo, MAX_TEXTO_LEN)) return -1;
        
        int idx = buscar_texto_var(destino_nombre);
        if (idx >= 0) {
            return nico_reemplazar_texto(texto_vars[idx].valor, busqueda, reemplazo, MAX_TEXTO_LEN) != -1 ? 0 : -1;
        }
        
        int es_local_ext, idx_ext, scope_ext;
        if (buscar_texto_extenso(destino_nombre, &es_local_ext, &idx_ext, &scope_ext) >= 0) {
            const char *valor_actual = es_local_ext ? scopes_locales[scope_ext].textos_ext[idx_ext].valor : textos_ext_globales[idx_ext].valor;
            if (valor_actual) {
                size_t safe_len = strlen(valor_actual) + strlen(reemplazo) * 10 + 64;
                char *buffer = malloc(safe_len);
                if (buffer) {
                    strcpy(buffer, valor_actual);
                    nico_reemplazar_texto(buffer, busqueda, reemplazo, safe_len);
                    int res = asignar_texto_extenso_valor(es_local_ext, idx_ext, scope_ext, buffer);
                    free(buffer);
                    return res;
                }
            }
        }
        return -1;
    }
    
    else if (strcmp(nombre_cmd, "REPETIRTEXTO") == 0 && num_args == 3) {
        char texto_buf[MAX_TEXTO_LEN], destino_nombre[MAX_NOMBRE] = "";
        int veces = atoi(args[1]);
        if (!obtener_texto_para_cadena(args[0], texto_buf, MAX_TEXTO_LEN)) return -1;
        
        const char *dest_raw = args[2];
        if (*dest_raw == '$') dest_raw++;
        int i = 0;
        while (es_alnum(*dest_raw) && i < MAX_NOMBRE - 1) destino_nombre[i++] = *dest_raw++;
        destino_nombre[i] = '\0';
        
        int idx = buscar_texto_var(destino_nombre);
        if (idx >= 0) {
            return nico_repetir_texto(texto_buf, veces, texto_vars[idx].valor, MAX_TEXTO_LEN) != -1 ? 0 : -1;
        }
        
        int es_local_ext, idx_ext, scope_ext;
        if (buscar_texto_extenso(destino_nombre, &es_local_ext, &idx_ext, &scope_ext) >= 0) {
            size_t safe_len = (strlen(texto_buf) * veces) + 1;
            char *buffer = malloc(safe_len);
            if (buffer) {
                nico_repetir_texto(texto_buf, veces, buffer, safe_len);
                int res = asignar_texto_extenso_valor(es_local_ext, idx_ext, scope_ext, buffer);
                free(buffer);
                return res;
            }
        }
        return -1;
    }
    
    else if (strcmp(nombre_cmd, "EXTRAERTEXTO") == 0 && num_args == 4) {
        char texto_buf[MAX_TEXTO_LEN], destino_nombre[MAX_NOMBRE] = "";
        int inicio = atoi(args[1]), longitud = atoi(args[2]);
        if (!obtener_texto_para_cadena(args[0], texto_buf, MAX_TEXTO_LEN)) return -1;
        
        const char *dest_raw = args[3];
        if (*dest_raw == '$') dest_raw++;
        int i = 0;
        while (es_alnum(*dest_raw) && i < MAX_NOMBRE - 1) destino_nombre[i++] = *dest_raw++;
        destino_nombre[i] = '\0';
        
        int idx = buscar_texto_var(destino_nombre);
        if (idx >= 0) {
            return nico_extraer_texto(texto_buf, inicio, longitud, texto_vars[idx].valor, MAX_TEXTO_LEN) != -1 ? 0 : -1;
        }
        
        int es_local_ext, idx_ext, scope_ext;
        if (buscar_texto_extenso(destino_nombre, &es_local_ext, &idx_ext, &scope_ext) >= 0) {
            char *buffer = malloc(longitud + 1);
            if (buffer) {
                nico_extraer_texto(texto_buf, inicio, longitud, buffer, longitud + 1);
                int res = asignar_texto_extenso_valor(es_local_ext, idx_ext, scope_ext, buffer);
                free(buffer);
                return res;
            }
        }
        return -1;
    }
    
    else if (strcmp(nombre_cmd, "ENTEROATEXTO") == 0 && num_args == 2) {
        char destino_nombre[MAX_NOMBRE] = "";
        int exito_val; double val = evaluar_expresion_completa(args[0], &exito_val);
        int numero = exito_val ? (int)val : atoi(args[0]);
        
        const char *dest_raw = args[1];
        if (*dest_raw == '$') dest_raw++;
        int i = 0;
        while (es_alnum(*dest_raw) && i < MAX_NOMBRE - 1) destino_nombre[i++] = *dest_raw++;
        destino_nombre[i] = '\0';
        
        int idx = buscar_texto_var(destino_nombre);
        if (idx >= 0) {
            return nico_entero_a_texto(numero, texto_vars[idx].valor, MAX_TEXTO_LEN) != -1 ? 0 : -1;
        }
        
        int es_local_ext, idx_ext, scope_ext;
        if (buscar_texto_extenso(destino_nombre, &es_local_ext, &idx_ext, &scope_ext) >= 0) {
            char *buffer = malloc(32);
            if (buffer) {
                nico_entero_a_texto(numero, buffer, 32);
                int res = asignar_texto_extenso_valor(es_local_ext, idx_ext, scope_ext, buffer);
                free(buffer);
                return res;
            }
        }
        return -1;
    }
    
    else if (strcmp(nombre_cmd, "DECIMALATEXTO") == 0 && num_args == 2) {
        char destino_nombre[MAX_NOMBRE] = "";
        int exito_val; double numero = evaluar_expresion_completa(args[0], &exito_val);
        if (!exito_val) numero = atof(args[0]);
        
        const char *dest_raw = args[1];
        if (*dest_raw == '$') dest_raw++;
        int i = 0;
        while (es_alnum(*dest_raw) && i < MAX_NOMBRE - 1) destino_nombre[i++] = *dest_raw++;
        destino_nombre[i] = '\0';
        
        int idx = buscar_texto_var(destino_nombre);
        if (idx >= 0) {
            return nico_decimal_a_texto(numero, texto_vars[idx].valor, MAX_TEXTO_LEN) != -1 ? 0 : -1;
        }
        
        int es_local_ext, idx_ext, scope_ext;
        if (buscar_texto_extenso(destino_nombre, &es_local_ext, &idx_ext, &scope_ext) >= 0) {
            char *buffer = malloc(64);
            if (buffer) {
                nico_decimal_a_texto(numero, buffer, 64);
                int res = asignar_texto_extenso_valor(es_local_ext, idx_ext, scope_ext, buffer);
                free(buffer);
                return res;
            }
        }
        return -1;
    }
    
    else if (strcmp(nombre_cmd, "DIVIDIRTEXTO") == 0 && num_args == 4) {
        char texto_buf[MAX_TEXTO_LEN], destino_nombre[MAX_NOMBRE] = "";
        if (!obtener_texto_para_cadena(args[0], texto_buf, MAX_TEXTO_LEN)) return -1;

        char *sep_ptr = args[1];
        char separador = sep_ptr[0];
        size_t len_sep = strlen(sep_ptr);
        if (len_sep >= 2 && ((sep_ptr[0] == '"' && sep_ptr[len_sep-1] == '"') || (sep_ptr[0] == '\'' && sep_ptr[len_sep-1] == '\''))) {
            separador = sep_ptr[1];
        }

        int exito_idx; 
        double val_idx = evaluar_expresion_completa(args[2], &exito_idx);
        int indice = exito_idx ? (int)val_idx : atoi(args[2]);

        const char *dest_raw = args[3];
        if (*dest_raw == '$') dest_raw++;
        int i = 0;
        while (es_alnum(*dest_raw) && i < MAX_NOMBRE - 1) destino_nombre[i++] = *dest_raw++;
        destino_nombre[i] = '\0';
        
        int idx = buscar_texto_var(destino_nombre);
        if (idx >= 0) {
            return nico_dividir_texto(texto_buf, separador, indice, texto_vars[idx].valor, MAX_TEXTO_LEN) != -1 ? 0 : -1;
        }
        
        int es_local_ext, idx_ext, scope_ext;
        if (buscar_texto_extenso(destino_nombre, &es_local_ext, &idx_ext, &scope_ext) >= 0) {
            char *buffer = malloc(strlen(texto_buf) + 1);
            if (buffer) {
                nico_dividir_texto(texto_buf, separador, indice, buffer, strlen(texto_buf) + 1);
                int res = asignar_texto_extenso_valor(es_local_ext, idx_ext, scope_ext, buffer);
                free(buffer);
                return res;
            }
        }
        return -1;
    }
    
    else if (strcmp(nombre_cmd, "CARACTERATEXTO") == 0 && num_args == 2) {
        char destino_nombre[MAX_NOMBRE] = "";
        int exito_val;
        double val = evaluar_expresion_completa(args[0], &exito_val);
        char caracter = exito_val ? (char)((int)val) : (args[0][0] == '\'' && args[0][1] ? args[0][1] : args[0][0]);
        
        const char *dest_raw = args[1];
        if (*dest_raw == '$') dest_raw++;
        int i = 0;
        while (es_alnum(*dest_raw) && i < MAX_NOMBRE - 1) destino_nombre[i++] = *dest_raw++;
        destino_nombre[i] = '\0';
        
        int idx = buscar_texto_var(destino_nombre);
        if (idx >= 0) {
            return nico_caracter_a_texto(caracter, texto_vars[idx].valor, MAX_TEXTO_LEN) != -1 ? 0 : -1;
        }
        
        int es_local_ext, idx_ext, scope_ext;
        if (buscar_texto_extenso(destino_nombre, &es_local_ext, &idx_ext, &scope_ext) >= 0) {
            char *buffer = malloc(2);
            if (buffer) {
                nico_caracter_a_texto(caracter, buffer, 2);
                int res = asignar_texto_extenso_valor(es_local_ext, idx_ext, scope_ext, buffer);
                free(buffer);
                return res;
            }
        }
        return -1;
    }
    
    return -1;
}

int es_funcion_cadena_valida(const char *nombre) {
    const char *funcs[] = {
        "LONGITUDTEXTO", "BUSCARTEXTO", "BUSCARCARACTER", "COMPARARTEXTO",
        "TEXTOVACIO", "TEXTOAENTERO", "TEXTOADECIMAL", "TEXTOACARACTER",
        "COPIARTEXTO", "CONCATENARTEXTO", "MAYUSCULAS", "MINUSCULAS",
        "RECORTARTEXTO", "REEMPLAZARTEXTO", "REPETIRTEXTO", "EXTRAERTEXTO",
        "DIVIDIRTEXTO", "ENTEROATEXTO", "DECIMALATEXTO", "CARACTERATEXTO",
        NULL
    };
    for (int i = 0; funcs[i] != NULL; i++) {
        if (strcmp(nombre, funcs[i]) == 0) return 1;
    }
    return 0;
}
