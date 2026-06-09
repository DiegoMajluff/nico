/*
 * Nico v1.1.1 - Intérprete Educativo de Scripting en Español
 * @file:         scopes.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Gestión de memoria y alcances (scopes). Maneja la creación y eliminación
 *                de contextos locales, registro/búsqueda de variables, textos, listas y
 *                matrices, así como el mantenimiento de los pools de datos globales.
 */
#include "nico.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// TIPOS PARA LISTAS (DEBEN COINCIDIR CON declaraciones.c)
#define TIPO_LISTA_ENTERA              0
#define TIPO_LISTA_DECIMAL             1   
#define TIPO_LISTA_ENTERA_SIN_SIGNO    2   
#define TIPO_LISTA_DECIMAL_SIN_SIGNO   3
#define TIPO_LISTA_CARACTER            4
#define TIPO_LISTA_CARACTER_SIN_SIGNO  5

// DEFINICIONES GLOBALES (ASIGNACIÓN DE MEMORIA REAL)
int num_variables = 0;
Variable variables[MAX_VARS];

int num_variables_sin_signo = 0;
VariableSinSigno variables_sin_signo[MAX_VARS_SIN_SIGNO];

int num_variables_decimal = 0;
VariableDecimal variables_decimal[MAX_VARS_DECIMAL];

int num_variables_decimal_sin_signo = 0;
VariableDecimalSinSigno variables_decimal_sin_signo[MAX_VARS_DECIMAL_SIN_SIGNO];

int num_variables_caracter = 0;
VariableCaracter variables_caracter[MAX_VARS_CARACTER];

int num_variables_caracter_sin_signo = 0;
VariableCaracterSinSigno variables_caracter_sin_signo[MAX_VARS_CARACTER_SIN_SIGNO];

int num_texto_vars = 0;
VariableTexto texto_vars[MAX_VARS_TEXTO];

ScopeLocal scopes_locales[MAX_SCOPES];
int scope_actual = -1;
Constante constantes[MAX_CONSTANTES];
ConstanteSinSigno constantes_sin_signo[MAX_CONSTANTES_SIN_SIGNO];
ConstanteDecimal constantes_decimal[MAX_CONSTANTES_DECIMAL];
ConstanteDecimalSinSigno constantes_decimal_sin_signo[MAX_CONSTANTES_DECIMAL_SIN_SIGNO];
ConstanteCaracter constantes_caracter[MAX_CONSTANTES_CARACTER];
ConstanteCaracterSinSigno constantes_caracter_sin_signo[MAX_CONSTANTES_CARACTER_SIN_SIGNO];
ConstanteTexto texto_constantes[MAX_CONSTANTES_TEXTO];
int num_constantes = 0, num_constantes_sin_signo = 0, num_constantes_decimal = 0;
int num_constantes_decimal_sin_signo = 0, num_constantes_caracter = 0;
int num_constantes_caracter_sin_signo = 0, num_texto_constantes = 0;

ListaEntera listas_enteras[MAX_LISTAS_ENTERAS];
ListaDecimal listas_decimales[MAX_LISTAS_DECIMALES];
ListaEnteraSinSigno listas_enteras_sin_signo[MAX_LISTAS_ENTERAS_SIN_SIGNO];
ListaDecimalSinSigno listas_decimales_sin_signo[MAX_LISTAS_DECIMALES_SIN_SIGNO];
int num_listas_enteras = 0, num_listas_decimales = 0;
int num_listas_enteras_sin_signo = 0, num_listas_decimales_sin_signo = 0;

ListaCaracter listas_caracter[MAX_LISTAS_CARACTER];
ListaCaracterSinSigno listas_caracter_sin_signo[MAX_LISTAS_CARACTER_SIN_SIGNO];
int num_listas_caracter = 0;
int num_listas_caracter_sin_signo = 0;

MatrizEntera matrices_enteras[MAX_MATRICES_ENTERAS];
MatrizDecimal matrices_decimales[MAX_MATRICES_DECIMALES];
MatrizEnteraSinSigno matrices_enteras_sin_signo[MAX_MATRICES_ENTERAS_SIN_SIGNO];
MatrizDecimalSinSigno matrices_decimales_sin_signo[MAX_MATRICES_DECIMALES_SIN_SIGNO];
int num_matriz_enteras = 0, num_matriz_decimales = 0;
int num_matriz_enteras_sin_signo = 0, num_matriz_decimales_sin_signo = 0;

MatrizCaracter matrices_caracter[MAX_MATRICES_CARACTER];
MatrizCaracterSinSigno matrices_caracter_sin_signo[MAX_MATRICES_CARACTER_SIN_SIGNO];
int num_matriz_caracter = 0;
int num_matriz_caracter_sin_signo = 0;

// VARIABLES DE ARCHIVOS 
VariableArchivo variables_archivo[MAX_VARS_ARCHIVO];
int num_variables_archivo = 0;

int fase_constantes = 1, fase_variables = 0;

// REMOVER COMENTARIO - SOLO SOPORTA // EN NICO
void remover_comentario(char *linea) {
    if (!linea) return;
    
    int dentro_comillas = 0;
    char comilla_tipo = 0;
    
    for (int i = 0; linea[i] != '\0'; i++) {
        if (linea[i] == '"' || linea[i] == '\'') {
            if (i > 0 && linea[i-1] == '\\') {
                continue;
            }
            
            if (!dentro_comillas) {
                dentro_comillas = 1;
                comilla_tipo = linea[i];
            } else if (linea[i] == comilla_tipo) {
                dentro_comillas = 0;
                comilla_tipo = 0;
            }
            continue;
        }
        
        if (dentro_comillas) {
            continue;
        }
        
        if (linea[i] == '/' && linea[i+1] == '*') {
            fprintf(stderr, "\n");
            fprintf(stderr, "ERROR: Comentarios de bloque no permitidos.\n");
            exit(1);
        }
        
        if (linea[i] == '/' && linea[i+1] == '/') {
            linea[i] = '\0';
            return;
        }
    }
}

