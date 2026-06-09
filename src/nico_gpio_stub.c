/*
 * Nico v1.1.1 - Intérprete Educativo de Scripting en Español
 * @file:         nico_gpio_stub.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Stub vacío de GPIO para compilación en Windows/Linux sin hardware.
 */
#include <stdio.h>

int gpio_inicializado = 0;

void nico_gpio_cleanup(void) {
    gpio_inicializado = 0;
}

void procesar_gpio_configurar(const char *argumento) {
    (void)argumento;
}

void procesar_gpio_estado_pin(const char *argumento) {
    (void)argumento;
}

void procesar_gpio_leer(const char *argumento) {
    (void)argumento;
}
