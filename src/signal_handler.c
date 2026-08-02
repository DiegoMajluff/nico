#include "signal_handler.h"
#include <stdio.h>

// Inicializar la bandera en falso
atomic_bool interrupcion_solicitada = ATOMIC_VAR_INIT(false);

// Función que se ejecuta cuando el SO envía SIGINT (Ctrl+C)
static void handler_sigint(int signum) {
    (void)signum;
    atomic_store(&interrupcion_solicitada, true);
}

void inicializar_manejo_senales(void) {
    struct sigaction sa;
    sa.sa_handler = handler_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // No usar SA_RESTART para que las llamadas se interrumpan inmediatamente
    
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

}
