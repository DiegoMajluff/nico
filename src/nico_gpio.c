/*
 * Nico v1.1.1 - Intérprete Educativo de Scripting en Español
 * @file:         nico_gpio.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Implementación nativa de GPIO para Raspberry Pi 
 *                (requiere libgpiod/wiringPi).
 */
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include "nico.h"
#include "nico_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// DETECCIÓN DE LIBGPIOD
#ifdef HAVE_LIBGPIOD
#include <gpiod.h>
#endif

// VARIABLES GLOBALES
#ifdef HAVE_LIBGPIOD
struct gpiod_chip *nico_chip = NULL;
struct gpiod_line_request *nico_requests[40] = {NULL};
#endif

static int gpio_inicializado = 0;
static int gpio_disponible = 0;

// VERIFICAR DISPONIBILIDAD
int gpio_verificar_disponibilidad(void) {
    if (gpio_disponible != 0) {
        return gpio_disponible;
    }
    
    FILE *f = fopen("/proc/device-tree/model", "r");
    if (f) {
        char model[256];
        fread(model, 1, sizeof(model), f);
        fclose(f);
        if (strstr(model, "Raspberry Pi") == NULL) {
            gpio_disponible = 0;
            return 0;
        }
    } else {
        gpio_disponible = 0;
        return 0;
    }
    
#ifndef HAVE_LIBGPIOD
    fprintf(stderr, "GPIO: libgpiod no disponible en compilación.\n");
    gpio_disponible = 0;
    return 0;
#endif
    
#ifdef HAVE_LIBGPIOD
    struct gpiod_chip *test_chip = gpiod_chip_open("/dev/gpiochip0");
    if (!test_chip) {
        fprintf(stderr, "GPIO: No se pudo abrir /dev/gpiochip0\n");
        fprintf(stderr, "Verificá permisos: sudo usermod -a -G gpio $USER.\n");
        gpio_disponible = 0;
        return 0;
    }
    gpiod_chip_close(test_chip);
#endif
    
    gpio_disponible = 1;
    return 1;
}

int nico_gpio_init(void) {
    if (gpio_inicializado) return 0;
    
    if (!gpio_verificar_disponibilidad()) {
        return -1;
    }
    
#ifdef HAVE_LIBGPIOD
    nico_chip = gpiod_chip_open("/dev/gpiochip0");
    if (!nico_chip) {
        fprintf(stderr, "Error: No se pudo abrir gpiochip0.\n");
        return -1;
    }
#endif
    
    gpio_inicializado = 1;
    return 0;
}

void nico_gpio_cleanup(void) {
#ifdef HAVE_LIBGPIOD
    if (gpio_inicializado) {
        for (int i = 0; i < 40; i++) {
            if (nico_requests[i]) {
                gpiod_line_request_release(nico_requests[i]);
                nico_requests[i] = NULL;
            }
        }
        if (nico_chip) {
            gpiod_chip_close(nico_chip);
            nico_chip = NULL;
        }
        gpio_inicializado = 0;
    }
#endif
}

// Helper: parsea número literal O variable $nombre
int parsear_numero_pin_o_variable(const char *ptr) {
    // Saltar espacios y paréntesis iniciales
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '(') ptr++;
    
    // Si comienza con $, es una variable de Nico
    if (*ptr == '$') {
        ptr++; // Saltar el $
        
        // Extraer nombre de variable (solo alfanuméricos)
        char nombre[MAX_NOMBRE] = "";
        int i = 0;
        while (es_alnum(*ptr) && i < MAX_NOMBRE - 1) {
            nombre[i++] = *ptr++;
        }
        nombre[i] = '\0';
        
        // Buscar en tabla de variables de Nico
        int idx = buscar_variable(nombre);
        if (idx >= 0) {
            // Retornar valor entero (trunca decimal si hace falta)
            return (int)variables[idx].valor;
        } else {
            fprintf(stderr, "Error: Variable '$%s' no declarada.\n", nombre);
            return -1;
        }
    }
    
    // Si no es variable, es literal numérico (comportamiento original)
    return atoi(ptr);
}

