/*
 * Nico v1.1.1 - Intérprete Educativo de Scripting en Español
 * @file:         leertecla.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión).
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización.
 * @license:      MIT / Personal Use (ver LICENSE).
 * @description:  Comando nativo LEERTECLA para Nico.
                  Lectura no bloqueante de teclado con decodificación ANSI.
 */
#include "nico.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <termios.h>
    #include <poll.h>
    #include <unistd.h>
#endif

#ifdef _WIN32
    static HANDLE h_stdin_lt = INVALID_HANDLE_VALUE;
    static DWORD orig_mode_lt = 0;
    static int win_lt_configured = 0;
#else
    static struct termios term_original;
    static int term_configurado = 0;
#endif

static void restaurar_terminal(void) {
#ifdef _WIN32
    if (win_lt_configured && h_stdin_lt != INVALID_HANDLE_VALUE) {
        SetConsoleMode(h_stdin_lt, orig_mode_lt);
        win_lt_configured = 0;
    }
#else
    if (term_configurado) {
        tcsetattr(STDIN_FILENO, TCSANOW, &term_original);
        term_configurado = 0;
    }
#endif
}

static int iniciar_terminal_raw(void) {
#ifdef _WIN32
    if (win_lt_configured) return 0;
    
    h_stdin_lt = GetStdHandle(STD_INPUT_HANDLE);
    if (h_stdin_lt == INVALID_HANDLE_VALUE) return -1;
    
    if (!GetConsoleMode(h_stdin_lt, &orig_mode_lt)) return -1;
    
    DWORD new_mode = orig_mode_lt & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
    if (!SetConsoleMode(h_stdin_lt, new_mode)) return -1;
    
    win_lt_configured = 1;
    atexit(restaurar_terminal);
    return 0;
#else
    if (term_configurado) return 0;
    if (tcgetattr(STDIN_FILENO, &term_original) == -1) return -1;
    
    struct termios raw = term_original;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;
    
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1) return -1;
    term_configurado = 1;
    atexit(restaurar_terminal);
    return 0;
#endif
}

static int leer_tecla_raw(void) {
#ifdef _WIN32
    INPUT_RECORD ir;
    DWORD events = 0;
    
    if (!GetNumberOfConsoleInputEvents(h_stdin_lt, &events) || events == 0) return 0;
    if (!PeekConsoleInput(h_stdin_lt, &ir, 1, &events) || events == 0) return 0;
    
    if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
        ReadConsoleInput(h_stdin_lt, &ir, 1, &events);
        
        WORD vk = ir.Event.KeyEvent.wVirtualKeyCode;
        switch (vk) {
            case VK_UP:    return 1001;  // ⬆️ 
            case VK_DOWN:  return 1002;  // ⬇️ 
            case VK_RIGHT: return 1003;  // ➡️ 
            case VK_LEFT:  return 1004;  // ⬅️ 
            case VK_ESCAPE: return 27;   // ESC
            default: {
                char c = ir.Event.KeyEvent.uChar.AsciiChar;
                if (c) return (unsigned char)c;
                return 0;
            }
        }
    }
    if (events > 0) ReadConsoleInput(h_stdin_lt, &ir, 1, &events);
    return 0;
#else
    struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };
    if (poll(&pfd, 1, 0) <= 0) return 0;
    
    char buf[8] = {0};
    ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
    if (n <= 0) return 0;
    buf[n] = '\0';
    
    if (n == 1) return (unsigned char)buf[0];
    
    if (n >= 3 && buf[0] == '\x1B' && buf[1] == '[') {
        switch (buf[2]) {
            case 'A': return 1001;  // ⬆️ 
            case 'B': return 1002;  // ⬇️ 
            case 'C': return 1003;  // ➡️ 
            case 'D': return 1004;  // ⬅️ 
        }
    }
    return (unsigned char)buf[0];
#endif
}

// COMANDO NATIVO: LEERTECLA ($var) 
int cmd_leertecla(const char *linea, CtxBloque *ctx, int linea_actual) {
    if (iniciar_terminal_raw() != 0) {
        fprintf(stderr, "Error línea %d: No se pudo configurar terminal para LEERTECLA.\n", linea_actual);
        return -1;
    }
    
    const char *ptr = linea + 9;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    if (*ptr != '(') {
        fprintf(stderr, "Error línea %d: Sintaxis inválida. Uso correcto: LEERTECLA ($variable).\n", linea_actual);
        return -1;
    }
    ptr++;
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    if (*ptr != '$') {
        fprintf(stderr, "Error línea %d: El signo '$' es obligatorio. Ejemplo: LEERTECLA ($tecla).\n", linea_actual);
        return -1;
    }
    ptr++;
    
    char nombre[MAX_NOMBRE] = {0};
    int i = 0;
    while (*ptr && *ptr != ')' && *ptr != ' ' && *ptr != '\t' && i < MAX_NOMBRE - 1) {
        nombre[i++] = *ptr++;
    }
    nombre[i] = '\0';
    
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    if (*ptr != ')') {
        fprintf(stderr, "Error línea %d: Falta paréntesis de cierre en LEERTECLA.\n", linea_actual);
        return -1;
    }
    
    if (!strlen(nombre)) {
        fprintf(stderr, "Error línea %d: Variable vacía en LEERTECLA.\n", linea_actual);
        return -1;
    }
    
    int valor = leer_tecla_raw();
    int encontrado = 0;
    
    if (scope_actual >= 0) {
        for (int s = scope_actual; s >= 0; s--) {
            ScopeLocal *scp = &scopes_locales[s];
            for (int v = 0; v < scp->num_variables; v++) {
                if (strcmp(scp->variables[v].nombre, nombre) == 0) {
                    switch (scp->variables[v].tipo) {
                        case 0: scp->variables[v].valor.valor_entero = valor; break;
                        case 1: scp->variables[v].valor.valor_sin_signo = (unsigned int)valor; break;
                        case 2:
                        case 3: scp->variables[v].valor.valor_decimal = (double)valor; break;
                        case 4: scp->variables[v].valor.valor_caracter = (char)valor; break;
                        case 5: scp->variables[v].valor.valor_caracter_sin_signo = (unsigned char)valor; break;
                    }
                    encontrado = 1;
                    goto fin_asignacion;
                }
            }
        }
    }
    
    int idx = buscar_variable(nombre);
    if (idx >= 0) { variables[idx].valor = valor; encontrado = 1; }
    else if ((idx = buscar_variable_sin_signo(nombre)) >= 0) {
        variables_sin_signo[idx].valor = (unsigned int)valor; encontrado = 1;
    }
    else if ((idx = buscar_variable_decimal(nombre)) >= 0) {
        variables_decimal[idx].valor = (double)valor; encontrado = 1;
    }
    else if ((idx = buscar_variable_caracter(nombre)) >= 0) {
        variables_caracter[idx].valor = (char)valor; encontrado = 1;
    }
    
fin_asignacion:
    if (!encontrado) {
        fprintf(stderr, "Error línea %d: Variable '$%s' no declarada en scope local ni global.\n", linea_actual, nombre);
    }
    
    ctx->linea_num++;
    return 0;
}
