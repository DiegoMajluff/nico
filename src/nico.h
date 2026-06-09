/*
 * Nico v1.1.1 - Intérprete Educativo de Scripting en Español
 * @file:         nico.h
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Definiciones globales, estructuras de datos, límites y 
 *                prototipos de funciones compartidas.
 */
#define _POSIX_C_SOURCE 200809L
#ifndef NICO_H
#define NICO_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <time.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>

// CONSTANTES
#define MAX_LINEA                       1024
#define MAX_LINEAS_PROGRAMA             5000
#define MAX_NOMBRE                      64
#define MAX_TEXTO_LEN                   4096
#define MAX_NESTING                     500
#define MAX_ETIQUETAS                   100
#define MAX_VARS                        100
#define MAX_VARS_SIN_SIGNO              100
#define MAX_VARS_DECIMAL                100
#define MAX_VARS_DECIMAL_SIN_SIGNO      100
#define MAX_VARS_CARACTER               100
#define MAX_VARS_CARACTER_SIN_SIGNO     100
#define MAX_VARS_TEXTO                  100 
#define MAX_CONSTANTES                  100
#define MAX_CONSTANTES_SIN_SIGNO        100
#define MAX_CONSTANTES_DECIMAL          100
#define MAX_CONSTANTES_DECIMAL_SIN_SIGNO 100
#define MAX_CONSTANTES_CARACTER         100
#define MAX_CONSTANTES_CARACTER_SIN_SIGNO 100
#define MAX_CONSTANTES_TEXTO            50
#define MAX_VARS_ARCHIVO                25
#define MAX_LISTA                       2000
#define MAX_LISTAS_ENTERAS              25
#define MAX_LISTAS_DECIMALES            25
#define MAX_LISTAS_ENTERAS_SIN_SIGNO    25
#define MAX_LISTAS_DECIMALES_SIN_SIGNO  25
#define MAX_PARAMETROS                  10

// MATRICES - CONSTANTES
#define MAX_MATRICES_ENTERAS              25
#define MAX_MATRICES_DECIMALES            25
#define MAX_MATRICES_ENTERAS_SIN_SIGNO    25
#define MAX_MATRICES_DECIMALES_SIN_SIGNO  25
#define MAX_DIMENSION_FILA                100
#define MAX_DIMENSION_COLUMNA             100
#define MAX_MATRICES                      64

// LISTAS Y MATRICES DE CARACTER
#define MAX_LISTAS_CARACTER 50
#define MAX_LISTAS_CARACTER_SIN_SIGNO 50
#define MAX_MATRICES_CARACTER 50
#define MAX_MATRICES_CARACTER_SIN_SIGNO 50

// LISTA TEXTO 
#define MAX_LISTAS_TEXTO 64
#define MAX_ITEMS_TEXTO MAX_LISTA 

typedef struct {
    char nombre[MAX_NOMBRE];
    char valores[MAX_ITEMS_TEXTO][MAX_TEXTO_LEN]; 
    int longitud;
    int esta_declarada;
} ListaTexto;

extern ListaTexto listas_texto[MAX_LISTAS_TEXTO];
extern int num_listas_texto;

#define MODO_ESCRITURA                  0
#define MODO_APPEND                     1
#define MODO_LECTURA                    2
#define MODO_LECTURA_ESCRITURA          3

extern ListaTexto listas_texto[MAX_LISTAS_TEXTO];
extern int num_listas_texto;

// ESTRUCTURAS DE DATOS
// Variables 
typedef struct { char nombre[MAX_NOMBRE]; int valor; } Variable;
typedef struct { char nombre[MAX_NOMBRE]; unsigned int valor; } VariableSinSigno;
typedef struct { char nombre[MAX_NOMBRE]; double valor; } VariableDecimal;
typedef struct { char nombre[MAX_NOMBRE]; double valor; } VariableDecimalSinSigno;
typedef struct { char nombre[MAX_NOMBRE]; char valor; } VariableCaracter;
typedef struct { char nombre[MAX_NOMBRE]; unsigned char valor; } VariableCaracterSinSigno;
typedef struct { char nombre[MAX_NOMBRE]; char valor[MAX_TEXTO_LEN]; } VariableTexto;

// Constantes
typedef struct { char nombre[MAX_NOMBRE]; int valor; } Constante;
typedef struct { char nombre[MAX_NOMBRE]; unsigned int valor; } ConstanteSinSigno;
typedef struct { char nombre[MAX_NOMBRE]; double valor; } ConstanteDecimal;
typedef struct { char nombre[MAX_NOMBRE]; double valor; } ConstanteDecimalSinSigno;
typedef struct { char nombre[MAX_NOMBRE]; char valor; } ConstanteCaracter;
typedef struct { char nombre[MAX_NOMBRE]; char valor[8]; } ConstanteCaracterSinSigno;
typedef struct { char nombre[MAX_NOMBRE]; char valor[MAX_TEXTO_LEN]; } ConstanteTexto;

// Archivos
typedef struct {
    char nombre[MAX_NOMBRE];
    FILE *archivo;
    int modo;
} VariableArchivo;

// Listas
typedef struct { char nombre[MAX_NOMBRE]; int valores[MAX_LISTA]; int longitud; int esta_declarada; } ListaEntera;
typedef struct { char nombre[MAX_NOMBRE]; double valores[MAX_LISTA]; int longitud; int esta_declarada; } ListaDecimal;
typedef struct { char nombre[MAX_NOMBRE]; unsigned int valores[MAX_LISTA]; int longitud; int esta_declarada; } ListaEnteraSinSigno;
typedef struct { char nombre[MAX_NOMBRE]; double valores[MAX_LISTA]; int longitud; int esta_declarada; } ListaDecimalSinSigno;