int parsear_estado_si_no(const char *ptr) {
    if (strstr(ptr, " SI") || strstr(ptr, " SI)")) return 1;
    if (strstr(ptr, " NO") || strstr(ptr, " NO)")) return 0;
    return -1;
}

// CONFIGURAR LÍNEA GPIO
#ifdef HAVE_LIBGPIOD
static int configurar_linea_gpio(int pin, int output) {
    (void)pin;
    (void)output;
    if (pin < 0 || pin >= 40) return -1;
    
    if (nico_requests[pin]) {
        gpiod_line_request_release(nico_requests[pin]);
        nico_requests[pin] = NULL;
    }
    
    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    if (!settings) return -1;
    
    gpiod_line_settings_set_direction(settings, 
        output ? GPIOD_LINE_DIRECTION_OUTPUT : GPIOD_LINE_DIRECTION_INPUT);
    
    struct gpiod_line_config *config = gpiod_line_config_new();
    if (!config) {
        gpiod_line_settings_free(settings);
        return -1;
    }
    unsigned int pin_uint = (unsigned int)pin;
    gpiod_line_config_add_line_settings(config, &pin_uint, 1, settings);
    
    struct gpiod_request_config *req_config = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_config, "nico");
    
    nico_requests[pin] = gpiod_chip_request_lines(nico_chip, req_config, config);
    
    gpiod_line_settings_free(settings);
    gpiod_line_config_free(config);
    gpiod_request_config_free(req_config);
    
    return nico_requests[pin] ? 0 : -1;
}
#else
static int configurar_linea_gpio(int pin, int output) {
    fprintf(stderr, "Error: libgpiod no disponible.\n");
    return -1;
}
#endif

// COMANDOS GPIO - CONFIGURAR con sintaxis explícita: ENTRADA [+ PULLUP/PULLDOWN]
void procesar_gpio_configurar(const char *argumento) {
    if (!gpio_verificar_disponibilidad()) {
        fprintf(stderr, "Error: GPIO no disponible.\n");
        return;
    }
    if (!gpio_inicializado) {
        if (nico_gpio_init() < 0) return;
    }

    int pin = parsear_numero_pin_o_variable(argumento);
    if (pin < 0 || pin >= 40) {
        fprintf(stderr, "Error: Pin fuera de rango (0-39 BCM).\n");
        return;
    }

    // Determinar dirección y bias
    int output = -1; // -1: error, 0: Entrada, 1: Salida
    int aplicar_bias = 0;
    enum gpiod_line_bias bias = GPIOD_LINE_BIAS_DISABLED; // Por defecto: flotante

    // Parseo explícito según nueva sintaxis Nico
    if (strstr(argumento, "SALIDA") || strstr(argumento, "OUTPUT")) {
        output = 1;
        bias = GPIOD_LINE_BIAS_DISABLED; // Salidas ignoran bias
    } else if (strstr(argumento, "ENTRADA") || strstr(argumento, "INPUT")) {
        output = 0;
        // Verificar si se solicita bias explícitamente
        if (strstr(argumento, "PULLUP")) {
            bias = GPIOD_LINE_BIAS_PULL_UP;
            aplicar_bias = 1;
        } else if (strstr(argumento, "PULLDOWN")) {
            bias = GPIOD_LINE_BIAS_PULL_DOWN;
            aplicar_bias = 1;
        } else {
            // Solo ENTRADA -> Flotante (requiere resistencia externa)
            aplicar_bias = 0;
            bias = GPIOD_LINE_BIAS_DISABLED;
        }
    } else {
        fprintf(stderr, "Error: CONFIGURARPIN requiere SALIDA o ENTRADA (opcional: PULLUP/PULLDOWN).\n");
        return;
    }

#ifdef HAVE_LIBGPIOD
    // 1. Liberar request previo
    if (nico_requests[pin]) {
        gpiod_line_request_release(nico_requests[pin]);
        nico_requests[pin] = NULL;
    }

    // 2. Crear settings
    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    if (!settings) {
        fprintf(stderr, "Error: No se pudo crear line_settings.\n");
        return;
    }

    gpiod_line_settings_set_direction(settings,
        output ? GPIOD_LINE_DIRECTION_OUTPUT : GPIOD_LINE_DIRECTION_INPUT);

    // 3. Aplicar bias solo si es entrada y se solicitó explícitamente
    if (!output && aplicar_bias) {
        gpiod_line_settings_set_bias(settings, bias);
    }

    // 4. Configurar y solicitar
    struct gpiod_line_config *config = gpiod_line_config_new();
    if (!config) {
        gpiod_line_settings_free(settings);
        fprintf(stderr, "Error: No se pudo crear line_config.\n");
        return;
    }

    unsigned int pin_uint = (unsigned int)pin;
    gpiod_line_config_add_line_settings(config, &pin_uint, 1, settings);

    struct gpiod_request_config *req_config = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_config, "nico");

    nico_requests[pin] = gpiod_chip_request_lines(nico_chip, req_config, config);

    // 5. Limpieza
    gpiod_line_settings_free(settings);
    gpiod_line_config_free(config);
    gpiod_request_config_free(req_config);

    if (!nico_requests[pin]) {
        fprintf(stderr, "Error: No se pudo solicitar GPIO %d.\n", pin);
    }
