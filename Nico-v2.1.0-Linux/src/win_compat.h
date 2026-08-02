/*
Nico v2.1.0 - Intérprete Educativo de Scripting en Español
@file:         win_compat.h
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
@license:      MIT / Personal Use (ver LICENSE)
@description:  Capa de compatibilidad cross-platform para Windows (MinGW-w64).
               Provee stubs y wrappers de funciones POSIX (sleep, usleep,
               strcasecmp, poll, termios, sigaction) y configuración de
               sockets (winsock2) para permitir que el código base escrito
               para Linux/Unix compile y ejecute correctamente en Windows 10/11.
               NO es compatible con Windows XP ni versiones anteriores.
*/
#ifndef WIN_COMPAT_H
#define WIN_COMPAT_H

#ifdef _WIN32

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <io.h>
#include <conio.h>
#include <signal.h>

/* ========================================================================
 * FILE DESCRIPTORS ESTÁNDAR
 * ======================================================================== */
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/* ========================================================================
 * sleep() → Sleep()
 * ======================================================================== */
#define sleep(sec) Sleep((DWORD)((sec) * 1000))

/* ========================================================================
 * usleep() → Sleep() con conversión µs → ms
 * ======================================================================== */
static inline int nico_usleep_win(unsigned int usec) {
    if (usec > 0) {
        DWORD ms = (DWORD)(usec / 1000 + (usec % 1000 > 0 ? 1 : 0));
        Sleep(ms);
    }
    return 0;
}
#define usleep(usec) nico_usleep_win(usec)

/* ========================================================================
 * Comparación de strings case-insensitive
 * ======================================================================== */
#define strcasecmp(s1, s2) _stricmp(s1, s2)
#define strncasecmp(s1, s2, n) _strnicmp(s1, s2, n)

/* ========================================================================
 * Socket API: winsock2.h para Windows 10/11
 * ======================================================================== */
#define NicoUseWSAPoll 1

/* ========================================================================
 * STUBS DE TERMIOS: no se usan realmente porque io.c usa WinAPI directo
 * ======================================================================== */
struct termios { int dummy; };
#define TCSANOW   0
#define TCSADRAIN 0
#define ICANON    0
#define ECHO      0
#define VMIN      0
#define VTIME     0

static inline int tcgetattr(int fd, struct termios *t) { (void)fd; (void)t; return 0; }
static inline int tcsetattr(int fd, int opt, const struct termios *t) { (void)fd; (void)opt; (void)t; return 0; }
static inline int tcdrain(int fd) { (void)fd; return 0; }

/* ========================================================================
 * STUBS DE SIGACTION: reemplaza la API POSIX por signal() de C estándar
 * En Windows no existe sigaction, pero signal() funciona para Ctrl+C.
 * ======================================================================== */
#ifndef SIG_SETMASK
#define SIG_SETMASK 0
#endif
#ifndef SA_RESTART
#define SA_RESTART 0
#endif

/* Definir sigset_t para Windows (no existe nativamente) */
typedef int sigset_t;

/* Estructura sigaction simplificada para Windows */
struct sigaction {
    void     (*sa_handler)(int);
    sigset_t sa_mask;      /* No se usa en Windows, pero debe existir */
    int      sa_flags;
};

/* sigemptyset: en Windows no hay máscara de señales, es un no-op */
static inline int sigemptyset(sigset_t *set) {
    (void)set;
    return 0;
}

/* sigfillset: no-op en Windows */
static inline int sigfillset(sigset_t *set) {
    (void)set;
    return 0;
}

/* sigaction: implementado usando signal() de C estándar */
static inline int sigaction(int sig, const struct sigaction *act, struct sigaction *oact) {
    (void)oact;  /* No devolvemos el handler anterior en Windows */
    if (act == NULL) return -1;
    if (signal(sig, act->sa_handler) == SIG_ERR) return -1;
    return 0;
}

#endif /* _WIN32 */

#endif /* WIN_COMPAT_H */