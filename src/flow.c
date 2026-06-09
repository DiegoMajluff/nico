/*
 * Nico v1.1.1 - Intérprete Educativo de Scripting en Español
 * @file:         flow.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Control de flujo: búsqueda de líneas, saltos, manejo de CORTE, 
 *                SALTAR A y resolución de etiquetas.
 */
#include "nico.h"
#include <string.h>
#include <stdio.h>

#define DEBUG_FLUJO 0 

int encontrar_fin_si(int linea_inicio) {
    int profundidad = 1;
    for (int i = linea_inicio + 1; i < num_lineas_programa; i++) {
        char l[MAX_LINEA];
        strncpy(l, lineas_programa[i], MAX_LINEA - 1);
        l[MAX_LINEA - 1] = '\0';
        limpiar_string(l);
        remover_comentario(l);
        if (!strlen(l)) continue;

#if DEBUG_FLUJO
        fprintf(stderr, "[FIN_SI] L%d: '%s' (prof=%d)\n", i, l, profundidad);
#endif

        if (strncmp(l, "FIN SI", 6) == 0) {
            profundidad--;
            if (profundidad == 0) return i;
        }
        else if ((strncmp(l, "SI(", 3) == 0 || strncmp(l, "SI ", 3) == 0) &&
                 strncmp(l, "SINO", 4) != 0 && strncmp(l, "SINOSI", 6) != 0) {
            profundidad++;
        }
    }
    return -1;
}

int saltar_hasta_rama_o_fin_si(int linea_inicio) {
    int profundidad = 1;
    for (int i = linea_inicio + 1; i < num_lineas_programa; i++) {
        char l[MAX_LINEA];
        strncpy(l, lineas_programa[i], MAX_LINEA - 1);
        l[MAX_LINEA - 1] = '\0';
        limpiar_string(l);
        remover_comentario(l);
        if (!strlen(l)) continue;

#if DEBUG_FLUJO
        fprintf(stderr, "[SALTAR] L%d: '%s' (prof=%d)\n", i, l, profundidad);
#endif

        if (strncmp(l, "FIN SI", 6) == 0) {
            profundidad--;
            if (profundidad == 0) return i;
        }
        else if ((strncmp(l, "SI(", 3) == 0 || strncmp(l, "SI ", 3) == 0) &&
                 strncmp(l, "SINO", 4) != 0 && strncmp(l, "SINOSI", 6) != 0) {
            profundidad++;
        }
        else if (profundidad == 1) {
            if (strncmp(l, "SINOSI", 6) == 0) return i;
            if (strncmp(l, "SINO SI", 7) == 0) return i;
            if (strncmp(l, "SINO", 4) == 0) {
                char c = l[4];
                if (c == '\0' || c == ' ' || c == '\t' || c == '\n' || strncmp(l+4, "ENTONCES", 8) == 0) {
                    return i;
                }
            }
        }
    }
    return -1;
}

int encontrar_sino(int linea_inicio) {
    int nivel = 0;
    for (int i = linea_inicio + 1; i < num_lineas_programa; i++) {
        char l[MAX_LINEA];
        strncpy(l, lineas_programa[i], MAX_LINEA - 1);
        l[MAX_LINEA - 1] = '\0';
        limpiar_string(l);
        remover_comentario(l);
        if (!strlen(l)) continue;

        if (comienza_con(l, "SI") && !comienza_con(l, "SINO") && !comienza_con(l, "SINOSI")) {
            nivel++;
        }
        else if (comienza_con(l, "FIN SI")) {
            if (nivel == 0) return -1;
            nivel--;
        }
        else if (nivel == 0 && (comienza_con(l, "SINOSI") || comienza_con(l, "SINO"))) {
            return i;
        }
    }
    return -1;
}
// ENCONTRAR FIN MIENTRAS
int encontrar_fin_mientras(int linea_inicio, int nivel) {
    int nivel_actual = nivel;
    for (int i = linea_inicio; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        
        if (comienza_con_keyword(linea, "MIENTRAS") && strstr(linea, "HACER")) {
            nivel_actual++;
        }

        else if (strncmp(linea, "FIN MIENTRAS", 12) == 0) {
            nivel_actual--;
            if (nivel_actual == nivel) return i;
        }
    }
    return -1;
}

// ENCONTRAR FIN REALIZAR
int encontrar_fin_proceder(int linea_inicio, int nivel) {
    int nivel_actual = nivel;
    for (int i = linea_inicio; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        
        if (strncmp(linea, "REALIZAR", 8) == 0) {
            nivel_actual++;
        }
        else if (strncmp(linea, "MIENTRAS", 8) == 0) {
            nivel_actual--;
            if (nivel_actual == nivel) return i;
        }
    }
    return -1;
}

// ENCONTRAR FIN PARA
int encontrar_fin_para(int linea_inicio, int nivel) {
    int nivel_actual = nivel;
    for (int i = linea_inicio; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        
        if (strncmp(linea, "PARA", 4) == 0 && strstr(linea, "HACER")) {
            nivel_actual++;
        }
        else if (strncmp(linea, "FIN PARA", 8) == 0) {
            nivel_actual--;
            if (nivel_actual == nivel) return i;
        }
    }
    return -1;
}

// ENCONTRAR FIN SEGUN
int encontrar_fin_segun(int linea_inicio) {
    int nivel = 0;
    for (int i = linea_inicio; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        
        if (comienza_con(linea, "SEGUN")) nivel++;
        if (comienza_con(linea, "FIN SEGUN")) {
            nivel--;
            if (nivel == 0) return i;
        }
    }
    return -1;
}

// ENCONTRAR SIGUIENTE CASO
int encontrar_siguiente_caso(int linea_inicio) {
    int nivel = 0;
    for (int i = linea_inicio; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        
        if (comienza_con(linea, "SEGUN") && strstr(linea, "HACER")) {
            nivel++;
        }
        else if (comienza_con(linea, "FIN SEGUN")) {
            nivel--;
            if (nivel == 0) return -1;
        }
        else if (nivel >= 1 && comienza_con(linea, "CASO")) {
            return i;
        }
    }
    return -1;
}

// ENCONTRAR FIN SUBPROGRAMA
int encontrar_fin_subprograma(int linea_inicio) {
    int nivel = 0;
    for (int i = linea_inicio; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        
        if (comienza_con(linea, "SUBPROGRAMA")) {
            nivel++;
        }
        else if (comienza_con(linea, "FIN SUBPROGRAMA")) {
            nivel--;
            if (nivel == 0) return i;
        }
    }
    return -1;
}

// ENCONTRAR FIN FUNCION
int encontrar_fin_funcion(int linea_inicio) {
    int nivel = 0;
    for (int i = linea_inicio; i < num_lineas_programa; i++) {
        char linea[MAX_LINEA];
        strncpy(linea, lineas_programa[i], MAX_LINEA - 1);
        linea[MAX_LINEA - 1] = '\0';
        limpiar_string(linea);
        remover_comentario(linea);
        if (!strlen(linea)) continue;
        
        if (comienza_con(linea, "FUNCION")) {
            nivel++;
        }
        else if (comienza_con(linea, "FIN FUNCION")) {
            nivel--;
            if (nivel == 0) return i;
        }
    }
    return -1;
}

// BUSCAR INICIO SUBPROGRAMA
int buscar_inicio_subprograma(const char *nombre) {
    for (int i = 0; i < num_subprogramas_registrados; i++) {
        if (strcmp(subprogramas_registrados[i].nombre, nombre) == 0)
            return subprogramas_registrados[i].linea_inicio;
    }
    return -1;
}