// Etiquetas
typedef struct { char nombre[MAX_NOMBRE]; int linea; } Etiqueta;

// Bloques de control de flujo
typedef struct { int linea_inicio; int linea_fin; int nivel; } MientrasBloque;
typedef struct { int linea_inicio; int linea_fin; int nivel; } ProcederBloque;
typedef struct { int linea_inicio; int linea_fin; } SiBloque;
typedef struct { int linea_inicio; int linea_fin; int nivel; char var_nombre[MAX_NOMBRE]; int fin; int paso; } ParaBloque;
typedef struct { int linea_inicio; int linea_fin; int caso_encontrado; int valor_variable; int linea_corte; } SegunBloque;
typedef struct { int linea_retorno; } SubBloque;
typedef struct { int linea_retorno; int tipo_retorno; } FuncionBloque;

// Información de subprogramas y funciones
typedef struct {
    char nombre[MAX_NOMBRE];
    int linea_inicio;
    int linea_fin;
    char params[MAX_PARAMETROS][MAX_NOMBRE];
    int num_params;
} SubprogramaInfo;

typedef struct {
    char nombre[MAX_NOMBRE];
    int linea_inicio;
    int linea_fin;
    char params[MAX_PARAMETROS][MAX_NOMBRE];
    int num_params;
    int tipo_retorno;
    int tipos_params[MAX_PARAMETROS];  
} FuncionInfo;

// MATRICES - ESTRUCTURAS
typedef struct {
    char nombre[MAX_NOMBRE];
    int valores[MAX_DIMENSION_FILA][MAX_DIMENSION_COLUMNA];
    int filas;
    int columnas;
    int esta_declarada;
} MatrizEntera;

typedef struct {
    char nombre[MAX_NOMBRE];
    double valores[MAX_DIMENSION_FILA][MAX_DIMENSION_COLUMNA];
    int filas;
    int columnas;
    int esta_declarada;
} MatrizDecimal;

typedef struct {
    char nombre[MAX_NOMBRE];
    unsigned int valores[MAX_DIMENSION_FILA][MAX_DIMENSION_COLUMNA];
    int filas;
    int columnas;
    int esta_declarada;
} MatrizEnteraSinSigno;

typedef struct {
    char nombre[MAX_NOMBRE];
    double valores[MAX_DIMENSION_FILA][MAX_DIMENSION_COLUMNA];
    int filas;
    int columnas;
    int esta_declarada;
} MatrizDecimalSinSigno;

// ESTRUCTURAS DE LISTAS DE CARACTER
typedef struct {
    char nombre[MAX_NOMBRE];
    char valores[MAX_LISTA];
    int longitud;
    int esta_declarada;
} ListaCaracter;

typedef struct {
    char nombre[MAX_NOMBRE];
    unsigned char valores[MAX_LISTA];
    int longitud;
    int esta_declarada;
} ListaCaracterSinSigno;

// ESTRUCTURAS DE MATRICES DE CARACTER
typedef struct {
    char nombre[MAX_NOMBRE];
    char valores[MAX_DIMENSION_FILA][MAX_DIMENSION_COLUMNA];
    int filas;
    int columnas;
    int esta_declarada;
} MatrizCaracter;

typedef struct {
    char nombre[MAX_NOMBRE];
    unsigned char valores[MAX_DIMENSION_FILA][MAX_DIMENSION_COLUMNA];
    int filas;
    int columnas;
    int esta_declarada;
} MatrizCaracterSinSigno;

// VARIABLES GLOBALES - EXTERN
extern char lineas_programa[MAX_LINEAS_PROGRAMA][MAX_LINEA];
extern int num_lineas_programa;

extern VariableArchivo variables_archivo[MAX_VARS_ARCHIVO];
extern int num_variables_archivo;

extern Etiqueta etiquetas[MAX_ETIQUETAS];
extern int num_etiquetas;

extern Variable variables[MAX_VARS];
extern VariableSinSigno variables_sin_signo[MAX_VARS_SIN_SIGNO];
extern VariableDecimal variables_decimal[MAX_VARS_DECIMAL];
extern VariableDecimalSinSigno variables_decimal_sin_signo[MAX_VARS_DECIMAL_SIN_SIGNO];
extern VariableCaracter variables_caracter[MAX_VARS_CARACTER];
extern VariableCaracterSinSigno variables_caracter_sin_signo[MAX_VARS_CARACTER_SIN_SIGNO];
extern VariableTexto texto_vars[MAX_VARS_TEXTO];
extern int num_variables, num_variables_sin_signo, num_variables_decimal, num_variables_decimal_sin_signo;
extern int num_variables_caracter, num_variables_caracter_sin_signo, num_texto_vars;
extern int error_fatal;
// Validación estructural compartida
void validar_estructura_bloques(int inicio, int fin);


// CONSTANTES PARA SCOPES LOCALES
#define MAX_TEXTOS_LOCALES  20  
#define MAX_LISTAS_LOCALES  10  
#define MAX_MATRICES_LOCALES 10  

