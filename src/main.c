/*
 * Nico v1.1.1 - Intérprete Educativo de Scripting en Español
 * @file:         main.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Punto de entrada y orquestación del intérprete. Gestiona la CLI,
 *                carga de programas, inicialización de la tabla de comandos y
 *                coordinación entre módulos.
 */
#include "nico.h"
#include "nico_gpio.h"
#include <unistd.h>
#include <limits.h>
#include <float.h>
#include <signal.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h> 
#endif

#define MAX_SI_RUNTIME 256
int si_runtime_stack[MAX_SI_RUNTIME];
int si_runtime_ptr = 0;

// TABLA DE DESPACHO DE COMANDOS
typedef int (*CmdHandler)(const char *linea, CtxBloque *ctx, int linea_actual);

// Handlers forward declarations
int cmd_resettexto(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_resetcolor(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_limpiarpantalla(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_escribir(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_colortexto(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_colorfondo(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_textonegrita(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_textocursiva(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_textosubrayado(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_calcular(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_resultado(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_asignar(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_var_entera(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_var_decimal(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_var_texto(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_var_texto_extenso(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_var_caracter(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_var_entera_sin(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_var_decimal_sin(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_var_caracter_sin(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_constante_entera(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_constante_decimal(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_constante_texto(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_constante_entera_sin(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_constante_decimal_sin(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_constante_caracter(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_constante_caracter_sin(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_llamar_a(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_subprograma(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_fin_subprograma(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_sinosi(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_sino_si(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_sino(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_si(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_fin_si(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_segun_caso(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_caso(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_por_defecto(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_fin_segun(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_funcion(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_fin_funcion(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_retornar(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_lista_entera(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_lista_decimal(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_lista_caracter(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_lista_entera_sin(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_lista_decimal_sin(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_lista_caracter_sin(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_matriz_entera(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_matriz_decimal(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_matriz_caracter(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_matriz_entera_sin(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_matriz_decimal_sin(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_matriz_caracter_sin(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_var_archivo(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_fin_para(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_fin_mientras(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_saltar_a(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_para(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_realizar(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_mientras(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_corte(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_configurar_pin(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_estado_pin(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_leer_pin(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_hora_actual(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_fecha_actual(const char *linea, CtxBloque *ctx, int linea_actual);

// DECLARACIONES FORWARD
int modo_estricto = 1;

void procesar_textonegrita(const char *argumento);
void procesar_textocursiva(const char *argumento);
void procesar_textosubrayado(const char *argumento);
void procesar_textoreset(const char *argumento);
void procesar_funcion_texto(const char *linea);

int encontrar_fin_para(int linea_inicio, int nivel);
int encontrar_fin_proceder(int linea_inicio, int nivel);
int encontrar_fin_mientras(int linea_inicio, int nivel_buscado);
int encontrar_fin_subprograma(int linea_inicio);
int encontrar_fin_funcion(int linea_inicio);
int ejecutar_salto_a_etiqueta(const char *nombre, int linea_actual);
int get_var_valor_global(const char *nombre);
void set_var_valor_global(const char *nombre, int valor);

// GPIO Raspberry Pi
void procesar_gpio_configurar(const char *argumento);
void procesar_gpio_estado_pin(const char *argumento);
void procesar_gpio_leer(const char *argumento);

// VARIABLES GLOBALES
char lineas_programa[MAX_LINEAS_PROGRAMA][MAX_LINEA];
int num_lineas_programa = 0;

MientrasBloque mientras_stack[MAX_NESTING];
int mientras_stack_ptr = 0;

ProcederBloque proceder_stack[MAX_NESTING];
int proceder_stack_ptr = 0;

SiBloque si_stack[MAX_NESTING];
int si_stack_ptr = 0;

ParaBloque para_stack[MAX_NESTING];
int para_stack_ptr = 0;

SegunBloque segun_stack[MAX_NESTING];
int segun_stack_ptr = 0;

SubBloque sub_stack[MAX_NESTING];
int sub_stack_ptr = 0;

FuncionBloque funcion_stack[MAX_NESTING];
int funcion_stack_ptr = 0;

int en_bloque_principal = 0;
int en_subprograma = 0;
int en_funcion = 0;
int error_fatal = 0;
int fase_declaraciones = 1;
int fin_principal_encontrado = 0;

char funcion_variable_destino[MAX_NOMBRE] = "";
char funcion_destino_stack[MAX_NESTING][MAX_NOMBRE];
int funcion_destino_stack_ptr = 0;

// FUNCIONES AUXILIARES
void limpiar_memoria_completa(void) {
    num_variables = 0;
    num_variables_sin_signo = 0;
    num_variables_decimal = 0;
    num_variables_decimal_sin_signo = 0;
    num_variables_caracter = 0;
    num_variables_caracter_sin_signo = 0;
    num_texto_vars = 0;
    num_constantes = 0;
    num_constantes_sin_signo = 0;
    num_constantes_decimal = 0;
    num_constantes_decimal_sin_signo = 0;
    num_constantes_caracter = 0;
    num_constantes_caracter_sin_signo = 0;
    num_texto_constantes = 0;
    num_listas_enteras = 0;
    num_listas_decimales = 0;
    num_listas_enteras_sin_signo = 0;
    num_listas_decimales_sin_signo = 0;
    num_etiquetas = 0;
    cerrar_todos_los_archivos();
    num_variables_archivo = 0;
    num_lineas_programa = 0;
    mientras_stack_ptr = 0;
    proceder_stack_ptr = 0;
    si_stack_ptr = 0;
    para_stack_ptr = 0;
    segun_stack_ptr = 0;
    sub_stack_ptr = 0;
    funcion_stack_ptr = 0;
    fase_constantes = 1;
    fase_variables = 0;
    en_bloque_principal = 0;
    en_subprograma = 0;
    en_funcion = 0;
    fase_declaraciones = 1;
    fin_principal_encontrado = 0;
    num_subprogramas_registrados = 0;
    num_funciones_registradas = 0;
    valor_retorno_funcion = 0;
    hay_valor_retorno = 0;
    funcion_variable_destino[0] = '\0';
    funcion_destino_stack_ptr = 0;
    num_matriz_enteras = 0;
    num_matriz_decimales = 0;
    num_matriz_enteras_sin_signo = 0;
    num_matriz_decimales_sin_signo = 0;
    si_runtime_ptr = 0;
}

void mostrar_prompt(void) {
    printf(">>> ");
    fflush(stdout);
}

void comando_ayuda(void) {
    fprintf(stderr, "\n");
    fprintf(stderr, "╔════════════════════════════════════════════════════════╗\n");
    fprintf(stderr, "║           COMANDOS DEL INTÉRPRETE NICO                 ║\n");
    fprintf(stderr, "╠════════════════════════════════════════════════════════╣\n");
    fprintf(stderr, "║  USAR archivo.nico   Carga un archivo .nico            ║\n");
    fprintf(stderr, "║  CORRER              Ejecuta el programa cargado       ║\n");
    fprintf(stderr, "║  CERRAR              Cierra el programa actual         ║\n");
    fprintf(stderr, "║  RANGOS              Muestra rangos de variables       ║\n");
    fprintf(stderr, "║  SALIR               Sale del intérprete               ║\n");
    fprintf(stderr, "║  ?                   Muestra esta ayuda                ║\n");
    fprintf(stderr, "╚════════════════════════════════════════════════════════╝\n");
    fprintf(stderr, "\n");
}

void comando_rangos(void) {
    fprintf(stderr, "\n");
    fprintf(stderr, "   RANGOS DE VARIABLES - NICO v1.1.1\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "   VARIABLES ENTERAS:\n");
    fprintf(stderr, "     Rango: %d a %d\n", INT_MIN, INT_MAX);
    fprintf(stderr, "     Máximo: %d vars\n", MAX_VARS);
    fprintf(stderr, "\n");
    fprintf(stderr, "   VARIABLES ENTERAS SIN SIGNO:\n");
    fprintf(stderr, "     Rango: 0 a %u\n", UINT_MAX);
    fprintf(stderr, "     Máximo: %d vars\n", MAX_VARS_SIN_SIGNO);
    fprintf(stderr, "\n");
    fprintf(stderr, "   VARIABLES DECIMALES:\n");
    fprintf(stderr, "     Rango: %e a %e\n", -DBL_MAX, DBL_MAX);
    fprintf(stderr, "     Máximo: %d vars\n", MAX_VARS_DECIMAL);
    fprintf(stderr, "\n");
    fprintf(stderr, "   VARIABLES DECIMALES SIN SIGNO:\n");
    fprintf(stderr, "     Rango: 0.0 a %e (mismo tipo que DECIMAL, solo valores ≥ 0)\n", DBL_MAX);
    fprintf(stderr, "     Máximo: %d vars\n", MAX_VARS_DECIMAL_SIN_SIGNO);
    fprintf(stderr, "\n");
    fprintf(stderr, "   VARIABLES CARACTER:\n");
    fprintf(stderr, "     Rango: %d a %d (ASCII)\n", SCHAR_MIN, SCHAR_MAX);
    fprintf(stderr, "     Máximo: %d vars\n", MAX_VARS_CARACTER);
    fprintf(stderr, "\n");
    fprintf(stderr, "   VARIABLES CARACTER SIN SIGNO:\n");
    fprintf(stderr, "     Rango: 0 a %u (ASCII)\n", UCHAR_MAX);
    fprintf(stderr, "     Máximo: %d vars\n", MAX_VARS_CARACTER_SIN_SIGNO);
    fprintf(stderr, "\n");
    fprintf(stderr, "   VARIABLES TEXTO:\n");
    fprintf(stderr, "     Máximo: %d caracteres por variable\n", MAX_TEXTO_LEN - 1);
    fprintf(stderr, "     Máximo: %d vars\n", MAX_VARS_TEXTO);
    fprintf(stderr, "\n");
    fprintf(stderr, "   VARIABLES TEXTO EXTENSO:\n");
    fprintf(stderr, "     Largo: Dinámico (limitado por RAM disponible)\n");
    fprintf(stderr, "     Máximo: %d vars (globales)\n", MAX_TEXTOS_EXT_GLOBALES);
    fprintf(stderr, "     Alcance: Solo global (no válido en bloques locales)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "   CONSTANTES (todos los tipos):\n");
    fprintf(stderr, "     Máximo: %d constantes\n", MAX_CONSTANTES);
    fprintf(stderr, "     Máximo: %d constantes sin signo\n", MAX_CONSTANTES_SIN_SIGNO);
    fprintf(stderr, "     Máximo: %d constantes decimales\n", MAX_CONSTANTES_DECIMAL);
    fprintf(stderr, "     Máximo: %d constantes texto\n", MAX_CONSTANTES_TEXTO);
    fprintf(stderr, "\n");
    fprintf(stderr, "   LISTAS / ARRAYS:\n");
    fprintf(stderr, "     Máximo: %d elementos por lista\n", MAX_LISTA);
    fprintf(stderr, "     Máximo: %d listas enteras\n", MAX_LISTAS_ENTERAS);
    fprintf(stderr, "     Máximo: %d listas decimales\n", MAX_LISTAS_DECIMALES);
    fprintf(stderr, "     Máximo: %d listas sin signo\n", MAX_LISTAS_ENTERAS_SIN_SIGNO);
    fprintf(stderr, "\n");
    fprintf(stderr, "   ARCHIVOS:\n");
    fprintf(stderr, "     Máximo: %d archivos simultáneos\n", MAX_VARS_ARCHIVO);
    fprintf(stderr, "     Modos: 0=ESCRITURA, 1=AGREGAR, 2=LECTURA, 3=LECTOESCRITURA\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "   FUNCIONES:\n");
    fprintf(stderr, "     Máximo: %d niveles de anidamiento\n", MAX_NESTING);
    fprintf(stderr, "     Máximo: %d parámetros por función\n", MAX_PARAMETROS);
    fprintf(stderr, "\n");
    fprintf(stderr, "   PROGRAMA:\n");
    fprintf(stderr, "     Máximo: %d líneas de código\n", MAX_LINEAS_PROGRAMA);
    fprintf(stderr, "     Máximo: %d caracteres por línea\n", MAX_LINEA - 1);
    fprintf(stderr, "\n");
}

int validar_estructura_programa(char *nombre_programa) {
    int inicio_encontrado = 0, fin_encontrado = 0;
    int bloque_principal_encontrado = 0;
    int fin_principal_encontrado_local = 0;
    
    for (int i = 0; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        
        if (!inicio_encontrado) {
            if (comienza_con(linea, "PROGRAMA")) {
                char *ptr = linea + 8;
                while (*ptr == ' ' || *ptr == '\t') ptr++;
                sscanf(ptr, "%s", nombre_programa);
                if (!strlen(nombre_programa)) {
                    fprintf(stderr, "Error línea %d: PROGRAMA requiere un nombre.\n", i + 1);
                    return 0;
                }
                inicio_encontrado = 1;
            } else {
                fprintf(stderr, "Error línea %d: El programa debe comenzar con PROGRAMA.\n", i + 1);
                return 0;
            }
            continue;
        }
        
        if (comienza_con(linea, "BLOQUE PRINCIPAL")) {
        scope_actual = -1;  
        bloque_principal_encontrado = 1;
        }

        if (comienza_con(linea, "FIN PRINCIPAL")) {
            fin_principal_encontrado_local = 1;
        }
        if (comienza_con(linea, "FINAL")) {
            fin_encontrado = 1;
            break;
        }
    }
    
    if (!inicio_encontrado) {
        fprintf(stderr, "Error: No se encontro sentencia PROGRAMA.\n");
        return 0;
    }
    if (!bloque_principal_encontrado) {
        fprintf(stderr, "Error: El programa no tiene BLOQUE PRINCIPAL.\n");
        return 0;
    }
    if (!fin_principal_encontrado_local) {
        fprintf(stderr, "Error: El programa no tiene FIN PRINCIPAL.\n");
        return 0;
    }
    if (!fin_encontrado) {
        fprintf(stderr, "Error: El programa no tiene sentencia FINAL.\n");
        return 0;
    }
    return 1;
}

int cargar_archivo_en_memoria(const char *nombre_archivo) {
    FILE *archivo = fopen(nombre_archivo, "r");
    if (!archivo) return -1;
    
    num_lineas_programa = 0;
    char buffer[MAX_LINEA];
    while (fgets(buffer, MAX_LINEA, archivo) && num_lineas_programa < MAX_LINEAS_PROGRAMA) {
        limpiar_string(buffer);
        strncpy(lineas_programa[num_lineas_programa], buffer, MAX_LINEA - 1);
        lineas_programa[num_lineas_programa][MAX_LINEA - 1] = '\0';
        num_lineas_programa++;
    }
    fclose(archivo);
    return 0;
}

// EJECUTAR LLAMADA A FUNCION
int ejecutar_llamada_funcion(const char *linea, int linea_actual, int linea_num) {
    char *igual = strchr(linea, '=');
    if (!igual) return -1;
    
    char lado_izq[MAX_LINEA];
    int len_izq = igual - linea;
    if (len_izq >= MAX_LINEA) len_izq = MAX_LINEA - 1;
    strncpy(lado_izq, linea, len_izq);
    lado_izq[len_izq] = '\0';
    limpiar_string(lado_izq);
    
    char nombre_variable[MAX_NOMBRE] = "";
    char *ptr = lado_izq;
    if (comienza_con(ptr, "CALCULAR EN")) {
        ptr += 11;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
    }
    if (*ptr == '$') ptr++;
    int i = 0;
    while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) nombre_variable[i++] = *ptr++;
    nombre_variable[i] = '\0';
    
    char *fin_igual = igual + 1;
    while (*fin_igual == ' ' || *fin_igual == '\t') fin_igual++;
    
    char nombre_funcion[MAX_NOMBRE];
    i = 0;
    ptr = fin_igual;
    while (*ptr && *ptr != '(' && !isspace((unsigned char)*ptr) && i < MAX_NOMBRE - 1) {
        if (*ptr == '$') ptr++;
        else nombre_funcion[i++] = *ptr++;
    }
    nombre_funcion[i] = '\0';
    limpiar_string(nombre_funcion);
    
    int func_idx = buscar_funcion_info(nombre_funcion);
    if (func_idx < 0) {
        fprintf(stderr, "Error línea %d: Función '$%s' no declarada.\n", linea_actual, nombre_funcion);
        return -1;
    }
    
    char *args[MAX_PARAMETROS];
    int num_args = 0;
    if (*ptr == '(') {
        ptr++;
        while (*ptr && *ptr != ')' && num_args < MAX_PARAMETROS) {
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            char arg[MAX_LINEA] = "";
            i = 0;
            int nivel_parentesis = 0;
            while (*ptr && (nivel_parentesis > 0 || (*ptr != ',' && *ptr != ')')) && i < MAX_LINEA - 1) {
                if (*ptr == '(') nivel_parentesis++;
                else if (*ptr == ')') nivel_parentesis--;
                arg[i++] = *ptr++;
            }
            arg[i] = '\0';
            limpiar_string(arg);
            if (strlen(arg) > 0) args[num_args++] = strdup(arg);
            if (*ptr == ',') ptr++;
        }
    }
    
    if (num_args != funciones_registradas[func_idx].num_params) {
        fprintf(stderr, "Error línea %d: Función '$%s' espera %d parámetros, se recibieron %d.\n",
                linea_actual, nombre_funcion,
                funciones_registradas[func_idx].num_params, num_args);
        for (int k = 0; k < num_args; k++) free(args[k]);
        return -1;
    }
    
    if (funcion_destino_stack_ptr < MAX_NESTING) {
        strncpy(funcion_destino_stack[funcion_destino_stack_ptr], nombre_variable, MAX_NOMBRE - 1);
        funcion_destino_stack[funcion_destino_stack_ptr][MAX_NOMBRE - 1] = '\0';
        funcion_destino_stack_ptr++;
    }
    strncpy(funcion_variable_destino, nombre_variable, MAX_NOMBRE - 1);
    funcion_variable_destino[MAX_NOMBRE - 1] = '\0';
    
    int exito;
    double resultado = llamar_funcion(nombre_funcion, args, num_args, &exito);
    
    for (int k = 0; k < num_args; k++) free(args[k]);
    
    if (!exito) {
        if (funcion_destino_stack_ptr > 0) funcion_destino_stack_ptr--;
        if (funcion_destino_stack_ptr >= 0) {
            strncpy(funcion_variable_destino, funcion_destino_stack[funcion_destino_stack_ptr], MAX_NOMBRE - 1);
            funcion_variable_destino[MAX_NOMBRE - 1] = '\0';
        } else {
            funcion_variable_destino[0] = '\0';
        }
        return -1;
    }
    
    if (nombre_variable[0] != '\0') {
        char *nombre = nombre_variable;
        if (nombre[0] == '$') nombre++;
        
        int idx = buscar_variable(nombre);
        if (idx >= 0) { variables[idx].valor = (int)resultado; }
        else {
            idx = buscar_variable_sin_signo(nombre);
            if (idx >= 0) { variables_sin_signo[idx].valor = (unsigned int)resultado; }
            else {
                idx = buscar_variable_decimal(nombre);
                if (idx >= 0) { variables_decimal[idx].valor = resultado; }
                else {
                    idx = buscar_variable_decimal_sin_signo(nombre);
                    if (idx >= 0) { variables_decimal_sin_signo[idx].valor = resultado; }
                    else {
                        idx = buscar_variable_caracter(nombre);
                        if (idx >= 0) { variables_caracter[idx].valor = (char)resultado; }
                        else {
                            idx = buscar_variable_caracter_sin_signo(nombre);
                            if (idx >= 0) { variables_caracter_sin_signo[idx].valor = (unsigned char)resultado; }
                            else {
                                agregar_variable_decimal(nombre, resultado);
                            }
                        }
                    }
                }
            }
        }
    }

    if (funcion_destino_stack_ptr > 0) {
        funcion_destino_stack_ptr--;
        if (funcion_destino_stack_ptr >= 0) {
            strncpy(funcion_variable_destino, funcion_destino_stack[funcion_destino_stack_ptr], MAX_NOMBRE - 1);
            funcion_variable_destino[MAX_NOMBRE - 1] = '\0';
        } else {
            funcion_variable_destino[0] = '\0';
        }
    }
    
    return linea_num + 1;
}

// ASIGNAR VALOR DE RETORNO DE FUNCION
void asignar_valor_retorno_funcion(void) {
    if (funcion_variable_destino[0] == '\0') return;
    if (!hay_valor_retorno) return;
    
    char *nombre = funcion_variable_destino;
    if (nombre[0] == '$') nombre++;
    
    int idx = buscar_variable(nombre);
    if (idx >= 0) {
        variables[idx].valor = (int)valor_retorno_funcion;
        return;
    }
    
    idx = buscar_variable_sin_signo(nombre);
    if (idx >= 0) {
        variables_sin_signo[idx].valor = (unsigned int)valor_retorno_funcion;
        return;
    }
    
    idx = buscar_variable_decimal(nombre);
    if (idx >= 0) {
        variables_decimal[idx].valor = valor_retorno_funcion;
        return;
    }
    
    idx = buscar_variable_decimal_sin_signo(nombre);
    if (idx >= 0) {
        variables_decimal_sin_signo[idx].valor = valor_retorno_funcion;
        return;
    }
    
    idx = buscar_variable_caracter(nombre);
    if (idx >= 0) {
        variables_caracter[idx].valor = (char)valor_retorno_funcion;
        return;
    }
    
    idx = buscar_variable_caracter_sin_signo(nombre);
    if (idx >= 0) {
        variables_caracter_sin_signo[idx].valor = (unsigned char)valor_retorno_funcion;
        return;
    }
    
    agregar_variable_decimal(nombre, valor_retorno_funcion);
}

// EJECUTAR PROGRAMA CARGADO
int ejecutar_programa_cargado(void) {
    si_runtime_ptr = 0;
    inicializar_semilla_aleatoria();
    scope_actual = -1;
    for (int i = 0; i < MAX_SCOPES; i++) {
        scopes_locales[i].num_variables = 0;
    }
    funcion_stack_ptr = 0;
    if (num_lineas_programa == 0) {
        fprintf(stderr, "Error: No hay ningún programa cargado.\n");
        return -1;
    }
    
    funcion_destino_stack_ptr = 0;
    funcion_variable_destino[0] = '\0';
    hay_valor_retorno = 0;
    valor_retorno_funcion = 0;
    fin_principal_encontrado = 0;
    
    char nombre_programa[MAX_NOMBRE] = "";
    
    if (!validar_estructura_programa(nombre_programa)) return -1;
    
    fprintf(stderr, "> Programa: %s\n", nombre_programa);
    
    registrar_todas_las_etiquetas();
    registrar_todos_los_subprogramas();
    
    for (int i = 0; i < num_subprogramas_registrados; i++) {
        int inicio_sub = subprogramas_registrados[i].linea_inicio + 1;
        int fin_sub = encontrar_fin_subprograma(subprogramas_registrados[i].linea_inicio);
        if (fin_sub != -1 && inicio_sub <= fin_sub) {
            validar_estructura_bloques(inicio_sub, fin_sub);
        }
    }

    if (registrar_todas_las_funciones() != 0) {
        fprintf(stderr, "Error: Hay funciones inválidas. Ejecución finalizada.\n");
        return -1;
    }    

    fprintf(stderr, "> Validando nombres de variables y parámetros...\n");
    
    for (int f = 0; f < num_funciones_registradas; f++) {
        for (int p = 0; p < funciones_registradas[f].num_params; p++) {
            char *param = funciones_registradas[f].params[p];
            
            if (buscar_variable(param) >= 0) {
                fprintf(stderr, "ADVERTENCIA: Función '$%s' tiene parámetro '$%s' que colisiona con otra variable.\n", funciones_registradas[f].nombre, param);
            }
            else if (buscar_variable_sin_signo(param) >= 0) {
                fprintf(stderr, "ADVERTENCIA: Función '$%s' tiene parámetro '$%s' que colisiona con variable global ENTERA SIN SIGNO.\n", funciones_registradas[f].nombre, param);
            }
            else if (buscar_variable_decimal(param) >= 0) {
                fprintf(stderr, "ADVERTENCIA: Función '$%s' tiene parámetro '$%s' que colisiona con variable global DECIMAL.\n", funciones_registradas[f].nombre, param);
            }
            else if (buscar_variable_decimal_sin_signo(param) >= 0) {
                fprintf(stderr, "ADVERTENCIA: Función '$%s' tiene parámetro '$%s' que colisiona con variable global DECIMAL SIN SIGNO.\n", funciones_registradas[f].nombre, param);
            }
            else if (buscar_variable_caracter(param) >= 0) {
                fprintf(stderr, "ADVERTENCIA: Función '$%s' tiene parámetro '$%s' que colisiona con variable global CARACTER.\n", funciones_registradas[f].nombre, param);
            }
            else if (buscar_variable_caracter_sin_signo(param) >= 0) {
                fprintf(stderr, "ADVERTENCIA: Función '$%s' tiene parámetro '$%s' que colisiona con variable global CARACTER SIN SIGNO.\n", funciones_registradas[f].nombre, param);
            }
        }
    }
    
    CtxBloque ctx = {
        .linea_num = 0,
        .linea_limite = -1,   
        .inicio_encontrado = 0,
        .nombre_programa = "",
        .declaraciones_permitidas = 1,
        .en_bloque_principal = 0,
        .en_subprograma = 0,
        .en_funcion = 0,
        .fase_declaraciones = 1,
        .error_fatal = 0
    };

    if (!validar_estructura_programa(ctx.nombre_programa)) return -1;
    
    ejecutar_bloque(&ctx);    
    nico_gpio_cleanup();
    scope_actual = -1;
    en_funcion = 0;
    cerrar_todos_los_archivos();
    fprintf(stderr, "> Programa '%s' finalizado.\n", ctx.nombre_programa);
    restaurar_terminal_completa();
    limpiar_memoria_completa();
    si_runtime_ptr = 0;
    return 0;
}

int cmd_resettexto(const char *linea, CtxBloque *ctx, int linea_actual) {
    const char *ptr = linea + 10;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr == '(' || *ptr == ')') {
        fprintf(stderr, "Error línea %d: RESETTEXTO no lleva paréntesis.\n", linea_actual);
        ctx->error_fatal = 1; return -1;
    }
    printf("\033[22;23;24m"); 
    fflush(stdout);
    ctx->linea_num++; return 0;
}

int cmd_resetcolor(const char *linea, CtxBloque *ctx, int linea_actual) {
    const char *ptr = linea + 10;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr == '(' || *ptr == ')') {
        fprintf(stderr, "Error línea %d: RESETCOLOR no lleva paréntesis. Uso: RESETCOLOR\n", linea_actual);
        ctx->error_fatal = 1; return -1;
    }
    procesar_resetcolor(NULL); 
    fflush(stdout);
    ctx->linea_num++; return 0;
}

int cmd_limpiarpantalla(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea; (void)linea_actual;
    procesar_limpiarpantalla(); ctx->linea_num++; return 0;
}

static int extraer_arg_escribir(const char *linea, char *dest, size_t max_dest) {
    const char *abre = strchr(linea, '(');
    const char *cierra = strrchr(linea, ')');
    if (!abre || !cierra || cierra <= abre) return -1;

    const char *p = abre + 1;
    size_t j = 0;
    
    if (*p == '"') p++;

    while (p < cierra && j < max_dest - 1) {
        if (*p == '\\' && *(p+1) != '\0') {
            dest[j++] = *p++; 
            dest[j++] = *p++;
        } else if (*p == '"') {
            break;
        } else {
            dest[j++] = *p++;
        }
    }
    dest[j] = '\0';
    return 0;
}

int cmd_escribir(const char *linea, CtxBloque *ctx, int linea_actual) {
    char arg[MAX_LINEA] = {0};
    if (extraer_arg_escribir(linea, arg, MAX_LINEA) != 0) {
        fprintf(stderr, "Error línea %d: ESCRIBIR requiere argumento entre paréntesis.\n", linea_actual);
        ctx->error_fatal = 1; return -1;
    }
    
    arg[MAX_LINEA - 1] = '\0';
    procesar_escribir(arg);
    fflush(stdout);
    ctx->linea_num++; 
    return 0;
}

int cmd_colortexto(const char *linea, CtxBloque *ctx, int linea_actual) {
    const char *start = strchr(linea, '(');
    const char *end = strrchr(linea, ')');
    if (!start || !end || end <= start) {
        fprintf(stderr, "Error línea %d: COLORTEXTO requiere argumento entre paréntesis.\n", linea_actual);
        ctx->error_fatal = 1; return -1;
    }
    start++;
    char arg[32];
    size_t len = end - start;
    if (len >= sizeof(arg)) len = sizeof(arg) - 1;
    strncpy(arg, start, len);
    arg[len] = '\0';
    procesar_colortexto(arg);
    fflush(stdout);
    ctx->linea_num++; return 0;
}

int cmd_colorfondo(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea_actual;
    const char *ptr = linea + 10;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    char arg[MAX_LINEA] = "";
    if (*ptr == '(') {
        ptr++;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        int i = 0;
        while (*ptr && *ptr != ')' && i < MAX_LINEA - 1) arg[i++] = *ptr++;
        arg[i] = '\0';
    }
    procesar_colorfondo(arg);
    fflush(stdout);
    ctx->linea_num++; return 0;
}

int cmd_textonegrita(const char *linea, CtxBloque *ctx, int linea_actual) {
    const char *ptr = linea + 13;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr == '(' || *ptr == ')') {
        fprintf(stderr, "Error línea %d: TEXTONEGRITA no lleva paréntesis.\n", linea_actual);
        ctx->error_fatal = 1; return -1;
    }
    procesar_textonegrita(NULL);
    fflush(stdout);
    ctx->linea_num++; return 0;
}

int cmd_textocursiva(const char *linea, CtxBloque *ctx, int linea_actual) {
    const char *ptr = linea + 12;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr == '(' || *ptr == ')') {
        fprintf(stderr, "Error línea %d: TEXTOCURSIVA no lleva paréntesis.\n", linea_actual);
        ctx->error_fatal = 1; return -1;
    }
    procesar_textocursiva(NULL);
    fflush(stdout);
    ctx->linea_num++; return 0;
}

int cmd_textosubrayado(const char *linea, CtxBloque *ctx, int linea_actual) {
    const char *ptr = linea + 14;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr == '(' || *ptr == ')') {
        fprintf(stderr, "Error línea %d: TEXTOSUBRAYADO no lleva paréntesis.\n", linea_actual);
        ctx->error_fatal = 1; return -1;
    }
    procesar_textosubrayado(NULL);
    fflush(stdout);
    ctx->linea_num++; return 0;
}

int cmd_calcular(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea_actual; 
    procesar_calcular(linea + 11);
    if (ctx->error_fatal) {
        fprintf(stderr, "Error fatal: Ejecución terminada.\n");
        return -1;
    }
    ctx->linea_num++; return 0;
}

int cmd_resultado(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea_actual;
    procesar_calcular(linea + 12);
    ctx->linea_num++; return 0;
}

int cmd_asignar(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea_actual;
    
    const char *ptr = strchr(linea, '$');
    if (!ptr) {
        procesar_calcular(linea + 10);
        ctx->linea_num++; return 0;
    }
    ptr++;
    
    char nombre_dest[MAX_NOMBRE];
    int i = 0;
    while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) nombre_dest[i++] = *ptr++;
    nombre_dest[i] = '\0';
    
    if (*ptr == '[') {
        procesar_calcular(linea + 10);
        ctx->linea_num++; return 0;
    } 

    // 1. Manejo de TEXTO EXTENSO
    int es_ext = 0, idx_ext = -1, scope_ext = -1;
    if (buscar_texto_extenso(nombre_dest, &es_ext, &idx_ext, &scope_ext) >= 0) {
        const char *eq = strchr(linea, '=');
        if (!eq) {
            fprintf(stderr, "Error línea %d: Falta '=' en ASIGNAR.\n", linea_actual);
            ctx->linea_num++; return -1;
        }
        eq++; while (*eq == ' ' || *eq == '\t') eq++;
        
        char valor_final[MAX_LINEA * 2] = "";
        const char *p = eq;
        
        while (*p) {
            while (*p == ' ' || *p == '\t') p++;
            if (!*p) break;
            
            if (*p == '"') {
                p++;
                while (*p && *p != '"') {
                    size_t len = strlen(valor_final);
                    if (len < sizeof(valor_final) - 2) valor_final[len] = *p;
                    p++;
                }
                if (*p == '"') p++;
            } 
            else if (*p == '$') {
                p++;
                char var_ref[MAX_NOMBRE]; int j=0;
                while (es_alnum(*p) && j < MAX_NOMBRE-1) var_ref[j++] = *p++;
                var_ref[j] = '\0';
                
                int es_r, idx_r, scope_r;
                if (buscar_texto_extenso(var_ref, &es_r, &idx_r, &scope_r) >= 0) {
                    char *val = es_r ? scopes_locales[scope_r].textos_ext[idx_r].valor : textos_ext_globales[idx_r].valor;
                    if (val) strncat(valor_final, val, sizeof(valor_final) - strlen(valor_final) - 1);
                } else {
                    int idx_txt = buscar_texto_var(var_ref);
                    if (idx_txt >= 0) strncat(valor_final, texto_vars[idx_txt].valor, sizeof(valor_final) - strlen(valor_final) - 1);
                }
            }
            else if (*p == '+') {
                p++;
            }
            else {
                break;
            }
        }
        
        if (asignar_texto_extenso_valor(es_ext, idx_ext, scope_ext, valor_final) < 0) {
            fprintf(stderr, "Error línea %d: Fallo al asignar TEXTO EXTENSO '$%s'.\n", linea_actual, nombre_dest);
            ctx->linea_num++; return -1;
        }
        ctx->linea_num++; return 0;
    }
    
    const char *eq = strchr(linea, '=');
    if (eq) {
        const char *ptr_valor = eq + 1;
        while (*ptr_valor == ' ' || *ptr_valor == '\t') ptr_valor++;
        
        char valor_orig[MAX_TEXTO_LEN] = "";
        
        if (*ptr_valor == '"') {
            ptr_valor++;
            int j = 0;
            while (*ptr_valor && *ptr_valor != '"' && j < MAX_TEXTO_LEN - 1) valor_orig[j++] = *ptr_valor++;
            valor_orig[j] = '\0';
        } else if (*ptr_valor == '$') {
            ptr_valor++;
            char nombre_orig[MAX_NOMBRE];
            int j = 0;
            while (es_alnum(*ptr_valor) && j < MAX_NOMBRE - 1) nombre_orig[j++] = *ptr_valor++;
            nombre_orig[j] = '\0';
            
            int idx_orig_global = buscar_texto_var(nombre_orig);
            if (idx_orig_global >= 0) {
                strcpy(valor_orig, texto_vars[idx_orig_global].valor);
            } else {
                int encontrado = 0;
                if (scope_actual >= 0) {
                    for (int s = scope_actual; s >= 0 && !encontrado; s--) {
                        for (int v = 0; v < scopes_locales[s].num_textos && !encontrado; v++) {
                            if (strcmp(scopes_locales[s].nombres_textos[v], nombre_orig) == 0) {
                                int idx_pool = scopes_locales[s].indices_textos[v];
                                strcpy(valor_orig, texto_vars[idx_pool].valor);
                                encontrado = 1;
                            }
                        }
                    }
                }
                if (!encontrado) {
                    procesar_calcular(linea + 10);
                    ctx->linea_num++; return 0;
                }
            }
        } else {
            procesar_calcular(linea + 10);
            ctx->linea_num++; return 0;
        }
        
        int idx_dest_global = buscar_texto_var(nombre_dest);
        if (idx_dest_global >= 0) {
            strncpy(texto_vars[idx_dest_global].valor, valor_orig, MAX_TEXTO_LEN - 1);
            texto_vars[idx_dest_global].valor[MAX_TEXTO_LEN - 1] = '\0';
            ctx->linea_num++; return 0;
        }
        
        if (scope_actual >= 0) {
            for (int s = scope_actual; s >= 0; s--) {
                for (int v = 0; v < scopes_locales[s].num_textos; v++) {
                    if (strcmp(scopes_locales[s].nombres_textos[v], nombre_dest) == 0) {
                        int idx_pool = scopes_locales[s].indices_textos[v];
                        strncpy(texto_vars[idx_pool].valor, valor_orig, MAX_TEXTO_LEN - 1);
                        texto_vars[idx_pool].valor[MAX_TEXTO_LEN - 1] = '\0';
                        ctx->linea_num++; return 0;
                    }
                }
            }
        }
        
        int es_ext_f=0, idx_ext_f=-1, scope_ext_f=-1;
        if (buscar_texto_extenso(nombre_dest, &es_ext_f, &idx_ext_f, &scope_ext_f) >= 0) {
            if (asignar_texto_extenso_valor(es_ext_f, idx_ext_f, scope_ext_f, valor_orig) >= 0) {
                ctx->linea_num++; return 0;
            }
        }
    }
    
    procesar_calcular(linea + 10);
    ctx->linea_num++; 
    return 0;
} 

static inline int check_declaracion(CtxBloque *ctx, int linea_actual, const char *msg) {
    if (!ctx->declaraciones_permitidas) {
        fprintf(stderr, "Error línea %d%s\n", linea_actual, msg);
        ctx->error_fatal = 1;
        return -1;
    }
    return 0;
}

int cmd_var_entera(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (check_declaracion(ctx, linea_actual, ".") != 0) return -1;
    if (procesar_declaracion_variable_entera(linea, linea_actual) < 0) return -1;
    ctx->linea_num++; return 0;
}

int cmd_var_decimal(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (check_declaracion(ctx, linea_actual, ".") != 0) return -1;
    if (procesar_declaracion_variable_decimal(linea, linea_actual) < 0) return -1;
    ctx->linea_num++; return 0;
}

int cmd_var_texto(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (check_declaracion(ctx, linea_actual, ".") != 0) return -1;
    if (procesar_declaracion_variable_texto(linea, linea_actual) < 0) return -1;
    ctx->linea_num++; return 0;
}

int cmd_var_texto_extenso(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (check_declaracion(ctx, linea_actual, ".") != 0) return -1;
    if (procesar_declaracion_variable_texto_extenso(linea, linea_actual) < 0) return -1;
    ctx->linea_num++; return 0;
}

int cmd_var_caracter(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (check_declaracion(ctx, linea_actual, ".") != 0) return -1;
    if (procesar_declaracion_variable_caracter(linea, linea_actual) < 0) return -1;
    ctx->linea_num++; return 0;
}

int cmd_var_entera_sin(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (check_declaracion(ctx, linea_actual, ": Declaraciones solo al inicio.") != 0) return -1;
    if (procesar_declaracion_variable_entera_sin_signo(linea, linea_actual) < 0) return -1;
    ctx->linea_num++; return 0;
}

int cmd_var_decimal_sin(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (check_declaracion(ctx, linea_actual, ": Declaraciones solo al inicio.") != 0) return -1;
    if (procesar_declaracion_variable_decimal_sin_signo(linea, linea_actual) < 0) return -1;
    ctx->linea_num++; return 0;
}

int cmd_var_caracter_sin(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (check_declaracion(ctx, linea_actual, ": Declaraciones solo al inicio.") != 0) return -1;
    if (procesar_declaracion_variable_caracter_sin_signo(linea, linea_actual) < 0) return -1;
    ctx->linea_num++; return 0;
}

int cmd_constante_entera(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (!ctx->declaraciones_permitidas) {
        fprintf(stderr, "Error línea %d.\n", linea_actual);
        return -1;
    }
    if (procesar_declaracion_constante_entera(linea, linea_actual) < 0) return -1;
    ctx->linea_num++; return 0;
}

int cmd_constante_decimal(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (!ctx->declaraciones_permitidas) {
        fprintf(stderr, "Error línea %d.\n", linea_actual);
        return -1;
    }
    if (procesar_declaracion_constante_decimal(linea, linea_actual) < 0) return -1;
    ctx->linea_num++; return 0;
}

int cmd_constante_texto(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (!ctx->declaraciones_permitidas) {
        fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
        return -1;
    }
    if (procesar_declaracion_constante_texto(linea, linea_actual) < 0) return -1;
    ctx->linea_num++; return 0;
}

int cmd_constante_entera_sin(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (!ctx->declaraciones_permitidas) {
        fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
        return -1;
    }
    if (procesar_declaracion_constante_entera_sin_signo(linea, linea_actual) < 0) return -1;
    ctx->linea_num++; return 0;
}

int cmd_constante_decimal_sin(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (!ctx->declaraciones_permitidas) {
        fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
        return -1;
    }
    if (procesar_declaracion_constante_decimal_sin_signo(linea, linea_actual) < 0) return -1;
    ctx->linea_num++; return 0;
}

int cmd_constante_caracter(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (!ctx->declaraciones_permitidas) {
        fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
        return -1;
    }
    if (procesar_declaracion_constante_caracter(linea, linea_actual) < 0) return -1;
    ctx->linea_num++; return 0;
}

int cmd_constante_caracter_sin(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (!ctx->declaraciones_permitidas) {
        fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual);
        return -1;
    }
    if (procesar_declaracion_constante_caracter_sin_signo(linea, linea_actual) < 0) return -1;
    ctx->linea_num++; return 0;
}

int cmd_llamar_a(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (!ctx->en_bloque_principal && !ctx->en_subprograma && !ctx->en_funcion) {
        fprintf(stderr, "Error línea %d: LLAMAR A solo permitido dentro de BLOQUE PRINCIPAL, SUBPROGRAMA o FUNCION.\n", linea_actual);
        return -1;
    }

    const char *ptr = linea + 8;
    while (*ptr == ' ' || *ptr == '\t') ptr++;

    char nombre[MAX_NOMBRE];
    int j = 0;
    while (*ptr && *ptr != '(' && !isspace((unsigned char)*ptr) && j < MAX_NOMBRE - 1)
        nombre[j++] = *ptr++;
    nombre[j] = '\0';

    if (strlen(nombre) == 0) {
        fprintf(stderr, "Error línea %d: LLAMAR A requiere nombre.\n", linea_actual);
        ctx->linea_num++;
        return 0;
    }

    if (*ptr == '(') {
        const char *inicio = ptr;
        int nivel = 1; ptr++;
        while (*ptr && nivel > 0) {
            if (*ptr == '(') nivel++;
            else if (*ptr == ')') nivel--;
            ptr++;
        }
        char expr[MAX_LINEA] = "";
        strncpy(expr, nombre, MAX_NOMBRE - 1);
        expr[MAX_NOMBRE - 1] = '\0';
        int len = (int)(ptr - inicio);
        if (len > 0 && len < MAX_LINEA) strncat(expr, inicio, len);
        
        int exito = 0;
        evaluar_expresion_completa(expr, &exito);
        if (!exito) {
            fprintf(stderr, "Error línea %d: No se pudo ejecutar función '$%s'.\n", linea_actual, nombre);
            ctx->error_fatal = 1;
            return -1;
        }
        ctx->linea_num++;
        return 0;
    }

    char *args[MAX_PARAMETROS];
    int num_args = 0;
    if (*ptr == '(') {
        ptr++;
        while (*ptr && *ptr != ')' && num_args < MAX_PARAMETROS) {
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            char arg[MAX_LINEA] = "";
            j = 0;
            int nivel_p = 0;
            while (*ptr && (nivel_p > 0 || (*ptr != ',' && *ptr != ')')) && j < MAX_LINEA - 1) {
                if (*ptr == '(') nivel_p++;
                else if (*ptr == ')') nivel_p--;
                arg[j++] = *ptr++;
            }
            arg[j] = '\0';
            limpiar_string(arg);
            if (strlen(arg) > 0) args[num_args++] = strdup(arg);
            if (*ptr == ',') ptr++;
        }
    }

    int sub_idx = buscar_subprograma_info(nombre);
    if (sub_idx == -1) {
        fprintf(stderr, "Error línea %d: Subprograma '$%s' no declarado.\n", linea_actual, nombre);
        for (int k = 0; k < num_args; k++) free(args[k]);
        ctx->linea_num++; return 0;
    }
    if (num_args != subprogramas_registrados[sub_idx].num_params) {
        fprintf(stderr, "Error línea %d: Subprograma '$%s' espera %d parametros, se recibieron %d.\n",
                linea_actual, nombre, subprogramas_registrados[sub_idx].num_params, num_args);
        for (int k = 0; k < num_args; k++) free(args[k]);
        ctx->linea_num++; return 0;
    }
    if (sub_stack_ptr >= MAX_NESTING) {
        fprintf(stderr, "Error línea %d: Anidamiento de subprogramas excedido.\n", linea_actual);
        for (int k = 0; k < num_args; k++) free(args[k]);
        ctx->linea_num++; return 0;
    }

    asignar_argumentos_a_parametros(
        subprogramas_registrados[sub_idx].params,
        subprogramas_registrados[sub_idx].num_params,
        args, num_args
    );
    for (int k = 0; k < num_args; k++) free(args[k]);

    sub_stack[sub_stack_ptr].linea_retorno = ctx->linea_num + 1;
    sub_stack_ptr++;
    ctx->en_subprograma = 1;
    ctx->linea_num = subprogramas_registrados[sub_idx].linea_inicio + 1;
    return 0;
}

int cmd_subprograma(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea; (void)linea_actual;
    if (ctx->fase_declaraciones || ctx->en_bloque_principal) {
        int fin = encontrar_fin_subprograma(ctx->linea_num);
        if (fin != -1) {
            ctx->linea_num = fin + 1;
        } else {
            fprintf(stderr, "Error línea %d: SUBPROGRAMA sin FIN SUBPROGRAMA.\n", linea_actual);
            return -1;
        }
    } else {
        ctx->en_subprograma = 1;
        ctx->linea_num++;
    }
    return 0;
}

int cmd_fin_subprograma(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea; (void)linea_actual; 
    ctx->en_subprograma = 0;
    if (sub_stack_ptr > 0) {
        sub_stack_ptr--;
        ctx->linea_num = sub_stack[sub_stack_ptr].linea_retorno;
    } else {
        ctx->linea_num++;
    }
    return 0;
}

static const char* encontrar_parentesis_cierre(const char *apertura) {
    int nivel = 1;
    const char *p = apertura + 1;
    while (*p) {
        if (*p == '(') nivel++;
        else if (*p == ')') {
            nivel--;
            if (nivel == 0) return p;
        }
        p++;
    }
    return NULL;
}

int cmd_si(const char *linea, CtxBloque *ctx, int linea_actual) {
    const char* abre = strchr(linea, '(');
    const char* cierra = strrchr(linea, ')');
    char expr[MAX_LINEA] = "";
    if (abre && cierra && cierra > abre) {
        size_t len = cierra - abre - 1;
        if (len < MAX_LINEA) strncpy(expr, abre + 1, len), expr[len] = '\0';
        char* e = strstr(expr, "ENTONCES"); if (e) *e = '\0';
        size_t j = strlen(expr);
        while (j > 0 && (expr[j-1] == ' ' || expr[j-1] == '\t')) expr[--j] = '\0';
    }
    int exito;
    int eval = evaluar_condicion(expr, &exito);
    if (!exito) eval = 0;
    if (si_runtime_ptr >= MAX_SI_RUNTIME - 1) return -1;
    si_runtime_stack[si_runtime_ptr++] = eval;
    if (!eval) {
        int destino = saltar_hasta_rama_o_fin_si(linea_actual - 1);
        if (destino != -1) { ctx->linea_num = destino; return 0; }
        return -1;
    }
    return 0;
}

int cmd_sino_si(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (si_runtime_ptr == 0) return -1;
    int cima = si_runtime_ptr - 1;
    if (si_runtime_stack[cima] == 1) {
        int destino = saltar_hasta_rama_o_fin_si(linea_actual - 1);
        if (destino != -1) ctx->linea_num = destino;
        return 0;
    }
    const char* abre = strchr(linea, '(');
    const char* cierra = strrchr(linea, ')');
    char expr[MAX_LINEA] = "";
    if (abre && cierra && cierra > abre) {
        size_t len = cierra - abre - 1;
        if (len < MAX_LINEA) strncpy(expr, abre + 1, len), expr[len] = '\0';
        char* e = strstr(expr, "ENTONCES"); if (e) *e = '\0';
        size_t j = strlen(expr);
        while (j > 0 && (expr[j-1] == ' ' || expr[j-1] == '\t')) expr[--j] = '\0';
    }
    int exito;
    int eval = evaluar_condicion(expr, &exito);
    if (!exito) eval = 0;
    si_runtime_stack[cima] = eval;
    if (!eval) {
        int destino = saltar_hasta_rama_o_fin_si(linea_actual - 1);
        if (destino != -1) ctx->linea_num = destino;
    }
    return 0;
}

int cmd_sino(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea;
    if (si_runtime_ptr == 0) return -1;
    int cima = si_runtime_ptr - 1;
    if (si_runtime_stack[cima] == 1) {
        int destino = encontrar_fin_si(linea_actual - 1);
        if (destino != -1) ctx->linea_num = destino;
        return 0;
    }
    si_runtime_stack[cima] = 1;
    return 0;
}

int cmd_fin_si(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea; (void)ctx; (void)linea_actual;
    if (si_runtime_ptr > 0) {
        si_runtime_ptr--;
    }
    return 0;
}

int cmd_fin_segun(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea; (void)linea_actual;
    if (segun_stack_ptr > 0) segun_stack_ptr--;
    ctx->linea_num++;
    return 0;
}

int cmd_segun_caso(const char *linea, CtxBloque *ctx, int linea_actual) {
    int exito;
    double valor = 0;
    const char *ptr = linea + 10; 
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr != '(') {
        fprintf(stderr, "Error línea %d: SEGUN CASO requiere paréntesis: SEGUN CASO ($variable).\n", linea_actual);
        int fin = encontrar_fin_segun(ctx->linea_num);
        if (fin != -1) ctx->linea_num = fin + 1;
        else ctx->linea_num++;
        return 0;
    }
    char *inicio = strchr(linea, '(');
    char *fin = strchr(linea, ')');
    if (inicio && fin) {
        char expresion[MAX_LINEA];
        int len = fin - inicio - 1;
        if (len > 0 && len < MAX_LINEA) {
            strncpy(expresion, inicio + 1, len);
            expresion[len] = '\0';
            valor = evaluar_expresion_completa(expresion, &exito);
        }
    }
    if (segun_stack_ptr < MAX_NESTING) {
        segun_stack[segun_stack_ptr].linea_inicio = ctx->linea_num;
        segun_stack[segun_stack_ptr].linea_fin = encontrar_fin_segun(ctx->linea_num);
        segun_stack[segun_stack_ptr].valor_variable = (int)valor;
        segun_stack[segun_stack_ptr].caso_encontrado = 0;
        segun_stack_ptr++;
    }
    ctx->linea_num++;
    return 0;
}

int cmd_caso(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea_actual;
    if (segun_stack_ptr <= 0) { ctx->linea_num++; return 0; }
    
    SegunBloque *bloque = &segun_stack[segun_stack_ptr - 1];
    const char *ptr = linea + 4;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    int valor_caso = atoi(ptr);

    if (bloque->caso_encontrado) {
        int level = 0;
        int i = ctx->linea_num + 1;
        while (i < num_lineas_programa) {
            char l[MAX_LINEA]; strncpy(l, lineas_programa[i], MAX_LINEA-1); l[MAX_LINEA-1]='\0';
            limpiar_string(l); remover_comentario(l);
            
            if (comienza_con(l, "SEGUN CASO")) { level++; i++; continue; }
            if (comienza_con(l, "FIN SEGUN")) {
                if (level == 0) { ctx->linea_num = i; return 0; }
                level--; i++; continue;
            }
            
            if (level == 0 && (comienza_con(l, "CASO") || comienza_con(l, "POR DEFECTO"))) {
                ctx->linea_num = i; return 0;
            }
            i++;
        }
        ctx->linea_num = bloque->linea_fin;
        return 0;
    }

    if (bloque->valor_variable == valor_caso) {
        bloque->caso_encontrado = 1;
        ctx->linea_num++;
        return 0;
    }

    int level = 0;
    int i = ctx->linea_num + 1;
    while (i < num_lineas_programa) {
        char l[MAX_LINEA]; strncpy(l, lineas_programa[i], MAX_LINEA-1); l[MAX_LINEA-1]='\0';
        limpiar_string(l); remover_comentario(l);
        
        if (comienza_con(l, "SEGUN CASO")) { level++; i++; continue; }
        if (comienza_con(l, "FIN SEGUN")) {
            if (level == 0) { ctx->linea_num = i; return 0; }
            level--; i++; continue;
        }
        
        if (level == 0 && (comienza_con(l, "CASO") || comienza_con(l, "POR DEFECTO"))) {
            ctx->linea_num = i; return 0;
        }
        i++;
    }
    ctx->linea_num = bloque->linea_fin;
    return 0;
}

int cmd_por_defecto(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea; (void)linea_actual;
    if (segun_stack_ptr <= 0) { ctx->linea_num++; return 0; }
    
    SegunBloque *bloque = &segun_stack[segun_stack_ptr - 1];
    
    if (bloque->caso_encontrado) {
        int level = 0;
        int i = ctx->linea_num + 1;
        while (i < num_lineas_programa) {
            char l[MAX_LINEA]; strncpy(l, lineas_programa[i], MAX_LINEA-1); l[MAX_LINEA-1]='\0';
            limpiar_string(l); remover_comentario(l);
            
            if (comienza_con(l, "SEGUN CASO")) { level++; i++; continue; }
            if (comienza_con(l, "FIN SEGUN")) {
                if (level == 0) { ctx->linea_num = i; return 0; }
                level--; i++; continue;
            }
            i++;
        }
        ctx->linea_num = bloque->linea_fin;
        return 0;
    }
    
    bloque->caso_encontrado = 1;
    ctx->linea_num++;
    return 0;
}

int cmd_funcion(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea;
    if (ctx->fase_declaraciones && !ctx->en_bloque_principal) {
        int fin = encontrar_fin_funcion(ctx->linea_num);
        if (fin != -1) ctx->linea_num = fin + 1;
        else {
            fprintf(stderr, "Error línea %d: FUNCION sin FIN FUNCION.\n", linea_actual);
            ctx->linea_num++;
        }
    } else {
        ctx->linea_num++;
    }
    return 0;
}

int cmd_fin_funcion(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea;
    if (si_stack_ptr != 0) {
        fprintf(stderr, "Error: Hay %d bloque(s) 'SI' sin cerrar al llegar a FIN FUNCION (línea %d).\n", si_stack_ptr, linea_actual);
        if (modo_estricto) exit(1);
        si_stack_ptr = 0;
    }

    if (funcion_stack_ptr > 0) {
        asignar_valor_retorno_funcion();
        funcion_stack_ptr--;
        ctx->linea_num = funcion_stack[funcion_stack_ptr].linea_retorno;
        ctx->en_funcion = 0;
        
        if (scope_actual >= 0) {
            eliminar_scope_local();
        }
        
        funcion_destino_stack_ptr--;
        if (funcion_destino_stack_ptr >= 0) {
            strncpy(funcion_variable_destino, funcion_destino_stack[funcion_destino_stack_ptr], MAX_NOMBRE - 1);
            funcion_variable_destino[MAX_NOMBRE - 1] = '\0';
        } else {
            funcion_variable_destino[0] = '\0';
        }
        ctx->linea_num++; 
    } else {
        fprintf(stderr, "Error línea %d: FIN FUNCION sin llamada\n", linea_actual);
        ctx->linea_num++;
    }
    return 0;
}

int cmd_retornar(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea_actual;

    // 1. Validar que estamos en una función
    if (!ctx->en_funcion) {
        int fin_func = encontrar_fin_funcion(ctx->linea_num);
        if (fin_func != -1) {
            ctx->linea_num = fin_func + 1;
        } else {
            ctx->linea_num = num_lineas_programa;
        }
        return 0;
    }

    // 2. Extraer la expresión a retornar
    const char *ptr = linea + 8; // Saltar "RETORNAR"
    while (*ptr == ' ' || *ptr == '\t') ptr++;

    // 3. Evaluar la expresión (si existe)
    if (strlen(ptr) > 0) {
        int exito;
        double resultado = evaluar_expresion_completa(ptr, &exito);
        if (exito) {
            valor_retorno_funcion = resultado;
            hay_valor_retorno = 1;
        } else {
            // Si falla la evaluación, retornar 0 por seguridad
            valor_retorno_funcion = 0.0;
            hay_valor_retorno = 1;
        }
    } else {
        // RETORNAR sin valor
        valor_retorno_funcion = 0.0;
        hay_valor_retorno = 1;
    }

    // 4. Limpiar la pila de SI al salir de la función
    si_runtime_ptr = ctx->si_ptr_inicio;

    // 5. Forzar la salida del bucle de la función
    ctx->linea_num = ctx->linea_limite + 1;
    
    return 0;
}

static inline int handle_declaracion(const char *linea, CtxBloque *ctx, int linea_actual, 
                                     int (*procesar)(const char *, int), const char *msg) {
    if (!ctx->declaraciones_permitidas) {
        fprintf(stderr, "Error línea %d%s\n", linea_actual, msg);
        return -1;
    }
    if (procesar(linea, linea_actual) < 0) return -1;
    ctx->linea_num++;
    return 0;
}

int cmd_lista_entera(const char *l, CtxBloque *c, int la) { return handle_declaracion(l, c, la, procesar_declaracion_lista_entera, "."); }
int cmd_lista_decimal(const char *l, CtxBloque *c, int la) { return handle_declaracion(l, c, la, procesar_declaracion_lista_decimal, "."); }
int cmd_lista_caracter(const char *l, CtxBloque *c, int la) { return handle_declaracion(l, c, la, procesar_declaracion_lista_caracter, ": Declaraciones solo al inicio."); }
int cmd_lista_entera_sin(const char *l, CtxBloque *c, int la) { return handle_declaracion(l, c, la, procesar_declaracion_lista_entera_sin_signo, ": Declaraciones solo al inicio."); }
int cmd_lista_decimal_sin(const char *l, CtxBloque *c, int la) { return handle_declaracion(l, c, la, procesar_declaracion_lista_decimal_sin_signo, ": Declaraciones solo al inicio."); }
int cmd_lista_caracter_sin(const char *l, CtxBloque *c, int la) { return handle_declaracion(l, c, la, procesar_declaracion_lista_caracter_sin_signo, ": Declaraciones solo al inicio."); }

int cmd_matriz_entera(const char *l, CtxBloque *c, int la) { return handle_declaracion(l, c, la, procesar_declaracion_matriz_entera, ": Declaraciones solo al inicio."); }
int cmd_matriz_decimal(const char *l, CtxBloque *c, int la) { return handle_declaracion(l, c, la, procesar_declaracion_matriz_decimal, ": Declaraciones solo al inicio."); }
int cmd_matriz_caracter(const char *l, CtxBloque *c, int la) { return handle_declaracion(l, c, la, procesar_declaracion_matriz_caracter, ": Declaraciones solo al inicio."); }
int cmd_matriz_entera_sin(const char *l, CtxBloque *c, int la) { return handle_declaracion(l, c, la, procesar_declaracion_matriz_entera_sin_signo, ": Declaraciones solo al inicio."); }
int cmd_matriz_decimal_sin(const char *l, CtxBloque *c, int la) { return handle_declaracion(l, c, la, procesar_declaracion_matriz_decimal_sin_signo, ": Declaraciones solo al inicio."); }
int cmd_matriz_caracter_sin(const char *l, CtxBloque *c, int la) { return handle_declaracion(l, c, la, procesar_declaracion_matriz_caracter_sin_signo, ": Declaraciones solo al inicio."); }

int cmd_var_archivo(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (!ctx->declaraciones_permitidas) {
        fprintf(stderr, "Error línea %d: Declaraciones solo al inicio.\n", linea_actual); return -1;
    }
    if (procesar_declaracion_variable_archivo(linea, linea_actual) < 0) return -1;
    ctx->linea_num++; return 0;
}

int cmd_fin_para(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea;
    if (para_stack_ptr > 0) {
        ParaBloque *bloque = &para_stack[para_stack_ptr - 1];
        int valor = get_var_valor_global(bloque->var_nombre);
        valor += bloque->paso;
        set_var_valor_global(bloque->var_nombre, valor);
        int continuar = (bloque->paso > 0) ? (valor <= bloque->fin) : (valor >= bloque->fin);
        if (continuar) ctx->linea_num = bloque->linea_inicio + 1;
        else { para_stack_ptr--; ctx->linea_num++; }
    } else {
        fprintf(stderr, "Error línea %d: FIN PARA sin PARA.\n", linea_actual);
        ctx->linea_num++;
    }
    return 0;
}

int cmd_fin_mientras(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea; (void)linea_actual;
    int nivel = 0, linea_mientras = ctx->linea_num - 1, while_line = -1;
    
    while (linea_mientras >= 0) {
        char lp[MAX_LINEA]; 
        strncpy(lp, lineas_programa[linea_mientras], MAX_LINEA-1); 
        lp[MAX_LINEA-1]='\0';
        limpiar_string(lp); 
        remover_comentario(lp);
        
        if (strncmp(lp, "FIN MIENTRAS", 12) == 0) nivel++;
        else if (strncmp(lp, "MIENTRAS", 8) == 0 && strstr(lp, "HACER")) { 
            if (nivel == 0) { while_line = linea_mientras; break; } 
            nivel--; 
        }
        linea_mientras--;
    }
    
    if (while_line != -1) {
        const char *src = lineas_programa[while_line];
        
        const char *pa = strchr(src, '(');
        const char *pc = pa ? encontrar_parentesis_cierre(pa) : NULL;
        
        char cond[MAX_LINEA] = "";
        if (pa && pc && pc > pa) {
            int len = (int)(pc - pa - 1);
            if (len > 0 && len < MAX_LINEA - 1) {
                memcpy(cond, pa + 1, len);
                cond[len] = '\0';
            }
        }
        
        char *ini = cond; 
        while (*ini == ' ' || *ini == '\t') ini++; 
        char *fin = ini + strlen(ini) - 1;
        while (fin > ini && (*fin == ' ' || *fin == '\t' || *fin == '\r')) { 
            *fin = '\0'; fin--; 
        }
        if (ini != cond) memmove(cond, ini, strlen(ini) + 1);
        
        char ce[MAX_LINEA + 2]; 
        ce[0] = ' '; 
        strcpy(ce + 1, cond);
        int exito = 0, res = evaluar_condicion(ce, &exito);
        
        if (exito && res) {
            ctx->linea_num = while_line + 1;
        } else {
            if (mientras_stack_ptr > 0) mientras_stack_ptr--;
            ctx->linea_num++;
        }
    } else {
        if (mientras_stack_ptr > 0) mientras_stack_ptr--;
        ctx->linea_num++;
    }
    return 0;
}

int cmd_saltar_a(const char *linea, CtxBloque *ctx, int linea_actual) {
    const char *ptr = linea + 8; while (*ptr == ' ' || *ptr == '\t') ptr++;
    char nom[MAX_NOMBRE]; int j=0; while (*ptr && !isspace((unsigned char)*ptr) && j<MAX_NOMBRE-1) nom[j++] = *ptr++; nom[j]='\0';
    int ld = ejecutar_salto_a_etiqueta(nom, linea_actual);
    ctx->linea_num = (ld == -1) ? ctx->linea_num + 1 : ld;
    return 0;
}

int cmd_para(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (!strstr(linea, "HACER")) {
        ctx->linea_num++;
        return 0;
    }

    const char *p = linea + 4;
    while (*p == ' ') p++;
    int nivel = 0;

    if (*p == '{') {
        p++;
        nivel = atoi(p);
        while (*p && *p != '}') p++;
        if (*p == '}') p++;
    }
    while (*p == ' ') p++;

    if (*p != '(') {
        fprintf(stderr, "Error línea %d: PARA requiere paréntesis.\n", linea_actual);
        int fp = encontrar_fin_para(ctx->linea_num, nivel);
        ctx->linea_num = (fp != -1) ? fp + 1 : ctx->linea_num + 1;
        return 0;
    }
    p++;
    while (*p == ' ') p++;

    if (*p != '$') {
        fprintf(stderr, "Error línea %d: PARA requiere variable $var.\n", linea_actual);
        int fp = encontrar_fin_para(ctx->linea_num, nivel);
        ctx->linea_num = (fp != -1) ? fp + 1 : ctx->linea_num + 1;
        return 0;
    }
    p++;

    char vn[MAX_NOMBRE] = "";
    int j = 0;
    while (es_alnum(*p) && j < MAX_NOMBRE - 1) { vn[j++] = *p++; }
    vn[j] = '\0';
    while (*p == ' ') p++;

    // PARSEAR DESDE (inicialización)
    int skip_op = 0;
    if (*p == '=') { skip_op = 1; }
    else if (strncmp(p, "DESDE", 5) == 0 && (p[5] == ' ' || p[5] == '\t' || p[5] == '\0' || p[5] == ')')) { skip_op = 5; }

    int vi = 0;
    (void) vi;
    // PARSEAR DESDE (con extracción limpia)
    if (skip_op > 0) {
        p += skip_op;
        while (*p == ' ') p++;
    
        // Extraer expresión hasta HASTA
        char expr_desde[MAX_LINEA] = "";
        int k = 0;
        int paren_depth = 0;
        const char *t = p;
        while (*t && k < MAX_LINEA - 2) {
            if (paren_depth == 0 && strncmp(t, "HASTA", 5) == 0) break;
            if (*t == '(') paren_depth++;
            else if (*t == ')') { paren_depth--; if (paren_depth < 0) break; }
            expr_desde[k++] = *t++;
        }
        expr_desde[k] = '\0';
        while (k > 0 && (expr_desde[k-1] == ' ' || expr_desde[k-1] == '\t' || expr_desde[k-1] == ')')) expr_desde[--k] = '\0';
    
        int exito_vi;
        double val_vi = evaluar_expresion_completa(expr_desde, &exito_vi);
        int vi = exito_vi ? (int)val_vi : 0; 
        set_var_valor_global(vn, vi);
    }

    // PARSEAR HASTA (límite)
    const char *hp = strstr(linea, "HASTA");
    if (!hp) {
        fprintf(stderr, "Error línea %d: PARA requiere HASTA.\n", linea_actual);
        int fp = encontrar_fin_para(ctx->linea_num, nivel);
        ctx->linea_num = (fp != -1) ? fp + 1 : ctx->linea_num + 1;
        return 0;
    }
    const char *start_expr = hp + 5;
    while (*start_expr == ' ' || *start_expr == '\t') start_expr++;

    // Extraer expresión respetando paréntesis anidados
    char expr_hasta[MAX_LINEA] = "";
    int k = 0;
    int paren_depth = 0;
    const char *t = start_expr;
    
    while (*t && k < MAX_LINEA - 2) {
        // Detener en keywords solo si no estamos dentro de paréntesis
        if (paren_depth == 0) {
            if (strncmp(t, "HACER", 5) == 0 || strncmp(t, "PASO", 4) == 0) break;
        }
        if (*t == '(') paren_depth++;
        else if (*t == ')') {
            paren_depth--;
            if (paren_depth < 0) break; // Cierre que no abre: fin de expresión
        }
        expr_hasta[k++] = *t++;
    }
    expr_hasta[k] = '\0';
    
    // Limpiar espacios/p paréntesis colgados al final
    while (k > 0 && (expr_hasta[k-1] == ' ' || expr_hasta[k-1] == '\t' || expr_hasta[k-1] == ')')) {
        expr_hasta[--k] = '\0';
    }

    int exito_fin = 0;
    double val_fin = evaluar_expresion_completa(expr_hasta, &exito_fin);
    int fin = exito_fin ? (int)val_fin : 0;
    if (!exito_fin) {
        fprintf(stderr, "Error línea %d: No se pudo evaluar HASTA '%s'.\n", linea_actual, expr_hasta);
        int fp = encontrar_fin_para(ctx->linea_num, nivel);
        ctx->linea_num = (fp != -1) ? fp + 1 : ctx->linea_num + 1;
        return 0;
    }

    // PARSEAR PASO (opcional)
    int paso = 1;
    const char *pptr = strstr(linea, "PASO");
    if (pptr) {
        const char *start_paso = pptr + 4;
        while (*start_paso == ' ' || *start_paso == '\t') start_paso++;
        char expr_paso[MAX_LINEA] = "";
        k = 0;
        t = start_paso;
        while (*t && k < MAX_LINEA - 2) {
            if (strncmp(t, "HACER", 5) == 0 || *t == ')') break;
            expr_paso[k++] = *t++;
        }
        expr_paso[k] = '\0';
        int exito_paso = 0;
        double val_paso = evaluar_expresion_completa(expr_paso, &exito_paso);
        paso = exito_paso ? (int)val_paso : 1;
        if (paso == 0) paso = 1;
    }

    // VALIDAR DIRECCIÓN Y PUSH STACK
    int valor = get_var_valor_global(vn);
    int dir_ok = !((paso > 0 && valor > fin) || (paso < 0 && valor < fin));

    if (dir_ok && para_stack_ptr < MAX_NESTING) {
        para_stack[para_stack_ptr].nivel = nivel;
        strncpy(para_stack[para_stack_ptr].var_nombre, vn, MAX_NOMBRE - 1);
        para_stack[para_stack_ptr].var_nombre[MAX_NOMBRE - 1] = '\0';
        para_stack[para_stack_ptr].fin = fin;
        para_stack[para_stack_ptr].paso = paso;
        para_stack[para_stack_ptr].linea_inicio = ctx->linea_num;
        para_stack[para_stack_ptr].linea_fin = encontrar_fin_para(ctx->linea_num, nivel);
        para_stack_ptr++;
        ctx->linea_num++;
    } else {
        int fp = encontrar_fin_para(ctx->linea_num, nivel);
        ctx->linea_num = (fp != -1) ? fp + 1 : ctx->linea_num + 1;
    }
    return 0;
}

int cmd_realizar(const char *linea, CtxBloque *ctx, int linea_actual) {
    int nivel=0; const char *p=linea+8; while(*p==' ')p++; if (*p=='{') { p++; nivel=atoi(p); while(*p&&*p!='}')p++; if (*p=='}')p++; }
    int ya=0; for(int i=0;i<proceder_stack_ptr;i++) if (proceder_stack[i].nivel==nivel && proceder_stack[i].linea_inicio==ctx->linea_num) { ya=1; break; }
    int fp=encontrar_fin_proceder(ctx->linea_num, nivel); if (fp==-1) { fprintf(stderr, "Error línea %d: REALIZAR sin MIENTRAS.\n", linea_actual); ctx->linea_num++; return 0; }
    if (!ya && proceder_stack_ptr<MAX_NESTING) { proceder_stack[proceder_stack_ptr].linea_inicio=ctx->linea_num; proceder_stack[proceder_stack_ptr].linea_fin=fp; proceder_stack[proceder_stack_ptr].nivel=nivel; proceder_stack_ptr++; }
    ctx->linea_num++; return 0;
}

int cmd_mientras(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (strchr(linea, '(') && proceder_stack_ptr > 0) {
        int nivel=0; const char *p=linea+8; while(*p==' ')p++; if (*p=='{') { p++; nivel=atoi(p); }
        int exito, res; char cond[MAX_LINEA]="";
        const char *pa=strchr(linea,'('), *pc=pa?encontrar_parentesis_cierre(pa):NULL;
        if (pa&&pc&&pc>pa) { int len=(int)(pc-pa-1); if(len>0&&len<MAX_LINEA-1){memcpy(cond,pa+1,len);cond[len]='\0';} }
        char *ini=cond; while(*ini==' '||*ini=='\t')ini++; char *fin=ini+strlen(ini)-1; while(fin>ini&&(*fin==' '||*fin=='\t'||*fin=='\r')){*fin='\0';fin--;} if(ini!=cond)memmove(cond,ini,strlen(ini)+1);
        char ce[MAX_LINEA+2]; ce[0]=' '; strcpy(ce+1,cond); res=evaluar_condicion(ce,&exito);
        if (!exito) { fprintf(stderr, "Error línea %d.\n", linea_actual); }
        if (exito && res) { for(int i=proceder_stack_ptr-1;i>=0;i--) if(proceder_stack[i].nivel==nivel){ctx->linea_num=proceder_stack[i].linea_inicio+1;break;} }
        else { for(int i=proceder_stack_ptr-1;i>=0;i--) if(proceder_stack[i].nivel==nivel){ for(int j=i;j<proceder_stack_ptr-1;j++)proceder_stack[j]=proceder_stack[j+1]; proceder_stack_ptr--; break; } ctx->linea_num++; }
        return 0;
    }

    if (strstr(linea, "HACER")) {
        if (mientras_stack_ptr < MAX_NESTING) { mientras_stack[mientras_stack_ptr].linea_inicio=ctx->linea_num; mientras_stack[mientras_stack_ptr].linea_fin=encontrar_fin_mientras(ctx->linea_num,0); mientras_stack_ptr++; }
        int exito=0,res=0; char cond[MAX_LINEA]="";
        const char *pa=strchr(linea,'('), *pc=pa?encontrar_parentesis_cierre(pa):NULL;
        if (pa&&pc&&pc>pa) { int len=(int)(pc-pa-1); if(len>0&&len<MAX_LINEA-1){memcpy(cond,pa+1,len);cond[len]='\0';} }
        limpiar_string(cond); char ce[MAX_LINEA+2]; ce[0]=' '; strcpy(ce+1,cond); res=evaluar_condicion(ce,&exito);
        if (!exito) { fprintf(stderr, "Error línea %d: Condición inválida.\n", linea_actual); if(mientras_stack_ptr>0)mientras_stack_ptr--; int fin=encontrar_fin_mientras(ctx->linea_num,0); ctx->linea_num=(fin!=-1)?fin+1:ctx->linea_num+1; return 0; }
        if (res) ctx->linea_num++; else { int fin=encontrar_fin_mientras(ctx->linea_num,0); ctx->linea_num=(fin!=-1)?fin+1:ctx->linea_num+1; if(mientras_stack_ptr>0)mientras_stack_ptr--; }
        return 0;
    }
    ctx->linea_num++; return 0;
}

int cmd_corte(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea; int realizado=0;
    if (segun_stack_ptr>0) { int fs=segun_stack[segun_stack_ptr-1].linea_fin; segun_stack_ptr--; if(fs!=-1){ctx->linea_num=fs+1;realizado=1;} }
    if (!realizado && mientras_stack_ptr>0) { int fm=mientras_stack[mientras_stack_ptr-1].linea_fin; if(fm!=-1){ctx->linea_num=fm+1;mientras_stack_ptr--;realizado=1;} }
    if (!realizado && para_stack_ptr>0) { para_stack_ptr--; int fp=para_stack[para_stack_ptr].linea_fin; if(fp!=-1){ctx->linea_num=fp+1;realizado=1;} }
    if (!realizado && proceder_stack_ptr>0) { int fpr=proceder_stack[proceder_stack_ptr-1].linea_fin; proceder_stack_ptr--; if(fpr!=-1){ctx->linea_num=fpr+1;realizado=1;} }
    if (!realizado && ctx->en_subprograma && sub_stack_ptr>0) {
        int nivel=0; for(int b=ctx->linea_num+1;b<num_lineas_programa;b++) { char lb[MAX_LINEA]; strncpy(lb,lineas_programa[b],MAX_LINEA-1); lb[MAX_LINEA-1]='\0'; limpiar_string(lb); remover_comentario(lb);
            if (comienza_con(lb,"FIN SUBPROGRAMA")) { if(nivel==0){ctx->linea_num=b;realizado=1;break;} nivel--; } else if (comienza_con(lb,"SUBPROGRAMA")) nivel++; }
    }
    if (!realizado) { fprintf(stderr, "Error línea %d: CORTE fuera de un bucle, SEGUN o SUBPROGRAMA.\nCORTE válido en: MIENTRAS, PARA, REALIZAR, SEGUN CASO, SUBPROGRAMA.\n", linea_actual); ctx->linea_num++; }
    return 0;
}

int cmd_configurar_pin(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea_actual;
    procesar_gpio_configurar(linea + 13);
    ctx->linea_num++;
    return 0;
}

int cmd_estado_pin(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea_actual;
    procesar_gpio_estado_pin(linea + 9);
    ctx->linea_num++;
    return 0;
}

int cmd_leer_pin(const char *linea, CtxBloque *ctx, int linea_actual) {
    (void)linea_actual;
    procesar_gpio_leer(linea + 7);
    ctx->linea_num++;
    return 0;
}

void handler_sigint(int sig) {
    (void)sig;
    extern void restaurar_terminal_completa(void);
    restaurar_terminal_completa();
    printf("\n> Terminal restaurada. Programa interrumpido.\n");
    fflush(stdout);
    exit(0);
}

int main(int argc, char *argv[]) {
    pila_retorno.tope = 0;
    signal(SIGINT, handler_sigint);   // Ctrl+C
    signal(SIGTERM, handler_sigint);  // Kill

#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
        SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif

    if (argc >= 3 && strcmp(argv[1], "-e") == 0) {
        int exito = 0;
        double resultado = evaluar_expresion_completa(argv[2], &exito);
        if (exito) {
            printf("%g\n", resultado);
        } else {
            fprintf(stderr, "Error: No se pudo evaluar la expresión.\n");
            return 1;
        }
        return 0;
    }

    if (argc == 2) {
        if (cargar_archivo_en_memoria(argv[1]) != 0) {
            fprintf(stderr, "Error: No se pudo abrir '%s'.\n", argv[1]);
            return 1;
        }

        scope_actual = -1;
        for (int i = 0; i < MAX_SCOPES; i++) {
            scopes_locales[i].num_variables = 0;
        }
        funcion_stack_ptr = 0;
       
        ejecutar_programa_cargado();
        limpiar_memoria_completa();
        return 0;
    }
    
    fprintf(stderr, "\n> Intérprete del lenguaje Nico v1.1.1\n");
    fprintf(stderr, "> Modo interactivo. Escribí SALIR para terminar.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "> Escribí ? para ver ayuda.\n");
    
    char comando[MAX_LINEA];
    int archivo_cargado = 0;
    
    while (1) {
        mostrar_prompt();
        if (!fgets(comando, MAX_LINEA, stdin)) break;
        limpiar_string(comando);
        if (!strlen(comando)) continue;
        
        if (strcmp(comando, "?") == 0) {
            comando_ayuda();
            continue;
        }
        
        if (comienza_con(comando, "USAR")) {
            if (archivo_cargado && num_lineas_programa > 0) {
                fprintf(stderr, "Error: Ya hay un programa cargado.\n");
                continue;
            }
            char *archivo = comando + 4;
            while (*archivo == ' ' || *archivo == '\t') archivo++;
            char *ext = strrchr(archivo, '.');
            if (!ext || strcmp(ext, ".nico") != 0) {
                fprintf(stderr, "Error: El archivo debe tener extensión .nico\n");
                continue;
            }
            limpiar_memoria_completa();
            if (cargar_archivo_en_memoria(archivo) != 0) {
                fprintf(stderr, "Error: No se pudo abrir '%s'.\n", archivo);
                continue;
            }
            fprintf(stderr, "> Archivo '%s' cargado con éxito.\n", archivo);
            archivo_cargado = 1;
            continue;
        }
        
        if (comienza_con(comando, "CORRER")) {
            if (!archivo_cargado || num_lineas_programa == 0) {
                fprintf(stderr, "Error: No hay programa cargado.\n");
                continue;
            }
            ejecutar_programa_cargado();
            continue;
        }
        
        if (comienza_con(comando, "CERRAR")) {
            if (!archivo_cargado || num_lineas_programa == 0) {
                fprintf(stderr, "Error: No hay programa cargado\n");
                continue;
            }
            limpiar_memoria_completa();
            archivo_cargado = 0;
            fprintf(stderr, "> Programa cerrado.\n");
            continue;
        }
        
        if (comienza_con(comando, "RANGOS")) {
            comando_rangos();
            continue;
        }
        
        if (comienza_con(comando, "SALIR")) {
            limpiar_memoria_completa();
            fprintf(stderr, "> Intérprete terminado.\n");
            break;
        }
        
        fprintf(stderr, "Comando desconocido. Escribí ? para ayuda.\n");
    }
    
    return 0;
}
