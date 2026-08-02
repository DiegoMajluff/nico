#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>

// Bandera global atómica (segura para ser modificada desde una señal)
extern atomic_bool interrupcion_solicitada;

// Inicializar el handler de señales (llamar una vez al inicio del programa)
void inicializar_manejo_senales(void);

// Verificar si se solicitó interrupción
static inline bool hay_interrupcion(void) {
    return atomic_load(&interrupcion_solicitada);
}

// Resetear la bandera (útil si el usuario captura el error y quiere seguir)
static inline void resetear_interrupcion(void) {
    atomic_store(&interrupcion_solicitada, false);
}

#endif
