/*
 * Nico v1.0.1 - Intérprete Educativo de Scripting en Español
 * @file:         nico_gpio.h
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Cabecera de la capa de abstracción de hardware GPIO. Define estructuras,
 *                constantes y prototipos para gestión de pines.
 */
#ifndef NICO_GPIO_H
#define NICO_GPIO_H

// DETECCIÓN DE LIBGPIOD
#ifdef HAVE_LIBGPIOD
#include <gpiod.h>
#endif

// INICIALIZACIÓN Y LIMPIEZA - SIEMPRE DECLARADAS
int nico_gpio_init(void);
void nico_gpio_cleanup(void);
int gpio_verificar_disponibilidad(void);

// COMANDOS GPIO BÁSICO - SIEMPRE DECLARADOS
void procesar_gpio_configurar(const char *argumento);
void procesar_gpio_estado_pin(const char *argumento);
void procesar_gpio_leer(const char *argumento);

// HELPERS INTERNOS DE PARSING - SIEMPRE DECLARADOS
int parsear_numero_pin(const char *ptr);
int parsear_estado_si_no(const char *ptr);

// VARIABLES GLOBALES DE LIBGPIOD - SOLO CON HAVE_LIBGPIOD
#ifdef HAVE_LIBGPIOD
extern struct gpiod_chip *nico_chip;
extern struct gpiod_line_request *nico_requests[40];
#endif
#endif 
