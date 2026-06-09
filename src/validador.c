/*
 * Nico v1.1.1 - Intérprete Educativo de Scripting en Español
 * @file:         validador.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Validación estática pre-ejecución de bloques 
 *                (SI, MIENTRAS, PARA, REALIZAR, SEGUN).
 */
#include "nico.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void validar_estructura_bloques(int inicio, int fin) {
    int si_depth = 0, mientras_depth = 0, realizar_depth = 0, para_depth = 0, segun_depth = 0;
    int ultimo_si = -1, ultimo_mientras = -1, ultimo_realizar = -1, ultimo_para = -1, ultimo_segun = -1;

    int caso_activo_stack[MAX_NESTING] = {0};
    int linea_caso_stack[MAX_NESTING] = {-1};
   
    for (int v = inicio; v <= fin; v++) {
        char lb[MAX_LINEA];
        strncpy(lb, lineas_programa[v], MAX_LINEA - 1);
        lb[MAX_LINEA - 1] = '\0';
        limpiar_string(lb);
        remover_comentario(lb);
        if (!strlen(lb)) continue;

        const char *p = lb;
        while (*p == ' ' || *p == '\t') p++;

        if (strncmp(p, "SEGUN CASO", 10) == 0) {
            if (segun_depth >= MAX_NESTING) {
                fprintf(stderr, "Error línea %d: Anidamiento máximo de SEGUN CASO excedido.\n", v + 1);
                exit(1);
            }
           
            caso_activo_stack[segun_depth] = 0;
            linea_caso_stack[segun_depth] = -1;
            segun_depth++;
            ultimo_segun = v;
        }
        else if (strncmp(p, "FIN SEGUN", 9) == 0) {
            if (segun_depth > 0 && caso_activo_stack[segun_depth - 1]) {
                fprintf(stderr, "Error línea %d: Falta 'CORTE' antes de FIN SEGUN.\n", v + 1);
                exit(1);
            }
            if (segun_depth <= 0) { 
                fprintf(stderr, "Error línea %d: 'FIN SEGUN' sin 'SEGUN CASO'.\n", v + 1); 
                exit(1); 
            }
            segun_depth--;
        }
        
        else if (segun_depth > 0) {
            int nivel_actual = segun_depth - 1; 
            
            if (strncmp(p, "CASO", 4) == 0 || strncmp(p, "POR DEFECTO", 11) == 0) {
                if (caso_activo_stack[nivel_actual]) {
                    fprintf(stderr, "Error línea %d: Falta 'CORTE' antes de este caso/línea %d.\n", 
                            v + 1, linea_caso_stack[nivel_actual] + 1);
                    exit(1);
                }
                caso_activo_stack[nivel_actual] = 1;
                linea_caso_stack[nivel_actual] = v;
            }
            else if (strncmp(p, "CORTE", 5) == 0) {
                if (caso_activo_stack[nivel_actual]) {
                    caso_activo_stack[nivel_actual] = 0;
                }
            }
        }
 
        else if (strstr(p, "ENTONCES") && 
                 (strncmp(p, "SI(", 3) == 0 || strncmp(p, "SI ", 3) == 0) &&
                 strncmp(p, "SINO", 4) != 0 && 
                 strncmp(p, "SINOSI", 6) != 0) {
            si_depth++; ultimo_si = v;
        }        
 
        else if (strncmp(p, "FIN SI", 6) == 0) {
            if (si_depth <= 0) { fprintf(stderr, "Error línea %d: 'FIN SI' sin 'SI'.\n", v + 1); exit(1); }
            si_depth--;
        }
        
        else if (comienza_con_keyword(p, "MIENTRAS") && strstr(p, "HACER")) {
            mientras_depth++; ultimo_mientras = v;
        }

        else if (strncmp(p, "FIN MIENTRAS", 12) == 0) {
            if (mientras_depth <= 0) { fprintf(stderr, "Error línea %d: 'FIN MIENTRAS' sin 'MIENTRAS...HACER'.\n", v + 1); exit(1); }
            mientras_depth--;
        }
   
        else if (strncmp(p, "REALIZAR", 8) == 0 && !strstr(p, "MIENTRAS")) {
            realizar_depth++; ultimo_realizar = v;
        }
 
        else if (comienza_con_keyword(p, "MIENTRAS") && !strstr(p, "HACER") && strchr(p, '(')) {
            if (realizar_depth <= 0) {
                fprintf(stderr, "Error línea %d: 'MIENTRAS' sin 'REALIZAR'.\n", v + 1);
                exit(1);
            }
            realizar_depth--;
        }
 
       else if (comienza_con_keyword(p, "PARA") && strstr(p, "HACER")) {
            para_depth++; ultimo_para = v;
        }        

        else if (strncmp(p, "FIN PARA", 8) == 0) {
            if (para_depth <= 0) { fprintf(stderr, "Error línea %d: 'FIN PARA' sin 'PARA...HACER'.\n", v + 1); exit(1); }
            para_depth--;
        }
    }

    if (segun_depth > 0 && caso_activo_stack[segun_depth - 1]) { 
        fprintf(stderr, "Error línea %d: Falta 'CORTE' al final del caso/línea %d.\n", 
                fin + 1, linea_caso_stack[segun_depth - 1] + 1); 
        exit(1); 
    }
    if (si_depth != 0) { fprintf(stderr, "Error línea %d: Falta 'FIN SI' para cerrar este SI.\n", ultimo_si + 1); exit(1); }
    if (mientras_depth != 0) { fprintf(stderr, "Error línea %d: Falta 'FIN MIENTRAS' para cerrar este bucle.\n", ultimo_mientras + 1); exit(1); }
    if (realizar_depth != 0) { fprintf(stderr, "Error línea %d: Falta 'MIENTRAS(condición)' para cerrar este bucle REALIZAR.\n", ultimo_realizar + 1); exit(1); }
    if (para_depth != 0) { fprintf(stderr, "Error línea %d: Falta 'FIN PARA' para cerrar este bucle.\n", ultimo_para + 1); exit(1); }
    if (segun_depth != 0) { fprintf(stderr, "Error línea %d: Falta 'FIN SEGUN' para cerrar este SEGUN CASO.\n", ultimo_segun + 1); exit(1); }
}