// VARIABLES LOCALES POR SCOPE
typedef struct {
    char nombre[MAX_NOMBRE];
    int tipo;  // 0=entera, 1=sin_signo, 2=decimal, 3=decimal_sin_signo, 4=caracter, 5=caracter_sin_signo
    union {
        int valor_entero;
        unsigned int valor_sin_signo;
        double valor_decimal;
        char valor_caracter;
        unsigned char valor_caracter_sin_signo;
    } valor;
} VariableLocal;

#define MAX_VARS_LOCALES 256
#define MAX_SCOPES 10000

typedef struct {
    VariableLocal variables[MAX_VARS_LOCALES];
    int num_variables;
    // Soporte para TEXTO EXTENSO locales
    struct {
        char nombre[MAX_NOMBRE];
        char *valor;
        size_t longitud;
        size_t capacidad;
    } textos_ext[MAX_VARS_LOCALES];
    int num_textos_ext;

    char nombres_textos[MAX_TEXTOS_LOCALES][MAX_NOMBRE];
    int indices_textos[MAX_TEXTOS_LOCALES]; 
    int num_textos;

    char nombres_listas[MAX_LISTAS_LOCALES][MAX_NOMBRE];
    int tipos_listas[MAX_LISTAS_LOCALES]; 
    int indices_listas[MAX_LISTAS_LOCALES];
    int capacidades_listas[MAX_LISTAS_LOCALES];
    int num_listas;

    char nombres_matrices[MAX_MATRICES_LOCALES][MAX_NOMBRE];
    int tipos_matrices[MAX_MATRICES_LOCALES]; 
    int indices_matrices[MAX_MATRICES_LOCALES];
    int filas_matrices[MAX_MATRICES_LOCALES];
    int cols_matrices[MAX_MATRICES_LOCALES];
    int num_matrices;

    int funcion_idx;
} ScopeLocal;

// TEXTO EXTENSO: Gestión de strings dinámicos
#define MAX_TEXTOS_EXT_GLOBALES 32

typedef struct {
    char nombre[MAX_NOMBRE];
    char *valor;
    size_t longitud;
    size_t capacidad;
} TextoExtGlobal;

extern TextoExtGlobal textos_ext_globales[MAX_TEXTOS_EXT_GLOBALES];
extern int num_textos_ext_globales;

int crear_texto_extenso_global(const char *nombre);
int crear_texto_extenso_local(const char *nombre, int scope_idx);
int buscar_texto_extenso(const char *nombre, int *es_local, int *idx, int *scope_idx);
int asignar_texto_extenso_valor(int es_local, int idx, int scope_idx, const char *valor);
void liberar_textos_ext_de_scope(int scope_idx);
void liberar_todos_textos_extensos(void);

extern int scope_actual; 

extern Constante constantes[MAX_CONSTANTES];
extern ConstanteSinSigno constantes_sin_signo[MAX_CONSTANTES_SIN_SIGNO];
extern ConstanteDecimal constantes_decimal[MAX_CONSTANTES_DECIMAL];
extern ConstanteDecimalSinSigno constantes_decimal_sin_signo[MAX_CONSTANTES_DECIMAL_SIN_SIGNO];
extern ConstanteCaracter constantes_caracter[MAX_CONSTANTES_CARACTER];
extern ConstanteCaracterSinSigno constantes_caracter_sin_signo[MAX_CONSTANTES_CARACTER_SIN_SIGNO];
extern ConstanteTexto texto_constantes[MAX_CONSTANTES_TEXTO];
extern int num_constantes, num_constantes_sin_signo, num_constantes_decimal, num_constantes_decimal_sin_signo, num_constantes_caracter, num_constantes_caracter_sin_signo, num_texto_constantes;

extern ListaEntera listas_enteras[MAX_LISTAS_ENTERAS];
extern ListaDecimal listas_decimales[MAX_LISTAS_DECIMALES];
extern ListaEnteraSinSigno listas_enteras_sin_signo[MAX_LISTAS_ENTERAS_SIN_SIGNO];
extern ListaDecimalSinSigno listas_decimales_sin_signo[MAX_LISTAS_DECIMALES_SIN_SIGNO];
extern int num_listas_enteras, num_listas_decimales, num_listas_enteras_sin_signo, num_listas_decimales_sin_signo;

extern SubprogramaInfo subprogramas_registrados[MAX_NESTING];
extern int num_subprogramas_registrados;

extern FuncionInfo funciones_registradas[MAX_NESTING];
extern int num_funciones_registradas;

extern double valor_retorno_funcion;

// PILA DE VALORES DE RETORNO (para recursión anidada)
#define MAX_PROFUNDIDAD_FUNCION 1000
typedef struct {
    double valores[MAX_PROFUNDIDAD_FUNCION];
    int hay_retorno[MAX_PROFUNDIDAD_FUNCION];
    int tope;
} PilaRetorno;

extern PilaRetorno pila_retorno;

// Flags de estado
extern int fase_constantes, fase_variables;
extern int en_bloque_principal, en_subprograma, en_funcion;

// Variables para recursividad
extern char funcion_variable_destino[MAX_NOMBRE];
extern char funcion_destino_stack[MAX_NESTING][MAX_NOMBRE];
extern int funcion_destino_stack_ptr;
extern FuncionBloque funcion_stack[MAX_NESTING];
extern int funcion_stack_ptr;
extern int fase_declaraciones;

