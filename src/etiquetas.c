/*
 * Nico v1.1.1 - Intérprete Educativo de Scripting en Español
 * @file:         etiquetas.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Registro y resolución de ETIQUETA, SUBPROGRAMA y 
 *                estructuras de salto.
 */
#include "nico.h"

// VARIABLES GLOBALES DE ETIQUETAS
Etiqueta etiquetas[MAX_ETIQUETAS];
int num_etiquetas = 0;

SubprogramaInfo subprogramas_registrados[MAX_NESTING];
int num_subprogramas_registrados = 0;

// FUNCIONES DE ETIQUETAS
int agregar_etiqueta(const char *nombre, int linea) {
    if (num_etiquetas >= MAX_ETIQUETAS) return -1;
    strncpy(etiquetas[num_etiquetas].nombre, nombre, MAX_NOMBRE - 1);
    etiquetas[num_etiquetas].nombre[MAX_NOMBRE - 1] = '\0';
    etiquetas[num_etiquetas].linea = linea;
    num_etiquetas++;
    return num_etiquetas - 1;
}

int buscar_etiqueta(const char *nombre) {
    for (int i = 0; i < num_etiquetas; i++) {
        if (strcmp(etiquetas[i].nombre, nombre) == 0) return etiquetas[i].linea;
    }
    return -1;
}

// REGISTRAR TODAS LAS ETIQUETAS
int registrar_todas_las_etiquetas(void) {
    for (int i = 0; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        eliminar_comentarios(linea);
        limpiar_string(linea);
        remover_comentario(linea);
        
        if (comienza_con(linea, "ETIQUETA")) {
            char *ptr = linea + 8;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            char nombre[MAX_NOMBRE];
            int j = 0;
            while (es_alnum(*ptr) && j < MAX_NOMBRE - 1) {
                nombre[j++] = *ptr++;
            }
            nombre[j] = '\0';
            agregar_etiqueta(nombre, i + 1);
        }
    }
    return 0;
}

// EJECUTAR SALTO A ETIQUETA
int ejecutar_salto_a_etiqueta(const char *nombre, int linea_actual) {
    int linea_destino = buscar_etiqueta(nombre);
    if (linea_destino == -1) {
        fprintf(stderr, "Error línea %d: Etiqueta '$%s' no encontrada.\n", linea_actual, nombre);
        return -1;
    }
    return linea_destino - 1;
}

// REGISTRAR TODOS LOS SUBPROGRAMAS
int registrar_todos_los_subprogramas(void) {
    for (int i = 0; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        eliminar_comentarios(linea);
        limpiar_string(linea);
        remover_comentario(linea);
        
        if (comienza_con(linea, "SUBPROGRAMA")) {
            char *ptr = linea + 11;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            char nombre[MAX_NOMBRE];
            int j = 0;
            while (es_alnum(*ptr) && j < MAX_NOMBRE - 1) {
                nombre[j++] = *ptr++;
            }
            nombre[j] = '\0';
            
            char params[MAX_PARAMETROS][MAX_NOMBRE];
            int num_params = 0;
            char *apertura = strchr(linea, '(');
            char *cierre = strchr(linea, ')');
            if (apertura && cierre && cierre > apertura) {
                char *p = apertura + 1;
                while (p < cierre && num_params < MAX_PARAMETROS) {
                    while (*p == ' ' || *p == '\t') p++;
                    int k = 0;
                    while (es_alnum(*p) && k < MAX_NOMBRE - 1) {
                        params[num_params][k++] = *p++;
                    }
                    params[num_params][k] = '\0';
                    if (k > 0) num_params++;
                    while (*p == ' ' || *p == '\t' || *p == ',') p++;
                }
            }
            
            int fin = i + 1;
            int nivel = 1;
            while (fin < num_lineas_programa && nivel > 0) {
                char l[MAX_LINEA];
                strncpy(l, lineas_programa[fin], MAX_LINEA - 1);
                l[MAX_LINEA - 1] = '\0';
                eliminar_comentarios(l);
                limpiar_string(l);
                remover_comentario(l);
                if (comienza_con(l, "SUBPROGRAMA")) nivel++;
                else if (comienza_con(l, "FIN SUBPROGRAMA")) nivel--;
                fin++;
            }
            
            if (num_subprogramas_registrados < MAX_NESTING) {
                strncpy(subprogramas_registrados[num_subprogramas_registrados].nombre, nombre, MAX_NOMBRE - 1);
                subprogramas_registrados[num_subprogramas_registrados].nombre[MAX_NOMBRE - 1] = '\0';
                subprogramas_registrados[num_subprogramas_registrados].linea_inicio = i;
                subprogramas_registrados[num_subprogramas_registrados].linea_fin = fin - 1;
                subprogramas_registrados[num_subprogramas_registrados].num_params = num_params;
                for (int k = 0; k < num_params; k++) {
                    strncpy(subprogramas_registrados[num_subprogramas_registrados].params[k], params[k], MAX_NOMBRE - 1);
                    subprogramas_registrados[num_subprogramas_registrados].params[k][MAX_NOMBRE - 1] = '\0';
                }
                num_subprogramas_registrados++;
            }
            
            i = fin - 1;
        }
    }
    return 0;
}

// BUSCAR SUBPROGRAMA INFO
int buscar_subprograma_info(const char *nombre) {
    for (int i = 0; i < num_subprogramas_registrados; i++) {
        if (strcmp(subprogramas_registrados[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

// ASIGNAR ARGUMENTOS A PARAMETROS
void asignar_argumentos_a_parametros(char params[][MAX_NOMBRE], int num_params, char *args[], int num_args) {
    for (int i = 0; i < num_params && i < num_args; i++) {
        char *nombre_param = params[i];
        char *valor_arg = args[i];
        
        int exito;
        double valor = evaluar_expresion_completa(valor_arg, &exito);
        
        int idx = buscar_variable(nombre_param);
        if (idx >= 0) {
            variables[idx].valor = (int)valor;
        } else {
            agregar_variable(nombre_param, (int)valor);
        }
    }
}