#else
    fprintf(stderr, "Error: libgpiod no disponible en esta compilación.\n");
#endif
}

void procesar_gpio_estado_pin(const char *argumento) {
    if (!gpio_verificar_disponibilidad()) {
        fprintf(stderr, "Error: GPIO no disponible.\n");
        return;
    }
    
    if (!gpio_inicializado) {
        if (nico_gpio_init() < 0) return;
    }
    
    int pin = parsear_numero_pin_o_variable(argumento);
    if (pin < 0 || pin >= 40) {
        fprintf(stderr, "Error: Pin fuera de rango (0-39 BCM).\n");
        return;
    }
    int estado = parsear_estado_si_no(argumento);
    
    if (estado == -1) {
        fprintf(stderr, "Error: ESTADOPIN requiere SI o NO.\n");
        return;
    }
    
#ifdef HAVE_LIBGPIOD
    if (nico_requests[pin]) {
        gpiod_line_request_set_value(nico_requests[pin], pin, 
            estado ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
    } else {
        fprintf(stderr, "Error: GPIO %d no configurado.\n", pin);
    }
#endif
}

void procesar_gpio_leer(const char *argumento) {
    if (!gpio_verificar_disponibilidad()) {
        fprintf(stderr, "Error: GPIO no disponible.\n");
        return;
    }
    
    if (!gpio_inicializado) {
        if (nico_gpio_init() < 0) return;
    }
    
    int valor = 0;
    int pin = parsear_numero_pin_o_variable(argumento);
    if (pin < 0 || pin >= 40) {
        fprintf(stderr, "Error: Pin fuera de rango (0-39 BCM).\n");
        return;
    }
#ifdef HAVE_LIBGPIOD
    if (!nico_requests[pin]) {
        configurar_linea_gpio(pin, 0);
    }
    if (nico_requests[pin]) {
        valor = gpiod_line_request_get_value(nico_requests[pin], pin);
    }
#endif
    
    const char *en_ptr = strstr(argumento, "EN");
    if (en_ptr) {
        en_ptr += 2;
        while (*en_ptr == ' ' || *en_ptr == '\t') en_ptr++;
        if (*en_ptr == '$') en_ptr++;
        
        char nombre_var[MAX_NOMBRE] = "";
        int i = 0;
        while (es_alnum(*en_ptr) && i < MAX_NOMBRE - 1) {
            nombre_var[i++] = *en_ptr++;
        }
        nombre_var[i] = '\0';
        
        int idx = buscar_variable(nombre_var);
        if (idx >= 0) {
            variables[idx].valor = valor;
        } else {
            fprintf(stderr, "Error: Variable '$%s' no declarada.\n", nombre_var);
        }
    } else {
        printf("%d", valor);
        fflush(stdout);
    }
}