// Matrices
extern MatrizEntera matrices_enteras[MAX_MATRICES_ENTERAS];
extern MatrizDecimal matrices_decimales[MAX_MATRICES_DECIMALES];
extern MatrizEnteraSinSigno matrices_enteras_sin_signo[MAX_MATRICES_ENTERAS_SIN_SIGNO];
extern MatrizDecimalSinSigno matrices_decimales_sin_signo[MAX_MATRICES_DECIMALES_SIN_SIGNO];
extern int num_matriz_enteras, num_matriz_decimales, num_matriz_enteras_sin_signo, num_matriz_decimales_sin_signo;

extern int modo_estricto; 

// VARIABLES GLOBALES DE LISTAS Y MATRICES DE CARACTER
extern int num_listas_caracter;
extern int num_listas_caracter_sin_signo;
extern int num_matriz_caracter;
extern int num_matriz_caracter_sin_signo;

// Declarar los arrays de pools
extern ListaCaracter listas_caracter[MAX_LISTAS_CARACTER];
extern ListaCaracterSinSigno listas_caracter_sin_signo[MAX_LISTAS_CARACTER_SIN_SIGNO];
extern MatrizCaracter matrices_caracter[MAX_MATRICES_CARACTER];
extern MatrizCaracterSinSigno matrices_caracter_sin_signo[MAX_MATRICES_CARACTER_SIN_SIGNO];

// FUNCIONES AUXILIARES
int es_letra(char c);
int es_alnum(char c);
void limpiar_string(char *str);
int comienza_con(const char *linea, const char *palabra);
void limpiar_nombre(const char *nombre, char *dest, int dest_size);
void remover_comentario(char *linea);
void saltar_espacios_inplace(char *linea);
int es_numero(const char *str);
static inline const char* comienza_con_keyword(const char *linea, const char *keyword) {
    if (!linea || !keyword) return NULL;
    int kw_len = (int)strlen(keyword);
    if (strncmp(linea, keyword, kw_len) != 0) return NULL;
    const char *ptr = linea + kw_len;
    while (*ptr == ' ' || *ptr == '\t') ptr++; 
    return ptr;  
}

// FUNCIONES DE ETIQUETAS
int agregar_etiqueta(const char *nombre, int linea);
int buscar_etiqueta(const char *nombre);

// FUNCIONES DE VARIABLES
int agregar_variable(const char *nombre, int valor);
int agregar_variable_sin_signo(const char *nombre, unsigned int valor);
int agregar_variable_decimal(const char *nombre, double valor);
int agregar_variable_decimal_sin_signo(const char *nombre, double valor);
int agregar_variable_caracter(const char *nombre, char valor);
int agregar_variable_caracter_sin_signo(const char *nombre, unsigned char valor);
int agregar_texto_var(const char *nombre, const char *valor);

int buscar_variable(const char *nombre);
int buscar_variable_sin_signo(const char *nombre);
int buscar_variable_decimal(const char *nombre);
int buscar_variable_decimal_sin_signo(const char *nombre);
int buscar_variable_caracter(const char *nombre);
int buscar_variable_caracter_sin_signo(const char *nombre);
int buscar_texto_var(const char *nombre);

int get_var_valor_global(const char *nombre);
void set_var_valor_global(const char *nombre, int valor);

// FUNCIONES DE CONSTANTES 
int agregar_constante(const char *nombre, int valor);
int agregar_constante_sin_signo(const char *nombre, unsigned int valor);
int agregar_constante_decimal(const char *nombre, double valor);
int agregar_constante_decimal_sin_signo(const char *nombre, double valor);
int agregar_constante_caracter(const char *nombre, char valor);
int agregar_constante_caracter_sin_signo(const char *nombre, const char *valor);
int agregar_texto_constante(const char *nombre, const char *valor);

int buscar_constante(const char *nombre);
int buscar_constante_sin_signo(const char *nombre);
int buscar_constante_decimal(const char *nombre);
int buscar_constante_decimal_sin_signo(const char *nombre);
int buscar_constante_caracter(const char *nombre);
int buscar_constante_caracter_sin_signo(const char *nombre);
int buscar_texto_constante(const char *nombre);

// FUNCIONES DE ARCHIVOS
int buscar_variable_archivo(const char *nombre);
int agregar_variable_archivo(const char *nombre, FILE *archivo, int modo);
void cerrar_todos_los_archivos(void);

int procesar_usararchivo(const char *nombre_archivo, const char *modo_str, const char *nombre_variable);
void procesar_abrirarchivo(const char *argumento);
void procesar_escribirarchivo(const char *argumento);
void procesar_leerarchivo(const char *argumento);
void procesar_leerlinea(const char *argumento);
void procesar_cerrararchivo(const char *argumento);

// FUNCIONES DE LISTAS
int agregar_lista_entera(const char *nombre, int longitud);
int agregar_lista_decimal(const char *nombre, int longitud);
int agregar_lista_entera_sin_signo(const char *nombre, int longitud);
int agregar_lista_decimal_sin_signo(const char *nombre, int longitud);

int buscar_lista_entera(const char *nombre);
int buscar_lista_decimal(const char *nombre);
int buscar_lista_entera_sin_signo(const char *nombre);
int buscar_lista_decimal_sin_signo(const char *nombre);
int buscar_lista_texto(const char *nombre);

int get_lista_entera_valor(const char *nombre, int indice);
double get_lista_decimal_valor(const char *nombre, int indice);
unsigned int get_lista_entera_sin_signo_valor(const char *nombre, int indice);
double get_lista_decimal_sin_signo_valor(const char *nombre, int indice);

