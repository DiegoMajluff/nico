/*
 * Nico v1.1.1 - Intérprete Educativo de Scripting en Español
 * @file:         web.h
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Módulo de conectividad y servidor web. Declara los comandos nativos
 *                para la gestión de un servidor HTTP asíncrono (INICIARSERVIDOR,
 *                DETENERSERVIDOR) utilizando hilos (pthreads).
 */

#ifndef WEB_H
#define WEB_H

#include <pthread.h>

int cmd_iniciarserver(const char *linea, CtxBloque *ctx, int linea_actual);
int cmd_detenerserver(const char *linea, CtxBloque *ctx, int linea_actual);

#endif