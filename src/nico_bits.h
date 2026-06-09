/*
 * Nico v1.1.1 - Intérprete Educativo de Scripting en Español
 * @file:         nico_bits.h
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Operaciones bit-a-bit 
 */
#ifndef NICO_BITS_H
#define NICO_BITS_H

// Rotaciones 
unsigned int nico_rotar_izquierda(unsigned int valor, int bits);
unsigned int nico_rotar_derecha(unsigned int valor, int bits);

// Desplazamientos
unsigned int nico_desplazar_izquierda(unsigned int valor, int bits);
unsigned int nico_desplazar_derecha(unsigned int valor, int bits);

// Operaciones lógicas
unsigned int nico_bit_y(unsigned int a, unsigned int b);
unsigned int nico_bit_o(unsigned int a, unsigned int b);
unsigned int nico_bit_xor(unsigned int a, unsigned int b);
unsigned int nico_bit_no(unsigned int a);

// Manipulación de bits individuales
unsigned int nico_leer_bit(unsigned int valor, int posicion);
unsigned int nico_activar_bit(unsigned int valor, int posicion);
unsigned int nico_desactivar_bit(unsigned int valor, int posicion);

// Utilidades
unsigned int nico_invertir_bytes(unsigned int valor);
int nico_contar_bits(unsigned int valor);

#endif