// FUNCIONES AUXILIARES 
int es_letra(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

int es_alnum(char c) {
    return es_letra(c) || (c >= '0' && c <= '9');
}

void limpiar_string(char *str) {
    if (!str) return;
    char *src = str, *dst = str;
    while (*src == ' ' || *src == '\t') src++;
    while (*src) *dst++ = *src++;
    *dst = '\0';
    size_t len = strlen(str);
    while (len > 0 && (str[len-1] == ' ' || str[len-1] == '\t' || str[len-1] == '\n' || str[len-1] == '\r')) {
        str[--len] = '\0';
    }
}

int comienza_con(const char *linea, const char *palabra) {
    if (!linea || !palabra) return 0;
    size_t len = strlen(palabra);
    if (strlen(linea) < len) return 0;
    if (strncmp(linea, palabra, len) != 0) return 0;
    return (linea[len] == ' ' || linea[len] == '\t' || linea[len] == '\0' || linea[len] == '(');
}

void limpiar_nombre(const char *nombre, char *dest, int dest_size) {
    if (!nombre || !dest) return;
    int i = 0, j = 0;
    while (nombre[i] && j < dest_size - 1) {
        if (nombre[i] == '$') { i++; continue; }
        if (es_alnum(nombre[i]) || nombre[i] == '_') {
            dest[j++] = nombre[i];
        }
        i++;
    }
    dest[j] = '\0';
}

int es_numero(const char *str) {
    if (!str || !*str) return 0;
    if (*str == '-' || *str == '+') str++;
    if (!*str) return 0;
    int tiene_punto = 0;
    while (*str) {
        if (*str == '.') {
            if (tiene_punto) return 0;
            tiene_punto = 1;
        } else if (*str < '0' || *str > '9') {
            return 0;
        }
        str++;
    }
    return 1;
}

void saltar_espacios_inplace(char *linea) {
    if (!linea) return;
    char *src = linea, *dst = linea;
    while (*src == ' ' || *src == '\t') src++;
    while (*src) *dst++ = *src++;
    *dst = '\0';
}

// FUNCIONES DE VARIABLES
int agregar_variable(const char *nombre, int valor) {
    if (num_variables >= MAX_VARS) return -1;
    strncpy(variables[num_variables].nombre, nombre, MAX_NOMBRE - 1);
    variables[num_variables].nombre[MAX_NOMBRE - 1] = '\0';
    variables[num_variables].valor = valor;
    num_variables++;
    return num_variables - 1;
}

int agregar_variable_sin_signo(const char *nombre, unsigned int valor) {
    if (num_variables_sin_signo >= MAX_VARS_SIN_SIGNO) return -1;
    strncpy(variables_sin_signo[num_variables_sin_signo].nombre, nombre, MAX_NOMBRE - 1);
    variables_sin_signo[num_variables_sin_signo].nombre[MAX_NOMBRE - 1] = '\0';
    variables_sin_signo[num_variables_sin_signo].valor = valor;
    num_variables_sin_signo++;
    return num_variables_sin_signo - 1;
}

int agregar_variable_decimal(const char *nombre, double valor) {
    if (num_variables_decimal >= MAX_VARS_DECIMAL) return -1;
    strncpy(variables_decimal[num_variables_decimal].nombre, nombre, MAX_NOMBRE - 1);
    variables_decimal[num_variables_decimal].nombre[MAX_NOMBRE - 1] = '\0';
    variables_decimal[num_variables_decimal].valor = valor;
    num_variables_decimal++;
    return num_variables_decimal - 1;
}

int agregar_variable_decimal_sin_signo(const char *nombre, double valor) {
    if (num_variables_decimal_sin_signo >= MAX_VARS_DECIMAL_SIN_SIGNO) return -1;
    strncpy(variables_decimal_sin_signo[num_variables_decimal_sin_signo].nombre, nombre, MAX_NOMBRE - 1);
    variables_decimal_sin_signo[num_variables_decimal_sin_signo].nombre[MAX_NOMBRE - 1] = '\0';
    variables_decimal_sin_signo[num_variables_decimal_sin_signo].valor = valor;
    num_variables_decimal_sin_signo++;
    return num_variables_decimal_sin_signo - 1;
}

int agregar_variable_caracter(const char *nombre, char valor) {
    if (num_variables_caracter >= MAX_VARS_CARACTER) return -1;
    strncpy(variables_caracter[num_variables_caracter].nombre, nombre, MAX_NOMBRE - 1);
    variables_caracter[num_variables_caracter].nombre[MAX_NOMBRE - 1] = '\0';
    variables_caracter[num_variables_caracter].valor = valor;
    num_variables_caracter++;
    return num_variables_caracter - 1;
}

int agregar_variable_caracter_sin_signo(const char *nombre, unsigned char valor) {
    if (num_variables_caracter_sin_signo >= MAX_VARS_CARACTER_SIN_SIGNO) return -1;
    strncpy(variables_caracter_sin_signo[num_variables_caracter_sin_signo].nombre, nombre, MAX_NOMBRE - 1);
    variables_caracter_sin_signo[num_variables_caracter_sin_signo].nombre[MAX_NOMBRE - 1] = '\0';
    variables_caracter_sin_signo[num_variables_caracter_sin_signo].valor = valor;
    num_variables_caracter_sin_signo++;
    return num_variables_caracter_sin_signo - 1;
}

int agregar_texto_var(const char *nombre, const char *valor) {
    if (num_texto_vars >= MAX_VARS_TEXTO) return -1;
    
    int idx = num_texto_vars;
    strncpy(texto_vars[idx].nombre, nombre ? nombre : "", MAX_NOMBRE - 1);
    texto_vars[idx].nombre[MAX_NOMBRE - 1] = '\0';
    
    strncpy(texto_vars[idx].valor, valor ? valor : "", MAX_TEXTO_LEN - 1);
    texto_vars[idx].valor[MAX_TEXTO_LEN - 1] = '\0';
    
    num_texto_vars++;
    return idx;
}

int buscar_variable(const char *nombre) {
    for (int i = 0; i < num_variables; i++) {
        if (strcmp(variables[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_variable_sin_signo(const char *nombre) {
    for (int i = 0; i < num_variables_sin_signo; i++) {
        if (strcmp(variables_sin_signo[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_variable_decimal(const char *nombre) {
    for (int i = 0; i < num_variables_decimal; i++) {
        if (strcmp(variables_decimal[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_variable_decimal_sin_signo(const char *nombre) {
    for (int i = 0; i < num_variables_decimal_sin_signo; i++) {
        if (strcmp(variables_decimal_sin_signo[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_variable_caracter(const char *nombre) {
    for (int i = 0; i < num_variables_caracter; i++) {
        if (strcmp(variables_caracter[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_variable_caracter_sin_signo(const char *nombre) {
    for (int i = 0; i < num_variables_caracter_sin_signo; i++) {
        if (strcmp(variables_caracter_sin_signo[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_texto_var(const char *nombre) {
    for (int i = 0; i < num_texto_vars; i++) {
        if (strcmp(texto_vars[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int get_var_valor_global(const char *nombre) {
    int idx = buscar_variable(nombre);
    if (idx >= 0) return variables[idx].valor;
    idx = buscar_variable_decimal(nombre);
    if (idx >= 0) return (int)variables_decimal[idx].valor;
    return 0;
}

void set_var_valor_global(const char *nombre, int valor) {
    int idx = buscar_variable(nombre);
    if (idx >= 0) {
        variables[idx].valor = valor;
        return;
    }
    idx = buscar_variable_decimal(nombre);
    if (idx >= 0) {
        variables_decimal[idx].valor = (double)valor;
        return;
    }
    agregar_variable(nombre, valor);
}

// FUNCIONES DE CONSTANTES
int agregar_constante(const char *nombre, int valor) {
    if (num_constantes >= MAX_CONSTANTES) return -1;
    strncpy(constantes[num_constantes].nombre, nombre, MAX_NOMBRE - 1);
    constantes[num_constantes].nombre[MAX_NOMBRE - 1] = '\0';
    constantes[num_constantes].valor = valor;
    num_constantes++;
    return num_constantes - 1;
}

int agregar_constante_sin_signo(const char *nombre, unsigned int valor) {
    if (num_constantes_sin_signo >= MAX_CONSTANTES_SIN_SIGNO) return -1;
    strncpy(constantes_sin_signo[num_constantes_sin_signo].nombre, nombre, MAX_NOMBRE - 1);
    constantes_sin_signo[num_constantes_sin_signo].nombre[MAX_NOMBRE - 1] = '\0';
    constantes_sin_signo[num_constantes_sin_signo].valor = valor;
    num_constantes_sin_signo++;
    return num_constantes_sin_signo - 1;
}

int agregar_constante_decimal(const char *nombre, double valor) {
    if (num_constantes_decimal >= MAX_CONSTANTES_DECIMAL) return -1;
    strncpy(constantes_decimal[num_constantes_decimal].nombre, nombre, MAX_NOMBRE - 1);
    constantes_decimal[num_constantes_decimal].nombre[MAX_NOMBRE - 1] = '\0';
    constantes_decimal[num_constantes_decimal].valor = valor;
    num_constantes_decimal++;
    return num_constantes_decimal - 1;
}

int agregar_constante_decimal_sin_signo(const char *nombre, double valor) {
    if (num_constantes_decimal_sin_signo >= MAX_CONSTANTES_DECIMAL_SIN_SIGNO) return -1;
    strncpy(constantes_decimal_sin_signo[num_constantes_decimal_sin_signo].nombre, nombre, MAX_NOMBRE - 1);
    constantes_decimal_sin_signo[num_constantes_decimal_sin_signo].nombre[MAX_NOMBRE - 1] = '\0';
    constantes_decimal_sin_signo[num_constantes_decimal_sin_signo].valor = valor;
    num_constantes_decimal_sin_signo++;
    return num_constantes_decimal_sin_signo - 1;
}

int agregar_constante_caracter(const char *nombre, char valor) {
    if (num_constantes_caracter >= MAX_CONSTANTES_CARACTER) return -1;
    strncpy(constantes_caracter[num_constantes_caracter].nombre, nombre, MAX_NOMBRE - 1);
    constantes_caracter[num_constantes_caracter].nombre[MAX_NOMBRE - 1] = '\0';
    constantes_caracter[num_constantes_caracter].valor = valor;
    num_constantes_caracter++;
    return num_constantes_caracter - 1;
}

int agregar_constante_caracter_sin_signo(const char *nombre, const char *valor) {
    if (num_constantes_caracter_sin_signo >= MAX_CONSTANTES_CARACTER_SIN_SIGNO) return -1;
    strncpy(constantes_caracter_sin_signo[num_constantes_caracter_sin_signo].nombre, nombre, MAX_NOMBRE - 1);
    constantes_caracter_sin_signo[num_constantes_caracter_sin_signo].nombre[MAX_NOMBRE - 1] = '\0';
    snprintf(constantes_caracter_sin_signo[num_constantes_caracter_sin_signo].valor, 8, "%s", valor ? valor : "");
    num_constantes_caracter_sin_signo++;
    return num_constantes_caracter_sin_signo - 1;
}

int agregar_texto_constante(const char *nombre, const char *valor) {
    if (num_texto_constantes >= MAX_CONSTANTES_TEXTO) return -1;
    strncpy(texto_constantes[num_texto_constantes].nombre, nombre, MAX_NOMBRE - 1);
    texto_constantes[num_texto_constantes].nombre[MAX_NOMBRE - 1] = '\0';
    strncpy(texto_constantes[num_texto_constantes].valor, valor, MAX_TEXTO_LEN - 1);
    texto_constantes[num_texto_constantes].valor[MAX_TEXTO_LEN - 1] = '\0';
    num_texto_constantes++;
    return num_texto_constantes - 1;
}

int buscar_constante(const char *nombre) {
    for (int i = 0; i < num_constantes; i++) {
        if (strcmp(constantes[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_constante_sin_signo(const char *nombre) {
    for (int i = 0; i < num_constantes_sin_signo; i++) {
        if (strcmp(constantes_sin_signo[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_constante_decimal(const char *nombre) {
    for (int i = 0; i < num_constantes_decimal; i++) {
        if (strcmp(constantes_decimal[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_constante_decimal_sin_signo(const char *nombre) {
    for (int i = 0; i < num_constantes_decimal_sin_signo; i++) {
        if (strcmp(constantes_decimal_sin_signo[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_constante_caracter(const char *nombre) {
    for (int i = 0; i < num_constantes_caracter; i++) {
        if (strcmp(constantes_caracter[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_constante_caracter_sin_signo(const char *nombre) {
    for (int i = 0; i < num_constantes_caracter_sin_signo; i++) {
        if (strcmp(constantes_caracter_sin_signo[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_texto_constante(const char *nombre) {
    for (int i = 0; i < num_texto_constantes; i++) {
        if (strcmp(texto_constantes[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

// FUNCIONES DE LISTAS
int agregar_lista_entera(const char *nombre, int longitud) {
    if (num_listas_enteras >= MAX_LISTAS_ENTERAS) return -1;
    strncpy(listas_enteras[num_listas_enteras].nombre, nombre, MAX_NOMBRE - 1);
    listas_enteras[num_listas_enteras].nombre[MAX_NOMBRE - 1] = '\0';
    listas_enteras[num_listas_enteras].longitud = longitud;
    listas_enteras[num_listas_enteras].esta_declarada = 1;
    for (int i = 0; i < longitud; i++) listas_enteras[num_listas_enteras].valores[i] = 0;
    num_listas_enteras++;
    return num_listas_enteras - 1;
}

int agregar_lista_decimal(const char *nombre, int longitud) {
    if (num_listas_decimales >= MAX_LISTAS_DECIMALES) return -1;
    strncpy(listas_decimales[num_listas_decimales].nombre, nombre, MAX_NOMBRE - 1);
    listas_decimales[num_listas_decimales].nombre[MAX_NOMBRE - 1] = '\0';
    listas_decimales[num_listas_decimales].longitud = longitud;
    listas_decimales[num_listas_decimales].esta_declarada = 1;
    for (int i = 0; i < longitud; i++) listas_decimales[num_listas_decimales].valores[i] = 0.0;
    num_listas_decimales++;
    return num_listas_decimales - 1;
}

int agregar_lista_entera_sin_signo(const char *nombre, int longitud) {
    if (num_listas_enteras_sin_signo >= MAX_LISTAS_ENTERAS_SIN_SIGNO) return -1;
    strncpy(listas_enteras_sin_signo[num_listas_enteras_sin_signo].nombre, nombre, MAX_NOMBRE - 1);
    listas_enteras_sin_signo[num_listas_enteras_sin_signo].nombre[MAX_NOMBRE - 1] = '\0';
    listas_enteras_sin_signo[num_listas_enteras_sin_signo].longitud = longitud;
    listas_enteras_sin_signo[num_listas_enteras_sin_signo].esta_declarada = 1;
    for (int i = 0; i < longitud; i++) listas_enteras_sin_signo[num_listas_enteras_sin_signo].valores[i] = 0;
    num_listas_enteras_sin_signo++;
    return num_listas_enteras_sin_signo - 1;
}

int agregar_lista_decimal_sin_signo(const char *nombre, int longitud) {
    if (num_listas_decimales_sin_signo >= MAX_LISTAS_DECIMALES_SIN_SIGNO) return -1;
    strncpy(listas_decimales_sin_signo[num_listas_decimales_sin_signo].nombre, nombre, MAX_NOMBRE - 1);
    listas_decimales_sin_signo[num_listas_decimales_sin_signo].nombre[MAX_NOMBRE - 1] = '\0';
    listas_decimales_sin_signo[num_listas_decimales_sin_signo].longitud = longitud;
    listas_decimales_sin_signo[num_listas_decimales_sin_signo].esta_declarada = 1;
    for (int i = 0; i < longitud; i++) listas_decimales_sin_signo[num_listas_decimales_sin_signo].valores[i] = 0.0;
    num_listas_decimales_sin_signo++;
    return num_listas_decimales_sin_signo - 1;
}

int buscar_lista_entera(const char *nombre) {
    for (int i = 0; i < num_listas_enteras; i++) {
        if (strcmp(listas_enteras[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_lista_decimal(const char *nombre) {
    for (int i = 0; i < num_listas_decimales; i++) {
        if (strcmp(listas_decimales[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_lista_entera_sin_signo(const char *nombre) {
    for (int i = 0; i < num_listas_enteras_sin_signo; i++) {
        if (strcmp(listas_enteras_sin_signo[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_lista_decimal_sin_signo(const char *nombre) {
    for (int i = 0; i < num_listas_decimales_sin_signo; i++) {
        if (strcmp(listas_decimales_sin_signo[i].nombre, nombre) == 0) return i;
    }
    return -1;
}   

int get_lista_entera_valor(const char *nombre, int indice) {
    int idx = buscar_lista_entera(nombre);
    if (idx >= 0 && indice >= 0 && indice < listas_enteras[idx].longitud) {
        return listas_enteras[idx].valores[indice];
    }
    return 0;
}

double get_lista_decimal_valor(const char *nombre, int indice) {
    int idx = buscar_lista_decimal(nombre);
    if (idx >= 0 && indice >= 0 && indice < listas_decimales[idx].longitud) {
        return listas_decimales[idx].valores[indice];
    }
    return 0.0;
}

unsigned int get_lista_entera_sin_signo_valor(const char *nombre, int indice) {
    int idx = buscar_lista_entera_sin_signo(nombre);
    if (idx >= 0 && indice >= 0 && indice < listas_enteras_sin_signo[idx].longitud) {
        return listas_enteras_sin_signo[idx].valores[indice];
    }
    return 0;
}

double get_lista_decimal_sin_signo_valor(const char *nombre, int indice) {
    int idx = buscar_lista_decimal_sin_signo(nombre);
    if (idx >= 0 && indice >= 0 && indice < listas_decimales_sin_signo[idx].longitud) {
        return listas_decimales_sin_signo[idx].valores[indice];
    }
    return 0.0;
}

void set_lista_entera_valor(const char *nombre, int indice, int valor) {
    const char *clean = (nombre[0] == '$') ? nombre + 1 : nombre;
    int tipo, indice_pool;
    if (buscar_lista_local(clean, &tipo, &indice_pool)) {
        if (tipo == TIPO_LISTA_ENTERA && indice >= 0 && indice < listas_enteras[indice_pool].longitud) {
            listas_enteras[indice_pool].valores[indice] = valor;
            return;
        }
    }
    int idx = buscar_lista_entera(clean);
    if (idx >= 0 && indice >= 0 && indice < listas_enteras[idx].longitud) {
        listas_enteras[idx].valores[indice] = valor;
    }
}

void set_lista_decimal_valor(const char *nombre, int indice, double valor) {
    const char *clean = (nombre[0] == '$') ? nombre + 1 : nombre;
    int tipo, indice_pool;
    if (buscar_lista_local(clean, &tipo, &indice_pool)) {
        if (tipo == TIPO_LISTA_DECIMAL && indice >= 0 && indice < listas_decimales[indice_pool].longitud) {
            listas_decimales[indice_pool].valores[indice] = valor;
            return;
        }
    }
    int idx = buscar_lista_decimal(clean);
    if (idx >= 0 && indice >= 0 && indice < listas_decimales[idx].longitud) {
        listas_decimales[idx].valores[indice] = valor;
    }
}

void set_lista_entera_sin_signo_valor(const char *nombre, int indice, unsigned int valor) {
    const char *clean = (nombre[0] == '$') ? nombre + 1 : nombre;
    int tipo, indice_pool;
    if (buscar_lista_local(clean, &tipo, &indice_pool)) {
        if (tipo == TIPO_LISTA_ENTERA_SIN_SIGNO && indice >= 0 && indice < listas_enteras_sin_signo[indice_pool].longitud) {
            listas_enteras_sin_signo[indice_pool].valores[indice] = valor;
            return;
        }
    }
    int idx = buscar_lista_entera_sin_signo(clean);
    if (idx >= 0 && indice >= 0 && indice < listas_enteras_sin_signo[idx].longitud) {
        listas_enteras_sin_signo[idx].valores[indice] = valor;
    }
}

void set_lista_decimal_sin_signo_valor(const char *nombre, int indice, double valor) {
    const char *clean = (nombre[0] == '$') ? nombre + 1 : nombre;
    int tipo, indice_pool;
    if (buscar_lista_local(clean, &tipo, &indice_pool)) {
        if (tipo == TIPO_LISTA_DECIMAL_SIN_SIGNO && indice >= 0 && indice < listas_decimales_sin_signo[indice_pool].longitud) {
            listas_decimales_sin_signo[indice_pool].valores[indice] = valor;
            return;
        }
    }
    int idx = buscar_lista_decimal_sin_signo(clean);
    if (idx >= 0 && indice >= 0 && indice < listas_decimales_sin_signo[idx].longitud) {
        listas_decimales_sin_signo[idx].valores[indice] = valor;
    }
}

void set_lista_caracter_valor(const char *nombre, int indice, char valor) {
    const char *clean = (nombre[0] == '$') ? nombre + 1 : nombre;
    int tipo, indice_pool;
    if (buscar_lista_local(clean, &tipo, &indice_pool)) {
        if (tipo == TIPO_LISTA_CARACTER && indice >= 0 && indice < listas_caracter[indice_pool].longitud) {
            listas_caracter[indice_pool].valores[indice] = valor;
            return;
        }
    }
    int idx = buscar_lista_caracter(clean);
    if (idx >= 0 && indice >= 0 && indice < listas_caracter[idx].longitud) {
        listas_caracter[idx].valores[indice] = valor;
    }
}

void set_lista_caracter_sin_signo_valor(const char *nombre, int indice, unsigned char valor) {
    const char *clean = (nombre[0] == '$') ? nombre + 1 : nombre;
    int tipo, indice_pool;
    if (buscar_lista_local(clean, &tipo, &indice_pool)) {
        if (tipo == TIPO_LISTA_CARACTER_SIN_SIGNO && indice >= 0 && indice < listas_caracter_sin_signo[indice_pool].longitud) {
            listas_caracter_sin_signo[indice_pool].valores[indice] = valor;
            return;
        }
    }
    int idx = buscar_lista_caracter_sin_signo(clean);
    if (idx >= 0 && indice >= 0 && indice < listas_caracter_sin_signo[idx].longitud) {
        listas_caracter_sin_signo[idx].valores[indice] = valor;
    }
}

// FUNCIONES DE MATRICES
int agregar_matriz_entera(const char *nombre, int filas, int columnas) {
    if (num_matriz_enteras >= MAX_MATRICES_ENTERAS) return -1;
    strncpy(matrices_enteras[num_matriz_enteras].nombre, nombre, MAX_NOMBRE - 1);
    matrices_enteras[num_matriz_enteras].nombre[MAX_NOMBRE - 1] = '\0';
    matrices_enteras[num_matriz_enteras].filas = filas;
    matrices_enteras[num_matriz_enteras].columnas = columnas;
    matrices_enteras[num_matriz_enteras].esta_declarada = 1;
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            matrices_enteras[num_matriz_enteras].valores[i][j] = 0;
        }
    }
    num_matriz_enteras++;
    return num_matriz_enteras - 1;
}

int agregar_matriz_decimal(const char *nombre, int filas, int columnas) {
    if (num_matriz_decimales >= MAX_MATRICES_DECIMALES) return -1;
    strncpy(matrices_decimales[num_matriz_decimales].nombre, nombre, MAX_NOMBRE - 1);
    matrices_decimales[num_matriz_decimales].nombre[MAX_NOMBRE - 1] = '\0';
    matrices_decimales[num_matriz_decimales].filas = filas;
    matrices_decimales[num_matriz_decimales].columnas = columnas;
    matrices_decimales[num_matriz_decimales].esta_declarada = 1;
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            matrices_decimales[num_matriz_decimales].valores[i][j] = 0.0;
        }
    }
    num_matriz_decimales++;
    return num_matriz_decimales - 1;
}

int agregar_matriz_entera_sin_signo(const char *nombre, int filas, int columnas) {
    if (num_matriz_enteras_sin_signo >= MAX_MATRICES_ENTERAS_SIN_SIGNO) return -1;
    strncpy(matrices_enteras_sin_signo[num_matriz_enteras_sin_signo].nombre, nombre, MAX_NOMBRE - 1);
    matrices_enteras_sin_signo[num_matriz_enteras_sin_signo].nombre[MAX_NOMBRE - 1] = '\0';
    matrices_enteras_sin_signo[num_matriz_enteras_sin_signo].filas = filas;
    matrices_enteras_sin_signo[num_matriz_enteras_sin_signo].columnas = columnas;
    matrices_enteras_sin_signo[num_matriz_enteras_sin_signo].esta_declarada = 1;
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            matrices_enteras_sin_signo[num_matriz_enteras_sin_signo].valores[i][j] = 0;
        }
    }
    num_matriz_enteras_sin_signo++;
    return num_matriz_enteras_sin_signo - 1;
}

int agregar_matriz_decimal_sin_signo(const char *nombre, int filas, int columnas) {
    if (num_matriz_decimales_sin_signo >= MAX_MATRICES_DECIMALES_SIN_SIGNO) return -1;
    strncpy(matrices_decimales_sin_signo[num_matriz_decimales_sin_signo].nombre, nombre, MAX_NOMBRE - 1);
    matrices_decimales_sin_signo[num_matriz_decimales_sin_signo].nombre[MAX_NOMBRE - 1] = '\0';
    matrices_decimales_sin_signo[num_matriz_decimales_sin_signo].filas = filas;
    matrices_decimales_sin_signo[num_matriz_decimales_sin_signo].columnas = columnas;
    matrices_decimales_sin_signo[num_matriz_decimales_sin_signo].esta_declarada = 1;
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            matrices_decimales_sin_signo[num_matriz_decimales_sin_signo].valores[i][j] = 0.0;
        }
    }
    num_matriz_decimales_sin_signo++;
    return num_matriz_decimales_sin_signo - 1;
}

int buscar_matriz_entera(const char *nombre) {
    for (int i = 0; i < num_matriz_enteras; i++) {
        if (strcmp(matrices_enteras[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_matriz_decimal(const char *nombre) {
    for (int i = 0; i < num_matriz_decimales; i++) {
        if (strcmp(matrices_decimales[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_matriz_entera_sin_signo(const char *nombre) {
    for (int i = 0; i < num_matriz_enteras_sin_signo; i++) {
        if (strcmp(matrices_enteras_sin_signo[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_matriz_decimal_sin_signo(const char *nombre) {
    for (int i = 0; i < num_matriz_decimales_sin_signo; i++) {
        if (strcmp(matrices_decimales_sin_signo[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int get_matriz_entera_valor(const char *nombre, int fila, int columna) {
    int idx = buscar_matriz_entera(nombre);
    if (idx >= 0 && fila >= 0 && fila < matrices_enteras[idx].filas && columna >= 0 && columna < matrices_enteras[idx].columnas) {
        return matrices_enteras[idx].valores[fila][columna];
    }
    return 0;
}

double get_matriz_decimal_valor(const char *nombre, int fila, int columna) {
    int idx = buscar_matriz_decimal(nombre);
    if (idx >= 0 && fila >= 0 && fila < matrices_decimales[idx].filas && columna >= 0 && columna < matrices_decimales[idx].columnas) {
        return matrices_decimales[idx].valores[fila][columna];
    }
    return 0.0;
}

unsigned int get_matriz_entera_sin_signo_valor(const char *nombre, int fila, int columna) {
    int idx = buscar_matriz_entera_sin_signo(nombre);
    if (idx >= 0 && fila >= 0 && fila < matrices_enteras_sin_signo[idx].filas && columna >= 0 && columna < matrices_enteras_sin_signo[idx].columnas) {
        return matrices_enteras_sin_signo[idx].valores[fila][columna];
    }
    return 0;
}

double get_matriz_decimal_sin_signo_valor(const char *nombre, int fila, int columna) {
    int idx = buscar_matriz_decimal_sin_signo(nombre);
    if (idx >= 0 && fila >= 0 && fila < matrices_decimales_sin_signo[idx].filas && columna >= 0 && columna < matrices_decimales_sin_signo[idx].columnas) {
        return matrices_decimales_sin_signo[idx].valores[fila][columna];
    }
    return 0.0;
}

void set_matriz_entera_valor(const char *nombre, int fila, int columna, int valor) {
    int idx = buscar_matriz_entera(nombre);
    if (idx >= 0 && fila >= 0 && fila < matrices_enteras[idx].filas && columna >= 0 && columna < matrices_enteras[idx].columnas) {
        matrices_enteras[idx].valores[fila][columna] = valor;
    }
}

void set_matriz_decimal_valor(const char *nombre, int fila, int columna, double valor) {
    int idx = buscar_matriz_decimal(nombre);
    if (idx >= 0 && fila >= 0 && fila < matrices_decimales[idx].filas && columna >= 0 && columna < matrices_decimales[idx].columnas) {
        matrices_decimales[idx].valores[fila][columna] = valor;
    }
}

void set_matriz_entera_sin_signo_valor(const char *nombre, int fila, int columna, unsigned int valor) {
    int idx = buscar_matriz_entera_sin_signo(nombre);
    if (idx >= 0 && fila >= 0 && fila < matrices_enteras_sin_signo[idx].filas && columna >= 0 && columna < matrices_enteras_sin_signo[idx].columnas) {
        matrices_enteras_sin_signo[idx].valores[fila][columna] = valor;
    }
}

void set_matriz_decimal_sin_signo_valor(const char *nombre, int fila, int columna, double valor) {
    int idx = buscar_matriz_decimal_sin_signo(nombre);
    if (idx >= 0 && fila >= 0 && fila < matrices_decimales_sin_signo[idx].filas && columna >= 0 && columna < matrices_decimales_sin_signo[idx].columnas) {
        matrices_decimales_sin_signo[idx].valores[fila][columna] = valor;
    }
}

// FUNCIONES DE LISTAS DE CARACTER
int agregar_lista_caracter(const char *nombre, int longitud) {
    if (num_listas_caracter >= MAX_LISTAS_CARACTER) return -1;
    strncpy(listas_caracter[num_listas_caracter].nombre, nombre, MAX_NOMBRE - 1);
    listas_caracter[num_listas_caracter].nombre[MAX_NOMBRE - 1] = '\0';
    listas_caracter[num_listas_caracter].longitud = longitud;
    listas_caracter[num_listas_caracter].esta_declarada = 1;
    for (int i = 0; i < longitud; i++) listas_caracter[num_listas_caracter].valores[i] = 0;
    num_listas_caracter++;
    return num_listas_caracter - 1;
}

int agregar_lista_caracter_sin_signo(const char *nombre, int longitud) {
    if (num_listas_caracter_sin_signo >= MAX_LISTAS_CARACTER_SIN_SIGNO) return -1;
    strncpy(listas_caracter_sin_signo[num_listas_caracter_sin_signo].nombre, nombre, MAX_NOMBRE - 1);
    listas_caracter_sin_signo[num_listas_caracter_sin_signo].nombre[MAX_NOMBRE - 1] = '\0';
    listas_caracter_sin_signo[num_listas_caracter_sin_signo].longitud = longitud;
    listas_caracter_sin_signo[num_listas_caracter_sin_signo].esta_declarada = 1;
    for (int i = 0; i < longitud; i++) listas_caracter_sin_signo[num_listas_caracter_sin_signo].valores[i] = 0;
    num_listas_caracter_sin_signo++;
    return num_listas_caracter_sin_signo - 1;
}

int buscar_lista_caracter(const char *nombre) {
    for (int i = 0; i < num_listas_caracter; i++) {
        if (strcmp(listas_caracter[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_lista_caracter_sin_signo(const char *nombre) {
    for (int i = 0; i < num_listas_caracter_sin_signo; i++) {
        if (strcmp(listas_caracter_sin_signo[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

char get_lista_caracter_valor(const char *nombre, int indice) {
    int idx = buscar_lista_caracter(nombre);
    if (idx >= 0 && indice >= 0 && indice < listas_caracter[idx].longitud) {
        return listas_caracter[idx].valores[indice];
    }
    return 0;
}

unsigned char get_lista_caracter_sin_signo_valor(const char *nombre, int indice) {
    int idx = buscar_lista_caracter_sin_signo(nombre);
    if (idx >= 0 && indice >= 0 && indice < listas_caracter_sin_signo[idx].longitud) {
        return listas_caracter_sin_signo[idx].valores[indice];
    }
    return 0;
}

// FUNCIONES DE MATRICES DE CARACTER
int agregar_matriz_caracter(const char *nombre, int filas, int columnas) {
    if (num_matriz_caracter >= MAX_MATRICES_CARACTER) return -1;
    strncpy(matrices_caracter[num_matriz_caracter].nombre, nombre, MAX_NOMBRE - 1);
    matrices_caracter[num_matriz_caracter].nombre[MAX_NOMBRE - 1] = '\0';
    matrices_caracter[num_matriz_caracter].filas = filas;
    matrices_caracter[num_matriz_caracter].columnas = columnas;
    matrices_caracter[num_matriz_caracter].esta_declarada = 1;
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            matrices_caracter[num_matriz_caracter].valores[i][j] = 0;
        }
    }
    num_matriz_caracter++;
    return num_matriz_caracter - 1;
}

int agregar_matriz_caracter_sin_signo(const char *nombre, int filas, int columnas) {
    if (num_matriz_caracter_sin_signo >= MAX_MATRICES_CARACTER_SIN_SIGNO) return -1;
    strncpy(matrices_caracter_sin_signo[num_matriz_caracter_sin_signo].nombre, nombre, MAX_NOMBRE - 1);
    matrices_caracter_sin_signo[num_matriz_caracter_sin_signo].nombre[MAX_NOMBRE - 1] = '\0';
    matrices_caracter_sin_signo[num_matriz_caracter_sin_signo].filas = filas;
    matrices_caracter_sin_signo[num_matriz_caracter_sin_signo].columnas = columnas;
    matrices_caracter_sin_signo[num_matriz_caracter_sin_signo].esta_declarada = 1;
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            matrices_caracter_sin_signo[num_matriz_caracter_sin_signo].valores[i][j] = 0;
        }
    }
    num_matriz_caracter_sin_signo++;
    return num_matriz_caracter_sin_signo - 1;
}

int buscar_matriz_caracter(const char *nombre) {
    for (int i = 0; i < num_matriz_caracter; i++) {
        if (strcmp(matrices_caracter[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int buscar_matriz_caracter_sin_signo(const char *nombre) {
    for (int i = 0; i < num_matriz_caracter_sin_signo; i++) {
        if (strcmp(matrices_caracter_sin_signo[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

char get_matriz_caracter_valor(const char *nombre, int fila, int columna) {
    int idx = buscar_matriz_caracter(nombre);
    if (idx >= 0 && fila >= 0 && fila < matrices_caracter[idx].filas && columna >= 0 && columna < matrices_caracter[idx].columnas) {
        return matrices_caracter[idx].valores[fila][columna];
    }
    return 0;
}

unsigned char get_matriz_caracter_sin_signo_valor(const char *nombre, int fila, int columna) {
    int idx = buscar_matriz_caracter_sin_signo(nombre);
    if (idx >= 0 && fila >= 0 && fila < matrices_caracter_sin_signo[idx].filas && columna >= 0 && columna < matrices_caracter_sin_signo[idx].columnas) {
        return matrices_caracter_sin_signo[idx].valores[fila][columna];
    }
    return 0;
}

void set_matriz_caracter_valor(const char *nombre, int fila, int columna, char valor) {
    int idx = buscar_matriz_caracter(nombre);
    if (idx >= 0 && fila >= 0 && fila < matrices_caracter[idx].filas && columna >= 0 && columna < matrices_caracter[idx].columnas) {
        matrices_caracter[idx].valores[fila][columna] = valor;
    }
}

void set_matriz_caracter_sin_signo_valor(const char *nombre, int fila, int columna, unsigned char valor) {
    int idx = buscar_matriz_caracter_sin_signo(nombre);
    if (idx >= 0 && fila >= 0 && fila < matrices_caracter_sin_signo[idx].filas && columna >= 0 && columna < matrices_caracter_sin_signo[idx].columnas) {
        matrices_caracter_sin_signo[idx].valores[fila][columna] = valor;
    }
}

// IMPLEMENTACIÓN MATRIZ TEXTO 
MatrizTexto matrices_texto[MAX_MATRICES] = {0};
int num_matrices_texto = 0;

int buscar_matriz_texto(const char *nombre) {
    for (int i = 0; i < num_matrices_texto; i++)
        if (strcmp(matrices_texto[i].nombre, nombre) == 0) return i;
    return -1;
}

int agregar_matriz_texto(const char *nombre, int filas, int columnas) {
    if (num_matrices_texto >= MAX_MATRICES) return -1;

    MatrizTexto *m = &matrices_texto[num_matrices_texto];
    strncpy(m->nombre, nombre, MAX_NOMBRE - 1);
    m->filas = filas;
    m->columnas = columnas;

    // Reservar memoria 3D segura
    m->valores = (char ***)calloc(filas, sizeof(char **));
    for (int f = 0; f < filas; f++) {
        m->valores[f] = (char **)calloc(columnas, sizeof(char *));
        for (int c = 0; c < columnas; c++) {
            m->valores[f][c] = (char *)calloc(MAX_TEXTO_LEN, sizeof(char));
            m->valores[f][c][0] = '\0'; // Inicializar vacía
        }
    }
    return num_matrices_texto++;
}

void set_matriz_texto_valor(const char *nombre, int f, int c, const char *valor) {
    int idx = buscar_matriz_texto_en_scope(nombre, scope_actual);
    if (idx >= 0 && f >= 0 && f < matrices_texto[idx].filas && 
        c >= 0 && c < matrices_texto[idx].columnas && valor) {
        strncpy(matrices_texto[idx].valores[f][c], valor, MAX_TEXTO_LEN - 1);
        matrices_texto[idx].valores[f][c][MAX_TEXTO_LEN - 1] = '\0';
    }
}

char* get_matriz_texto_valor(const char *nombre, int f, int c) {
    int idx = buscar_matriz_texto_en_scope(nombre, scope_actual);
    if (idx >= 0 && f >= 0 && f < matrices_texto[idx].filas &&
        c >= 0 && c < matrices_texto[idx].columnas) {
        return matrices_texto[idx].valores[f][c];
    }
    static char empty[] = "";
    return empty;
}

// FUNCIONES DE ARCHIVOS
int buscar_variable_archivo(const char *nombre) {
    for (int i = 0; i < num_variables_archivo; i++) {
        if (strcmp(variables_archivo[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

int agregar_variable_archivo(const char *nombre, FILE *archivo, int modo) {
    if (num_variables_archivo >= MAX_VARS_ARCHIVO) return -1;
    strncpy(variables_archivo[num_variables_archivo].nombre, nombre, MAX_NOMBRE - 1);
    variables_archivo[num_variables_archivo].nombre[MAX_NOMBRE - 1] = '\0';
    variables_archivo[num_variables_archivo].archivo = archivo;
    variables_archivo[num_variables_archivo].modo = modo;
    num_variables_archivo++;
    return num_variables_archivo - 1;
}

void cerrar_todos_los_archivos(void) {
    for (int i = 0; i < num_variables_archivo; i++) {
        if (variables_archivo[i].archivo) {
            fclose(variables_archivo[i].archivo);
            variables_archivo[i].archivo = NULL;
        }
    }
    num_variables_archivo = 0;
}

// GESTIÓN DE SCOPES LOCALES (Variables, Listas, Matrices)
int crear_scope_local(int funcion_idx) {
    if (!en_funcion) return -1;
    if (scope_actual + 1 >= MAX_SCOPES) return -1;
    scope_actual++;
    memset(&scopes_locales[scope_actual], 0, sizeof(ScopeLocal));
    scopes_locales[scope_actual].funcion_idx = funcion_idx;
    return scope_actual;
}

void eliminar_scope_local(void) {
    if (scope_actual >= 0 && scope_actual < MAX_SCOPES) {
        memset(&scopes_locales[scope_actual], 0, sizeof(ScopeLocal));
        scope_actual--;
    }
    if (scope_actual < -1) scope_actual = -1;
}

int agregar_variable_local(const char *nombre, int tipo, double valor) {
    if (scope_actual < 0) return -1;
    ScopeLocal *scp = &scopes_locales[scope_actual];
    if (scp->num_variables >= MAX_VARS_LOCALES) return -1;
    int idx = scp->num_variables;
    strncpy(scp->variables[idx].nombre, nombre, MAX_NOMBRE - 1);
    scp->variables[idx].nombre[MAX_NOMBRE - 1] = '\0';
    scp->variables[idx].tipo = tipo;
    switch(tipo) {
        case 0: scp->variables[idx].valor.valor_entero = (int)valor; break;
        case 1: scp->variables[idx].valor.valor_sin_signo = (unsigned int)valor; break;
        case 2: case 3: scp->variables[idx].valor.valor_decimal = valor; break;
        case 4: scp->variables[idx].valor.valor_caracter = (char)valor; break;
        case 5: scp->variables[idx].valor.valor_caracter_sin_signo = (unsigned char)valor; break;
    }
    scp->num_variables++;
    return idx;
}

int buscar_variable_local(const char *nombre, int *tipo, double *valor) {
    // Buscar desde el scope más interno hacia el externo
    for (int s = scope_actual; s >= 0; s--) {
        ScopeLocal *scp = &scopes_locales[s];
        
        // Iterar en orden INVERSO para dar prioridad a variables 
        // declaradas localmente sobre los parámetros de la función.
        for (int i = scp->num_variables - 1; i >= 0; i--) {
            if (strcmp(scp->variables[i].nombre, nombre) == 0) {
                if (tipo) *tipo = scp->variables[i].tipo;
                if (valor) {
                    switch(scp->variables[i].tipo) {
                        case 0: *valor = (double)scp->variables[i].valor.valor_entero; break;
                        case 1: *valor = (double)scp->variables[i].valor.valor_sin_signo; break;
                        case 2: case 3: *valor = scp->variables[i].valor.valor_decimal; break;
                        case 4: *valor = (double)scp->variables[i].valor.valor_caracter; break;
                        case 5: *valor = (double)scp->variables[i].valor.valor_caracter_sin_signo; break;
                        default: *valor = 0.0;
                    }
                }
                return 1;
            }
        }
    }
    return 0;
}

int agregar_texto_local(const char *nombre, const char *valor) {
    int idx_global = agregar_texto_var(nombre, valor);
    if (idx_global < 0) return -1;

    if (scope_actual >= 0) {
        ScopeLocal *scp = &scopes_locales[scope_actual];
        if (scp->num_textos >= MAX_TEXTOS_LOCALES) return -1;
        
        int idx_local = scp->num_textos;
        strncpy(scp->nombres_textos[idx_local], nombre, MAX_NOMBRE - 1);
        scp->nombres_textos[idx_local][MAX_NOMBRE - 1] = '\0';
        scp->indices_textos[idx_local] = idx_global;
        scp->num_textos++;
    }
    return 0;
}

int buscar_texto_local(const char *nombre, char *buffer_out) {
    for (int s = scope_actual; s >= 0; s--) {
        ScopeLocal *scp = &scopes_locales[s];
        for (int i = 0; i < scp->num_textos; i++) {
            if (strcmp(scp->nombres_textos[i], nombre) == 0) {
                if (buffer_out) {
                    int pool_idx = scp->indices_textos[i];
                    if (pool_idx >= 0 && pool_idx < MAX_VARS_TEXTO) {
                        strncpy(buffer_out, texto_vars[pool_idx].valor, MAX_TEXTO_LEN - 1);
                        buffer_out[MAX_TEXTO_LEN - 1] = '\0';
                    }
                }
                return 1;
            }
        }
    }
    return 0;
}

int registrar_lista_local(const char *nombre, int tipo, int indice_pool, int capacidad) {
    if (scope_actual < 0) return -1;
    ScopeLocal *scp = &scopes_locales[scope_actual];
    if (scp->num_listas >= MAX_LISTAS_LOCALES) return -1;
    const char *clean = (nombre[0] == '$') ? nombre + 1 : nombre;
    strncpy(scp->nombres_listas[scp->num_listas], clean, MAX_NOMBRE - 1);
    scp->nombres_listas[scp->num_listas][MAX_NOMBRE - 1] = '\0';
    scp->tipos_listas[scp->num_listas] = tipo;
    scp->indices_listas[scp->num_listas] = indice_pool;
    scp->capacidades_listas[scp->num_listas] = capacidad;
    scp->num_listas++;
    return scp->num_listas - 1;
}

int buscar_lista_local(const char *nombre, int *tipo, int *indice_pool) {
    for (int s = scope_actual; s >= 0; s--) {
        ScopeLocal *scp = &scopes_locales[s];
        for (int i = 0; i < scp->num_listas; i++) {
            if (strcmp(scp->nombres_listas[i], nombre) == 0) {
                if (tipo) *tipo = scp->tipos_listas[i];
                if (indice_pool) *indice_pool = scp->indices_listas[i];
                return 1;
            }
        }
    }
    return 0;
}

int registrar_matriz_local(const char *nombre, int tipo, int indice_pool, int filas, int cols) {
    if (scope_actual < 0) return -1;
    ScopeLocal *scp = &scopes_locales[scope_actual];
    if (scp->num_matrices >= MAX_MATRICES_LOCALES) return -1;
    const char *clean = (nombre[0] == '$') ? nombre + 1 : nombre;
    strncpy(scp->nombres_matrices[scp->num_matrices], clean, MAX_NOMBRE - 1);
    scp->nombres_matrices[scp->num_matrices][MAX_NOMBRE - 1] = '\0';
    scp->tipos_matrices[scp->num_matrices] = tipo;
    scp->indices_matrices[scp->num_matrices] = indice_pool;
    scp->filas_matrices[scp->num_matrices] = filas;
    scp->cols_matrices[scp->num_matrices] = cols;
    scp->num_matrices++;
    return scp->num_matrices - 1;
}

int buscar_matriz_texto_en_scope(const char *nombre, int scope_id) {
    // Normalizar nombre: quitar $ si existe (coherente con registrar_matriz_local)
    const char *clean = (nombre[0] == '$') ? nombre + 1 : nombre;
    
    // 1. Buscar PRIMERO en scope local (shadowing intencional)
    if (scope_id >= 0) {
        ScopeLocal *scp = &scopes_locales[scope_id];
        for (int i = 0; i < scp->num_matrices; i++) {
            if (strcmp(scp->nombres_matrices[i], clean) == 0) {
                // Encontrada en scope local: retornar su índice del pool
                return scp->indices_matrices[i];
            }
        }
    }
    
    // 2. Fallback: buscar en pool global (variables no shadoweadas)
    for (int i = 0; i < num_matrices_texto; i++) {
        if (strcmp(matrices_texto[i].nombre, clean) == 0) {
            return i;
        }
    }
    
    return -1;  // No encontrada en ningún scope
}

int buscar_matriz_local(const char *nombre, int *tipo, int *indice_pool) {
    for (int s = scope_actual; s >= 0; s--) {
        ScopeLocal *scp = &scopes_locales[s];
        for (int i = 0; i < scp->num_matrices; i++) {
            if (strcmp(scp->nombres_matrices[i], nombre) == 0) {
                if (tipo) *tipo = scp->tipos_matrices[i];
                if (indice_pool) *indice_pool = scp->indices_matrices[i];
                return 1;
            }
        }
    }
    return 0;
}