void set_lista_entera_valor(const char *nombre, int indice, int valor);
void set_lista_decimal_valor(const char *nombre, int indice, double valor);
void set_lista_entera_sin_signo_valor(const char *nombre, int indice, unsigned int valor);
void set_lista_decimal_sin_signo_valor(const char *nombre, int indice, double valor);

// FUNCIONES DE DECLARACIONES
int parsear_declaracion(const char *ptr, char *nombre, char *valor_texto, double *valor_double, int *hay_asignacion, int *es_texto);
int cargar_archivo_en_memoria(const char *nombre_archivo);

int procesar_declaracion_variable_archivo(const char *linea, int linea_actual);
int procesar_declaracion_constante_entera_sin_signo(const char *linea, int linea_actual);
int procesar_declaracion_constante_decimal_sin_signo(const char *linea, int linea_actual);
int procesar_declaracion_constante_texto(const char *linea, int linea_actual);
int procesar_declaracion_constante_entera(const char *linea, int linea_actual);
int procesar_declaracion_constante_decimal(const char *linea, int linea_actual);
int procesar_declaracion_constante_caracter(const char *linea, int linea_actual);
int procesar_declaracion_constante_caracter_sin_signo(const char *linea, int linea_actual);
int procesar_declaracion_variable_entera_sin_signo(const char *linea, int linea_actual);
int procesar_declaracion_variable_decimal_sin_signo(const char *linea, int linea_actual);
int procesar_declaracion_variable_caracter_sin_signo(const char *linea, int linea_actual);
int procesar_declaracion_variable_caracter(const char *linea, int linea_actual);
int procesar_declaracion_variable_entera(const char *linea, int linea_actual);
int procesar_declaracion_variable_decimal(const char *linea, int linea_actual);
int procesar_declaracion_variable_texto(const char *linea, int linea_actual);
int procesar_declaracion_lista_entera_sin_signo(const char *linea, int linea_actual);
int procesar_declaracion_lista_decimal_sin_signo(const char *linea, int linea_actual);
int procesar_declaracion_lista_entera(const char *linea, int linea_actual);
int procesar_declaracion_lista_decimal(const char *linea, int linea_actual);

// Funciones de matrices - declaraciones
int procesar_declaracion_matriz_entera(const char *linea, int linea_actual);
int procesar_declaracion_matriz_decimal(const char *linea, int linea_actual);
int procesar_declaracion_matriz_entera_sin_signo(const char *linea, int linea_actual);
int procesar_declaracion_matriz_decimal_sin_signo(const char *linea, int linea_actual);

// FUNCIONES DE SCOPES PARA LISTAS Y MATRICES
int buscar_lista_local(const char *nombre, int *tipo, int *indice_pool);
int registrar_lista_local(const char *nombre, int tipo, int indice_pool, int capacidad);
int buscar_matriz_local(const char *nombre, int *tipo, int *indice_pool);
int registrar_matriz_local(const char *nombre, int tipo, int indice_pool, int filas, int cols);

// FUNCIONES DE EXPRESIONES
double obtener_valor_token(const char *token, int *exito);
double evaluar_expresion_completa(const char *expr, int *exito);
int evaluar_condicion(const char *linea, int *exito);
int evaluar_condicion_simple(const char *condicion, int *exito);

// FUNCIONES MATEMÁTICAS NATIVAS
double nico_seno(double x);
double nico_coseno(double x);
double nico_tangente(double x);
double nico_raiz(double x);
double nico_potencia(double base, double exp);
double nico_logaritmo(double x);
double nico_absoluto(double x);
double nico_pi(void);
double nico_modulo(double a, double b);
double nico_raizcubica(double x);
double nico_arcoseno(double x);
double nico_arcocoseno(double x);
double nico_arcotangente(double x);
double nico_redondear_arriba(double a);
double nico_redondear_abajo(double a);
double nico_redondear_entero(double a);
double nico_quitar_decimal(double a);
double nico_maximo(double a, double b);
double nico_minimo(double a, double b);
double nico_lognatural(double x);
double nico_logbase10(double x);
double nico_logbase2(double x);
double nico_logaritmo_base(double numero, double base);
double nico_numeropi(void);
double nico_numeroeuler(void);
double nico_raizdeunmedio(void);
double nico_lognaturalde2(void);
double nico_lognaturalde10(void);
double nico_exponencial(double x);
double nico_dosalax(double x);
double nico_sigmoide(double x);
int obtener_longitud_lista(const char *nombre_lista);

// Funciones aleatorias
double nico_aleatorio_entero(int min, int max);
double nico_aleatorio_decimal(double min, double max);
double nico_aleatorio_caracter(char min, char max);
unsigned int nico_aleatorio_sin_signo(unsigned int min, unsigned int max);
void inicializar_semilla_aleatoria(void);

// FUNCIONES DE CADENAS - cadenas.c
double nico_longitud_texto(const char *texto);
int nico_copiar_texto(char *destino, const char *origen, int max_len);
int nico_concatenar_texto(char *destino, const char *origen, int max_len);
int nico_comparar_texto(const char *texto1, const char *texto2);
int nico_comparar_texto_ignorando(const char *texto1, const char *texto2);
int nico_buscar_caracter(const char *texto, char caracter);
int nico_buscar_texto(const char *texto, const char *busqueda);
int nico_extraer_texto(const char *texto, int inicio, int longitud, char *destino, int max_len);
int nico_reemplazar_texto(char *texto, const char *busqueda, const char *reemplazo, int max_len);
int nico_a_mayusculas(char *texto);
int nico_a_minusculas(char *texto);
int nico_texto_a_entero(const char *texto);
double nico_texto_a_decimal(const char *texto);
int nico_entero_a_texto(int numero, char *texto, int max_len);
int nico_decimal_a_texto(double numero, char *texto, int max_len);
int nico_texto_vacio(const char *texto);
int nico_recortar_texto(char *texto);
int nico_caracter_a_texto(char caracter, char *texto, int max_len);
int nico_texto_a_caracter(const char *texto);
int nico_repetir_texto(const char *texto, int veces, char *destino, int max_len);
int nico_dividir_texto(const char *texto, char separador, int indice, char *destino, int max_len);
void eliminar_comentarios(char *linea);
int obtener_texto_para_cadena(const char *arg, char *buffer, int max_len);

