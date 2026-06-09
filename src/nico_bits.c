/*
 * Nico v1.1.1 - Intérprete Educativo de Scripting en Español
 * @file:         nico_bits.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Funciones de bajo nivel.
 */
#include "nico.h"
#include "nico_bits.h"

// ROTACIONES
unsigned int nico_rotar_izquierda(unsigned int valor, int bits) {
    bits = bits % 32;
    if (bits == 0) return valor;
    return (valor << bits) | (valor >> (32 - bits));
}

unsigned int nico_rotar_derecha(unsigned int valor, int bits) {
    bits = bits % 32;
    if (bits == 0) return valor;
    return (valor >> bits) | (valor << (32 - bits));
}

// DESPLAZAMIENTOS
unsigned int nico_desplazar_izquierda(unsigned int valor, int bits) {
    return valor << bits;
}

unsigned int nico_desplazar_derecha(unsigned int valor, int bits) {
    return valor >> bits;
}

// OPERACIONES LÓGICAS
unsigned int nico_bit_y(unsigned int a, unsigned int b) {
    return a & b;
}

unsigned int nico_bit_o(unsigned int a, unsigned int b) {
    return a | b;
}

unsigned int nico_bit_xor(unsigned int a, unsigned int b) {
    return a ^ b;
}

unsigned int nico_bit_no(unsigned int a) {
    return ~a;
}

// MANIPULACIÓN DE BITS INDIVIDUALES
unsigned int nico_leer_bit(unsigned int valor, int posicion) {
    return (valor >> posicion) & 1;
}

unsigned int nico_activar_bit(unsigned int valor, int posicion) {
    return valor | (1 << posicion);
}

unsigned int nico_desactivar_bit(unsigned int valor, int posicion) {
    return valor & ~(1 << posicion);
}

unsigned int nico_toggle_bit(unsigned int valor, int posicion) {
    return valor ^ (1 << posicion);
}

// UTILIDADES
unsigned int nico_invertir_bytes(unsigned int valor) {
    return ((valor & 0x000000FF) << 24) |
           ((valor & 0x0000FF00) << 8)  |
           ((valor & 0x00FF0000) >> 8)  |
           ((valor & 0xFF000000) >> 24);
}

int nico_contar_bits(unsigned int valor) {
    int count = 0;
    while (valor) {
        count += valor & 1;
        valor >>= 1;
    }
    return count;
}

unsigned int nico_mascara_bits(int bits) {
    if (bits >= 32) return 0xFFFFFFFF;
    return (1 << bits) - 1;
}
