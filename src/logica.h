/*
Nico v2.1.0 - Motor Lógico (estilo Prolog)
@file:         logica.h
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
@license:      MIT / Personal Use (ver LICENSE)
@description:  Definiciones públicas del motor de inferencia lógica.
               Contiene las estructuras de datos (BaseConocimiento, Hecho,
               Regla, Condicion, Bindings, ResultadoConsulta) y las
               funciones públicas para interactuar con el motor desde
               el evaluador (evaluator.c): agregar hechos/reglas,
               consultar, limpiar, imprimir, y wrappers para funciones
               internas necesarias por el evaluador.
*/
#ifndef LOGICA_H
#define LOGICA_H
#include <stdbool.h>
#include <stdlib.h>

#define MAX_ARGS 16
#define MAX_BINDINGS 64
#define MAX_HECHOS 256
#define MAX_REGLAS 64
#define MAX_CONDICIONES 16
#define MAX_SOLUCIONES 64

// ============================================================
// BINDINGS (sustituciones de variables)
// ============================================================
typedef struct
{
    char variable[64]; // Nombre de la variable (ej: "X", "Y_1")
    char valor[64];    // Valor asignado (ej: "juan", "pedro")
} Binding;

typedef struct
{
    Binding items[MAX_BINDINGS];
    int count;
} Bindings;

// ============================================================
// RESULTADO DE CONSULTA (para integración con el lenguaje)
// ============================================================
typedef struct
{
    Bindings soluciones[MAX_SOLUCIONES];
    int num_soluciones;
    bool exito; // true si hay al menos una solución
} ResultadoConsulta;

// ============================================================
// HECHO: predicado(arg1, arg2, ...)
// ============================================================
typedef struct
{
    char predicado[64];
    char args[MAX_ARGS][64];
    int num_args;
    int orden;
} Hecho;

// ============================================================
// CONDICIÓN (parte del cuerpo de una regla)
// ============================================================
typedef struct
{
    char predicado[64];
    char args[MAX_ARGS][64];
    int num_args;
    bool negacion;
    bool es_aritmetica; // true si es una condición aritmética (X ES 5 + 3)
    bool es_corte;      // true si es un operador de corte (! o CORTE)
    char operador[16];  // Operador aritmético: "ES", ">", "<", ">=", "<=", "=:="
} Condicion;

// ============================================================
// REGLA: cabeza SI cuerpo
// ============================================================
typedef struct
{
    char nombre[64];
    char cabeza_args[MAX_ARGS][64];
    int num_cabeza;
    Condicion cuerpo[MAX_CONDICIONES];
    int num_cuerpo;
    bool usa_neck;
    int orden;
} Regla;

// ============================================================
// BASE DE CONOCIMIENTO
// ============================================================
typedef struct
{
    Hecho hechos[MAX_HECHOS];
    int num_hechos;
    Regla reglas[MAX_REGLAS];
    int num_reglas;
} BaseConocimiento;

// ============================================================
// FUNCIONES PÚBLICAS
// ============================================================
BaseConocimiento *bc_crear(void);
void bc_destruir(BaseConocimiento *bc);
void bc_limpiar(BaseConocimiento *bc);
bool bc_agregar_hecho_desde_string(BaseConocimiento *bc, const char *str);
bool bc_agregar_regla_desde_string(BaseConocimiento *bc, const char *str);

// Devuelve los bindings en lugar de solo imprimir
ResultadoConsulta bc_consultar(BaseConocimiento *bc, const char *str, void *contexto);

// Función de impresión (mantiene compatibilidad)
void bc_imprimir_soluciones(BaseConocimiento *bc, const char *str);
void bc_imprimir(const BaseConocimiento *bc);

// ============================================================
// FUNCIONES PÚBLICAS PARA EVALUATOR
// ============================================================
bool parsear_predicado_publico(const char *str, char *predicado, char args[][64], int *num_args);
bool es_variable_publico(const char *s);
const char *resolver_var_publico(const Bindings *b, const char *nombre);
void recolectar_variables_en_arg_publico(const char *arg, char vars[][64], int *num_vars);
double evaluar_expresion_simple(const char *expr, const Bindings *bindings);
#endif // LOGICA_H