// FUNCIONES DE E/S
void procesar_escribir(const char *linea);
void nico_posicionar_cursor(int fila, int columna);
void procesar_leer(const char *linea);
void procesar_leercaracter(const char *linea);
void procesar_leerhasta(const char *argumento);
void procesar_calcular(const char *linea);
void procesar_limpiarpantalla(void);
void procesar_esperar(const char *linea);
void procesar_escribirarchivo(const char *linea);
void procesar_leerarchivo(const char *linea);
void procesar_leerlinea(const char *linea);
void procesar_cerrararchivo(const char *linea);
void procesar_sistema(const char *linea);

// Funciones de colores 
void procesar_colortexto(const char *argumento);
void procesar_colorfondo(const char *argumento);
void procesar_resetcolor(const char *argumento);

// Funciones de efectos de texto
void procesar_textonegrita(const char *argumento);
void procesar_textocursiva(const char *argumento);
void procesar_textosubrayado(const char *argumento);
void procesar_textoreset(const char *argumento);

// Funciones de texto
void procesar_funcion_texto(const char *linea);

// FUNCIONES DE CONTROL DE FLUJO
int encontrar_fin_si(int linea_inicio);
int saltar_hasta_rama_o_fin_si(int linea_inicio);
int encontrar_fin_mientras(int linea_inicio, int nivel);
int encontrar_fin_proceder(int linea_inicio, int nivel);
int encontrar_fin_para(int linea_inicio, int nivel);
int encontrar_fin_segun(int linea_inicio);
int encontrar_siguiente_caso(int linea_inicio);
int encontrar_fin_subprograma(int linea_inicio);
int encontrar_fin_funcion(int linea_inicio);
int buscar_inicio_subprograma(const char *nombre);
int buscar_inicio_funcion(const char *nombre);
int encontrar_sino(int linea_inicio);
int ejecutar_salto_a_etiqueta(const char *nombre, int linea_actual);

// FUNCIONES DE SUBPROGRAMAS
int registrar_todas_las_etiquetas(void);
int registrar_todos_los_subprogramas(void);
int parsear_parametros_subprograma(const char *linea, char params[][MAX_NOMBRE], int max_params);
int registrar_subprograma(const char *nombre, int linea_inicio, int linea_fin, char params[][MAX_NOMBRE], int num_params);
int buscar_subprograma_info(const char *nombre);
void asignar_argumentos_a_parametros(char params[][MAX_NOMBRE], int num_params, char *args[], int num_args);

// FUNCIONES DE FUNCIONES
int registrar_todas_las_funciones(void);
int registrar_funcion(const char *nombre, int linea_inicio, int linea_fin, char params[][MAX_NOMBRE], int num_params, int tipo_retorno);
int buscar_funcion_info(const char *nombre);
int ejecutar_retornar(const char *valor, int linea_actual);
double llamar_funcion(const char *nombre, char *args[], int num_args, int *exito);
int declarar_parametros_funcion(int func_idx, char *args[], int num_args);

// FUNCIONES DEL INTÉRPRETE
void limpiar_memoria_completa(void);
void mostrar_prompt(void);
void comando_ayuda(void);
void comando_rangos(void);
int ejecutar_programa_cargado(void);
int validar_estructura_programa(char *nombre_programa);

// FUNCIONES DE MATRICES
int agregar_matriz_entera(const char *nombre, int filas, int columnas);
int agregar_matriz_decimal(const char *nombre, int filas, int columnas);
int agregar_matriz_entera_sin_signo(const char *nombre, int filas, int columnas);
int agregar_matriz_decimal_sin_signo(const char *nombre, int filas, int columnas);

int buscar_matriz_entera(const char *nombre);
int buscar_matriz_decimal(const char *nombre);
int buscar_matriz_entera_sin_signo(const char *nombre);
int buscar_matriz_decimal_sin_signo(const char *nombre);

int get_matriz_entera_valor(const char *nombre, int fila, int columna);
double get_matriz_decimal_valor(const char *nombre, int fila, int columna);
unsigned int get_matriz_entera_sin_signo_valor(const char *nombre, int fila, int columna);
double get_matriz_decimal_sin_signo_valor(const char *nombre, int fila, int columna);

void set_matriz_entera_valor(const char *nombre, int fila, int columna, int valor);
void set_matriz_decimal_valor(const char *nombre, int fila, int columna, double valor);
void set_matriz_entera_sin_signo_valor(const char *nombre, int fila, int columna, unsigned int valor);
void set_matriz_decimal_sin_signo_valor(const char *nombre, int fila, int columna, double valor);

// FUNCIONES DE LISTAS DE CARACTER
int agregar_lista_caracter(const char *nombre, int longitud);
int agregar_lista_caracter_sin_signo(const char *nombre, int longitud);
int buscar_lista_caracter(const char *nombre);
int buscar_lista_caracter_sin_signo(const char *nombre);
char get_lista_caracter_valor(const char *nombre, int indice);
unsigned char get_lista_caracter_sin_signo_valor(const char *nombre, int indice);
void set_lista_caracter_valor(const char *nombre, int indice, char valor);
void set_lista_caracter_sin_signo_valor(const char *nombre, int indice, unsigned char valor);

// FUNCIONES DE MATRICES DE CARACTER
int agregar_matriz_caracter(const char *nombre, int filas, int columnas);
int agregar_matriz_caracter_sin_signo(const char *nombre, int filas, int columnas);
int buscar_matriz_caracter(const char *nombre);
int buscar_matriz_caracter_sin_signo(const char *nombre);
char get_matriz_caracter_valor(const char *nombre, int fila, int columna);
unsigned char get_matriz_caracter_sin_signo_valor(const char *nombre, int fila, int columna);
void set_matriz_caracter_valor(const char *nombre, int fila, int columna, char valor);
void set_matriz_caracter_sin_signo_valor(const char *nombre, int fila, int columna, unsigned char valor);

// GESTIÓN DE SCOPES LOCALES
int crear_scope_local(int funcion_idx);
// Prototipos de gestión local (Listas y Matrices)
int buscar_lista_local(const char *nombre, int *tipo, int *indice_pool);
int registrar_lista_local(const char *nombre, int tipo, int indice_pool, int capacidad);
int buscar_matriz_local(const char *nombre, int *tipo, int *indice_pool);
int registrar_matriz_local(const char *nombre, int tipo, int indice_pool, int filas, int cols);
void eliminar_scope_local(void);
int agregar_variable_local(const char *nombre, int tipo, double valor);
int buscar_variable_local(const char *nombre, int *tipo, double *valor);
int buscar_texto_local(const char *nombre, char *buffer_out);
int agregar_texto_local(const char *nombre, const char *valor);

// PARSER DE DECLARACIONES DE CARACTER
int procesar_declaracion_lista_caracter(const char *linea, int linea_actual);
int procesar_declaracion_lista_caracter_sin_signo(const char *linea, int linea_actual);
int procesar_declaracion_matriz_caracter(const char *linea, int linea_actual);
int procesar_declaracion_matriz_caracter_sin_signo(const char *linea, int linea_actual);

// VALIDACIÓN DE LÍMITES
void validar_indice_matriz(const char *nombre, int fila, int columna, int filas_max, int columnas_max);
void validar_indice_lista(const char *nombre, int indice, int tamaño_max);

// FUNCIONES DE OPERACIONES BINARIAS - nico_bits.c 
unsigned int nico_bit_y(unsigned int a, unsigned int b);
unsigned int nico_bit_o(unsigned int a, unsigned int b);
unsigned int nico_bit_xor(unsigned int a, unsigned int b);
unsigned int nico_bit_no(unsigned int a);
unsigned int nico_desplazar_izquierda(unsigned int valor, int bits);
unsigned int nico_desplazar_derecha(unsigned int valor, int bits);
unsigned int nico_rotar_izquierda(unsigned int valor, int bits);
unsigned int nico_rotar_derecha(unsigned int valor, int bits);
unsigned int nico_leer_bit(unsigned int valor, int posicion);
unsigned int nico_activar_bit(unsigned int valor, int posicion);
unsigned int nico_desactivar_bit(unsigned int valor, int posicion);
unsigned int nico_invertir_bytes(unsigned int valor);
int nico_contar_bits(unsigned int valor);

// Wrappers para expressions.c
double nico_bity(double a, double b);
double nico_bito(double a, double b);
double nico_bitxor(double a, double b);
double nico_bitno(double a);
double nico_desplazarizquierda(double a, double b);
double nico_desplazarderecha(double a, double b);
double nico_rotarizquierda(double a, double b);
double nico_rotarderecha(double a, double b);
double nico_leerbit(double a, double b);
double nico_activarbit(double a, double b);
double nico_desactivarbit(double a, double b);
double nico_invertirbytes(double a);
double nico_contarbits(double a);

// PROTOTIPOS DE CADENAS.C 
double ejecutar_funcion_cadena(const char *nombre_func, char *args[], int num_args, int *exito);
int ejecutar_comando_cadena(const char *nombre_cmd, char *args[], int num_args);
int es_funcion_cadena_valida(const char *nombre);

// TIPOS COMPARTIDOS PARA MOTOR UNIFICADO
typedef struct {
    int linea_num;
    int linea_limite;
    int inicio_encontrado;
    char nombre_programa[MAX_NOMBRE];
    int declaraciones_permitidas;
    int en_bloque_principal;
    int en_subprograma;
    int en_funcion;
    int fase_declaraciones;
    int error_fatal;
    int si_ptr_inicio;
} CtxBloque;

// TIPO PARA TABLA DE COMANDOS
typedef struct {
    const char *keyword;
    int (*handler)(const char *linea, CtxBloque *ctx, int linea_actual);
} CmdEntry;

// Forward declaration del motor de ejecución
int ejecutar_bloque(CtxBloque *ctx);

extern int scope_actual;
extern ScopeLocal scopes_locales[MAX_SCOPES];
extern int en_funcion;
extern int profundidad_funcion;
extern int hay_valor_retorno;
extern double valor_retorno_funcion;
extern int num_lineas_programa;
extern int fin_principal_encontrado;
extern void nico_gpio_cleanup(void);

void restaurar_terminal_completa(void);

// VARIABLES DE SCOPES Y POOLS
extern VariableTexto texto_vars[MAX_VARS_TEXTO]; 
extern int num_texto_vars;

// Pools de texto
extern char *nombres_textos[];
extern int indices_textos[];

// Pools numéricos
extern Variable variables[];
extern VariableDecimal variables_decimal[];
extern VariableCaracter variables_caracter[];

// SCOPES & POOLS
extern int scope_actual;
extern int num_texto_vars;

// Prototipos 
int crear_scope_local(int funcion_idx);
void eliminar_scope_local(void);

#include <sqlite3.h>

// SQLite BD
typedef struct {
    sqlite3 *db;
    int conectado;
    sqlite3_stmt *stmt;
    int consulta_abierta;
    int col_count;
    char error_msg[512];
} ContextoBD;

extern ContextoBD ctx_bd;
extern int cmd_conectarbd(const char *linea, CtxBloque *ctx, int linea_actual);
extern int cmd_cerrarbd(const char *linea, CtxBloque *ctx, int linea_actual);
extern int cmd_ejecutarbd(const char *linea, CtxBloque *ctx, int linea_actual);
extern int cmd_consultarbd(const char *linea, CtxBloque *ctx, int linea_actual);
extern int cmd_cerrarconsultabd(const char *linea, CtxBloque *ctx, int linea_actual);
extern int cmd_iniciartransaccion(const char *l, CtxBloque *ctx, int la);
extern int cmd_confirmartransaccion(const char *l, CtxBloque *ctx, int la);
extern int cmd_deshacertransaccion(const char *l, CtxBloque *ctx, int la);

/* Función nativa para MIENTRAS() */
double func_siguientefilabd(int *exito);

#include "web.h"

int cmd_resettexto(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_resetcolor(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_limpiarpantalla(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_escribir(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_colortexto(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_colorfondo(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_textonegrita(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_textocursiva(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_textosubrayado(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_calcular(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_resultado(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_asignar(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_var_entera(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_var_decimal(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_var_texto(const char *linea, CtxBloque *ctx, int linea_actual) ;
int procesar_declaracion_variable_texto_extenso(const char *linea, int linea_actual);
int cmd_var_texto_extenso(const char *linea, CtxBloque *ctx, int linea_actual); 
int cmd_var_caracter(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_var_entera_sin(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_var_decimal_sin(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_var_caracter_sin(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_constante_entera(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_constante_decimal(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_constante_texto(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_constante_entera_sin(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_constante_decimal_sin(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_constante_caracter(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_constante_caracter_sin(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_lista_texto(const char *l, CtxBloque *c, int la);
int cmd_llamar_a(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_subprograma(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_fin_subprograma(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_si(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_sino_si(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_sino(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_fin_si(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_fin_segun(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_segun_caso(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_caso(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_por_defecto(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_funcion(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_fin_funcion(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_retornar(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_lista_entera(const char *l, CtxBloque *c, int la) ;
int cmd_lista_decimal(const char *l, CtxBloque *c, int la) ;
int cmd_lista_caracter(const char *l, CtxBloque *c, int la) ;
int cmd_lista_entera_sin(const char *l, CtxBloque *c, int la) ;
int cmd_lista_decimal_sin(const char *l, CtxBloque *c, int la) ;
int cmd_lista_caracter_sin(const char *l, CtxBloque *c, int la) ;
int cmd_matriz_entera(const char *l, CtxBloque *c, int la) ;
int cmd_matriz_decimal(const char *l, CtxBloque *c, int la) ;
int cmd_matriz_caracter(const char *l, CtxBloque *c, int la) ;
int cmd_matriz_entera_sin(const char *l, CtxBloque *c, int la) ;
int cmd_matriz_decimal_sin(const char *l, CtxBloque *c, int la) ;
int cmd_matriz_caracter_sin(const char *l, CtxBloque *c, int la) ;
int cmd_var_archivo(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_fin_para(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_fin_mientras(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_saltar_a(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_para(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_realizar(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_mientras(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_corte(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_configurar_pin(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_estado_pin(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_leer_pin(const char *linea, CtxBloque *ctx, int linea_actual) ;
int cmd_leertecla(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_ocultarcursor(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_mostrarcursor(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_tiempoms(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_anchoterminal(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_altoterminal(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_dibujarlinea(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_dibujarcirculo(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_rellenarrectangulo(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_teclamantenida(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_colisionrectangulos(const char *linea, CtxBloque *ctx, int linea_actual);

// MATRIZ DE TEXTO
#define MAX_FILAS_MAT 256
#define MAX_COLS_MAT  256

typedef struct {
    char nombre[MAX_NOMBRE];
    int filas, columnas;
    char ***valores; // [filas][columnas][MAX_TEXTO_LEN]
} MatrizTexto;

extern MatrizTexto matrices_texto[MAX_MATRICES];
extern int num_matrices_texto;

int buscar_matriz_texto_en_scope(const char *nombre, int scope_id);
int buscar_matriz_texto(const char *nombre);
int agregar_matriz_texto(const char *nombre, int filas, int columnas);
void set_matriz_texto_valor(const char *nombre, int f, int c, const char *valor);
char* get_matriz_texto_valor(const char *nombre, int f, int c);
int cmd_matriz_texto(const char *linea, CtxBloque *ctx, int linea_actual);
#endif
