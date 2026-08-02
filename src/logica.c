/*
Nico v2.1.0 - Motor Lógico (estilo Prolog)
@file:         logica.c
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
@license:      MIT / Personal Use (ver LICENSE)
@description:  Motor de inferencia lógica con unificación y backtracking,
               integrado en el lenguaje Nico. Implementa un subconjunto
               del paradigma Prolog: hechos, reglas, consultas, variables,
               listas (concretas y patrones [H|T]), occurs check, corte,
               negación por fallo, disyunción (O / ;), aritmética, y
               built-ins (MIEMBRO, LONGITUD, PRIMER, ULTIMO, JUNTAR,
               REVERSA, BUSCARTODOS). Diseñado para ser embebible dentro
               del intérprete imperativo de Nico, permitiendo combinar
               programación lógica con imperativa en un mismo programa.
*/
#include "logica.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

static int contador_renombre_global = 0;
static int contador_orden_global = 0;

static void trim(char *s)
{
    char *p = s;
    while (isspace((unsigned char)*p))
        p++;
    if (p != s)
        memmove(s, p, strlen(p) + 1);
    int len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1]))
        s[--len] = '\0';
}

static bool es_identificador(char c)
{
    return isalnum((unsigned char)c) || c == '_';
}

// Evalúa una expresión aritmética simple (solo +, -, *, /, %, ^ y paréntesis)
// No usa el contexto de Nico, solo los bindings actuales.
// IMPORTANTE: NO es static, debe ser visible desde evaluator.c
double evaluar_expresion_simple(const char *expr, const Bindings *bindings)
{
    if (!expr || !bindings)
        return 0.0;

    char expr_resuelta[512] = {0};
    strncpy(expr_resuelta, expr, sizeof(expr_resuelta) - 1);

    // Normalizar " MOD " a "%"
    char *mod_pos;
    while ((mod_pos = strstr(expr_resuelta, " MOD ")) != NULL)
    {
        memmove(mod_pos + 1, mod_pos + 5, strlen(mod_pos + 5) + 1);
        *mod_pos = '%';
    }

    // Reemplazar variables SIN paréntesis
    for (int i = 0; i < bindings->count; i++)
    {
        const char *var_name = bindings->items[i].variable;
        size_t var_len = strlen(var_name);
        char *pos = strstr(expr_resuelta, var_name);

        while (pos != NULL)
        {
            bool inicio_ok =
                (pos == expr_resuelta || !es_identificador(*(pos - 1)));

            bool fin_ok =
                (!es_identificador(*(pos + var_len)));

            if (inicio_ok && fin_ok)
            {
                char antes[512] = {0};
                strncpy(antes, expr_resuelta, pos - expr_resuelta);
                char despues[512] = {0};
                strcpy(despues, pos + var_len);
                // SIN PARÉNTESIS - reemplazo directo
                snprintf(expr_resuelta, sizeof(expr_resuelta), "%s%s%s",
                         antes, bindings->items[i].valor, despues);
                // Buscar la siguiente ocurrencia DESPUÉS del valor insertado
                pos = strstr(expr_resuelta + strlen(antes) + strlen(bindings->items[i].valor), var_name);
            }
            else
            {
                pos = strstr(pos + 1, var_name);
            }
        }
    }

    // Intentar convertir directamente a número
    char *endptr;
    double val = strtod(expr_resuelta, &endptr);

    // Ignorar espacios al final
    while (isspace((unsigned char)*endptr))
        endptr++;

    if (endptr != expr_resuelta && *endptr == '\0')
    {
        return val;
    }
    // Evaluar recursivamente buscando operadores de menor a mayor precedencia
    int len = strlen(expr_resuelta);
    int nivel = 0;

    // 1. Buscar + y - (menor precedencia) de derecha a izquierda
    for (int i = len - 1; i >= 0; i--)
    {
        if (expr_resuelta[i] == ')')
            nivel++;
        else if (expr_resuelta[i] == '(')
            nivel--;
        else if (nivel == 0 && (expr_resuelta[i] == '+' || expr_resuelta[i] == '-') && i > 0)
        {
            char izq_str[256], der_str[256];
            strncpy(izq_str, expr_resuelta, i);
            izq_str[i] = '\0';
            strcpy(der_str, expr_resuelta + i + 1);

            trim(izq_str);
            trim(der_str);

            double izq = evaluar_expresion_simple(izq_str, bindings);
            double der = evaluar_expresion_simple(der_str, bindings);

            if (expr_resuelta[i] == '+')
                return izq + der;
            if (expr_resuelta[i] == '-')
                return izq - der;
        }
    }

    // 2. Buscar *, /, % (mayor precedencia) de derecha a izquierda
    nivel = 0;
    for (int i = len - 1; i >= 0; i--)
    {
        if (expr_resuelta[i] == ')')
            nivel++;
        else if (expr_resuelta[i] == '(')
            nivel--;
        else if (nivel == 0 && (expr_resuelta[i] == '*' || expr_resuelta[i] == '/' || expr_resuelta[i] == '%'))
        {
            char izq_str[256], der_str[256];
            strncpy(izq_str, expr_resuelta, i);
            izq_str[i] = '\0';
            strcpy(der_str, expr_resuelta + i + 1);

            trim(izq_str);
            trim(der_str);

            double izq = evaluar_expresion_simple(izq_str, bindings);
            double der = evaluar_expresion_simple(der_str, bindings);

            if (expr_resuelta[i] == '*')
                return izq * der;
            if (expr_resuelta[i] == '/')
                return der != 0 ? izq / der : 0.0;
            if (expr_resuelta[i] == '%')
                return der != 0 ? (double)((long long)izq % (long long)der) : 0.0;
        }
    }

    // 3. Buscar ^ (potencia)
    nivel = 0;
    for (int i = len - 1; i >= 0; i--)
    {
        if (expr_resuelta[i] == ')')
            nivel++;
        else if (expr_resuelta[i] == '(')
            nivel--;
        else if (nivel == 0 && expr_resuelta[i] == '^')
        {
            char izq_str[256], der_str[256];
            strncpy(izq_str, expr_resuelta, i);
            izq_str[i] = '\0';
            strcpy(der_str, expr_resuelta + i + 1);

            trim(izq_str);
            trim(der_str);

            double izq = evaluar_expresion_simple(izq_str, bindings);
            double der = evaluar_expresion_simple(der_str, bindings);

            return pow(izq, der);
        }
    }

    // 4. Si está entre paréntesis, quitarlos y reevaluar
    if (expr_resuelta[0] == '(' && expr_resuelta[len - 1] == ')')
    {
        int nivel_check = 0;
        bool balanceado = true;
        for (int i = 0; i < len; i++)
        {
            if (expr_resuelta[i] == '(')
                nivel_check++;
            else if (expr_resuelta[i] == ')')
                nivel_check--;
            if (nivel_check == 0 && i < len - 1)
            {
                balanceado = false;
                break;
            }
        }
        if (balanceado && nivel_check == 0)
        {
            char interior[512];
            strncpy(interior, expr_resuelta + 1, len - 2);
            interior[len - 2] = '\0';
            return evaluar_expresion_simple(interior, bindings);
        }
    }

    return 0.0;
}

// ============================================================
// CREACIÓN / DESTRUCCIÓN
// ============================================================
BaseConocimiento *bc_crear(void)
{
    BaseConocimiento *bc = calloc(1, sizeof(BaseConocimiento));
    return bc;
}

void bc_destruir(BaseConocimiento *bc)
{
    if (bc)
        free(bc);
}

void bc_limpiar(BaseConocimiento *bc)
{
    if (!bc)
        return;
    bc->num_hechos = 0;
    bc->num_reglas = 0;
    contador_renombre_global = 0;
    contador_orden_global = 0;
}

void bc_imprimir(const BaseConocimiento *bc)
{
    if (!bc)
        return;
    printf("\n=== BASE DE CONOCIMIENTO ===\n");
    printf("Hechos (%d):\n", bc->num_hechos);
    for (int i = 0; i < bc->num_hechos; i++)
    {
        printf("  %s(", bc->hechos[i].predicado);
        for (int j = 0; j < bc->hechos[i].num_args; j++)
        {
            if (j > 0)
                printf(", ");
            printf("%s", bc->hechos[i].args[j]);
        }
        printf(")\n");
    }
    printf("Reglas (%d):\n", bc->num_reglas);
    for (int i = 0; i < bc->num_reglas; i++)
    {
        printf("  %s(", bc->reglas[i].nombre);
        for (int j = 0; j < bc->reglas[i].num_cabeza; j++)
        {
            if (j > 0)
                printf(", ");
            printf("%s", bc->reglas[i].cabeza_args[j]);
        }
        printf(") %s ", bc->reglas[i].usa_neck ? ":-" : "SI");
        for (int j = 0; j < bc->reglas[i].num_cuerpo; j++)
        {
            if (j > 0)
                printf(", ");
            if (bc->reglas[i].cuerpo[j].negacion)
                printf("NO ");
            printf("%s(", bc->reglas[i].cuerpo[j].predicado);
            for (int k = 0; k < bc->reglas[i].cuerpo[j].num_args; k++)
            {
                if (k > 0)
                    printf(", ");
                printf("%s", bc->reglas[i].cuerpo[j].args[k]);
            }
            printf(")");
        }
        printf("\n");
    }
    printf("============================\n\n");
}

// ============================================================
// PARSER DE STRINGS
// ============================================================
// Versión ESTRICTA: debe empezar con mayúscula y solo contener
// caracteres alfanuméricos o guión bajo. Esto permite distinguir
// "X" (variable) de "X,Y" (cabeza múltiple de patrón).
static bool es_variable(const char *s)
{
    if (!s || strlen(s) == 0)
        return false;

    // Debe empezar con mayúscula
    if (!isupper((unsigned char)s[0]))
        return false;

    // El resto de los caracteres deben ser alfanuméricos o guión bajo
    size_t len = strlen(s);
    for (size_t i = 1; i < len; i++)
    {
        if (!isalnum((unsigned char)s[i]) && s[i] != '_')
        {
            return false; // Contiene caracteres inválidos (como ',', '[', etc.)
        }
    }
    return true;
}

// ============================================================
// FUNCIONES AUXILIARES PARA LISTAS
// ============================================================

// Prototipo forward (necesario porque unificar_listas llama a unificar_arg)
static bool unificar_arg(const char *consulta_arg, const char *hecho_arg, Bindings *bindings);

// Detecta si un string es una lista (empieza con '[' y termina con ']')
static bool es_lista(const char *s)
{
    if (!s)
        return false;
    while (*s == ' ')
        s++; // Saltar espacios iniciales
    if (*s != '[')
        return false;
    // Buscar el ']' final (ignorando espacios)
    size_t len = strlen(s);
    while (len > 0 && s[len - 1] == ' ')
        len--;
    return len > 0 && s[len - 1] == ']';
}

// Busca el '|' que separa cabeza de cola, respetando corchetes anidados.
// Solo considera válido el '|' que está al nivel 1 de corchetes
// (es decir, dentro del corchete externo pero fuera de corchetes internos).
// Ej: "[[H|T]|R]" → devuelve el segundo '|', no el primero.
// Retorna un puntero al '|' válido, o NULL si no hay.
static const char *buscar_pipe_cabeza_cola(const char *lista)
{
    if (!lista)
        return NULL;
    int nivel = 0;
    for (const char *p = lista; *p; p++)
    {
        if (*p == '[')
            nivel++;
        else if (*p == ']')
            nivel--;
        else if (*p == '|' && nivel == 1)
            return p;
    }
    return NULL;
}

// Detecta si una lista es el patrón [H|T] (cabeza|cola)
static bool es_patron_cabeza_cola(const char *s)
{
    if (!es_lista(s))
        return false;
    return buscar_pipe_cabeza_cola(s) != NULL;
}

// Detecta si es lista vacía "[]"
static bool es_lista_vacia(const char *s)
{
    if (!s)
        return false;
    while (*s == ' ')
        s++;
    if (*s != '[')
        return false;
    s++;
    while (*s == ' ')
        s++;
    return *s == ']';
}

// Normaliza una lista quitándole los espacios: "[a, b, c]" → "[a,b,c]"
// Respeta los espacios DENTRO de los elementos (por si son átomos compuestos)
static void normalizar_lista(const char *entrada, char *salida, size_t tam_salida)
{
    salida[0] = '\0';
    if (!entrada)
        return;

    size_t j = 0;
    for (size_t i = 0; entrada[i] && j < tam_salida - 1; i++)
    {
        char c = entrada[i];
        if (c == ' ' || c == '\t')
        {
            // Solo omitir espacios si NO estamos dentro de un elemento
            // (en esta primera versión simple, siempre omitimos)
            continue;
        }
        salida[j++] = c;
    }
    salida[j] = '\0';
}

// Divide una lista [H|T] en cabeza y cola
// Retorna true si es patrón [H|T], false si no
static bool dividir_cabeza_cola(const char *lista, char *cabeza, size_t tam_c,
                                char *cola, size_t tam_t)
{
    cabeza[0] = '\0';
    cola[0] = '\0';
    if (!es_lista(lista))
        return false;

    // Normalizar primero (quitar espacios)
    char norm[256];
    normalizar_lista(lista, norm, sizeof(norm));

    // Buscar el '|' al nivel 1 de corchetes (respeta anidamiento)
    const char *barra = buscar_pipe_cabeza_cola(norm);
    if (!barra)
        return false;

    // Cabeza: desde después de '[' hasta antes de '|'
    const char *inicio = norm + 1; // Saltar '['
    size_t len_c = barra - inicio;
    if (len_c >= tam_c)
        len_c = tam_c - 1;
    strncpy(cabeza, inicio, len_c);
    cabeza[len_c] = '\0';

    // Cola: desde después de '|' hasta antes de ']'
    const char *fin_cola = norm + strlen(norm) - 1; // Apunta a ']'
    size_t len_t = fin_cola - (barra + 1);
    if (len_t >= tam_t)
        len_t = tam_t - 1;
    strncpy(cola, barra + 1, len_t);
    cola[len_t] = '\0';

    return true;
}

// ============================================================
// RECOLECCIÓN DE VARIABLES DENTRO DE LISTAS
// ============================================================

// Prototipo forward (necesario porque recolectar_variables_en_arg llama a dividir_lista_en_elementos)
static int dividir_lista_en_elementos(const char *lista, char elems[][64], int max_elems);

// Recolecta todas las variables (que empiezan con mayúscula y no son "_")
// dentro de un argumento, incluyendo dentro de listas y patrones [H|T].
// Las agrega a vars[] si no están ya.
static void recolectar_variables_en_arg(const char *arg,
                                        char vars[][64], int *num_vars)
{
    if (!arg || *num_vars >= MAX_ARGS)
        return;

    // Caso 1: Es una variable simple (ej: "X", "Nombre")
    if (es_variable(arg) && strcmp(arg, "_") != 0)
    {
        // Verificar que no esté ya
        for (int j = 0; j < *num_vars; j++)
        {
            if (strcmp(vars[j], arg) == 0)
            {
                return; // Ya está
            }
        }
        if (*num_vars < MAX_ARGS)
        {
            strncpy(vars[*num_vars], arg, 63);
            vars[*num_vars][63] = '\0';
            (*num_vars)++;
        }
        return;
    }

    // Caso 2: Es una lista (concreta o patrón [H|T])
    if (es_lista(arg))
    {
        // Si es patrón [H|T], procesar cabeza y cola por separado
        if (es_patron_cabeza_cola(arg))
        {
            char cabeza[256], cola[256];  // Aumentado para cabezas múltiples
            if (dividir_cabeza_cola(arg, cabeza, sizeof(cabeza), cola, sizeof(cola)))
            {
                recolectar_variables_en_arg(cabeza, vars, num_vars);
                recolectar_variables_en_arg(cola, vars, num_vars);
            }
            return;
        }

        // Lista concreta: recorrer cada elemento
        if (es_lista_vacia(arg))
        {
            return;
        }

        char elems[64][64];
        int n = dividir_lista_en_elementos(arg, elems, 64);
        for (int i = 0; i < n; i++)
        {
            recolectar_variables_en_arg(elems[i], vars, num_vars);
        }
        return;
    }

    // Caso 3: String con comas al nivel 0 (ej: "X,Y" o "X,[A,B]")
    // Esto ocurre cuando la cabeza de un patrón [H|T] tiene múltiples elementos.
    // Dividimos por comas al nivel 0 y procesamos cada elemento recursivamente.
    {
        int nivel_c = 0, nivel_p = 0;
        bool tiene_coma_nivel_0 = false;
        for (const char *p = arg; *p; p++)
        {
            if (*p == '[')
                nivel_c++;
            else if (*p == ']')
                nivel_c--;
            else if (*p == '(')
                nivel_p++;
            else if (*p == ')')
                nivel_p--;
            else if (*p == ',' && nivel_c == 0 && nivel_p == 0)
            {
                tiene_coma_nivel_0 = true;
                break;
            }
        }
        if (tiene_coma_nivel_0)
        {
            // Dividir por comas al nivel 0
            const char *inicio = arg;
            nivel_c = 0;
            nivel_p = 0;
            for (const char *p = arg;; p++)
            {
                bool es_fin = (*p == '\0');
                bool es_coma = (*p == ',' && nivel_c == 0 && nivel_p == 0);
                if (es_fin || es_coma)
                {
                    size_t len = p - inicio;
                    char elem[256];
                    if (len >= sizeof(elem))
                        len = sizeof(elem) - 1;
                    strncpy(elem, inicio, len);
                    elem[len] = '\0';
                    // Trim espacios
                    char *q = elem;
                    while (*q == ' ')
                        q++;
                    char *fin = q + strlen(q) - 1;
                    while (fin > q && *fin == ' ')
                    {
                        *fin = '\0';
                        fin--;
                    }
                    if (q != elem)
                        memmove(elem, q, strlen(q) + 1);
                    // Procesar recursivamente
                    if (strlen(elem) > 0)
                        recolectar_variables_en_arg(elem, vars, num_vars);
                    if (es_fin)
                        break;
                    inicio = p + 1;
                }
                else
                {
                    if (*p == '[')
                        nivel_c++;
                    else if (*p == ']')
                        nivel_c--;
                    else if (*p == '(')
                        nivel_p++;
                    else if (*p == ')')
                        nivel_p--;
                }
            }
            return;
        }
    }
}

// ============================================================
// FUNCIONES AUXILIARES PARA UNIFICACIÓN DE LISTAS
// ============================================================

// Divide una lista [a,b,c] en sus elementos: ["a", "b", "c"]
// Retorna el número de elementos, o -1 si no es lista válida
// Los elementos se guardan en el array elems[] (cada uno de hasta 64 chars)
static int dividir_lista_en_elementos(const char *lista,
                                      char elems[][64], int max_elems)
{
    if (!es_lista(lista))
        return -1;
    if (es_lista_vacia(lista))
        return 0;
    if (es_patron_cabeza_cola(lista))
        return -1; // No se puede dividir [H|T]

    char norm[256];
    normalizar_lista(lista, norm, sizeof(norm));

    int count = 0;
    const char *inicio_elem = norm + 1; // Saltar '['
    int nivel = 0;

    for (size_t i = 1; norm[i] && count < max_elems; i++)
    {
        if (norm[i] == '[')
            nivel++;
        else if (norm[i] == ']')
        {
            if (nivel == 0)
            {
                // Último elemento
                size_t len = (norm + i) - inicio_elem;
                if (len >= 64)
                    len = 63;
                strncpy(elems[count], inicio_elem, len);
                elems[count][len] = '\0';
                // Trim espacios
                char *q = elems[count];
                while (*q == ' ')
                    q++;
                if (q != elems[count])
                    memmove(elems[count], q, strlen(q) + 1);
                char *fin = elems[count] + strlen(elems[count]) - 1;
                while (fin > elems[count] && *fin == ' ')
                {
                    *fin = '\0';
                    fin--;
                }
                count++;
                break;
            }
            nivel--;
        }
        else if (norm[i] == ',' && nivel == 0)
        {
            // Elemento intermedio
            size_t len = (norm + i) - inicio_elem;
            if (len >= 64)
                len = 63;
            strncpy(elems[count], inicio_elem, len);
            elems[count][len] = '\0';
            // Trim
            char *q = elems[count];
            while (*q == ' ')
                q++;
            if (q != elems[count])
                memmove(elems[count], q, strlen(q) + 1);
            char *fin = elems[count] + strlen(elems[count]) - 1;
            while (fin > elems[count] && *fin == ' ')
            {
                *fin = '\0';
                fin--;
            }
            count++;
            inicio_elem = norm + i + 1;
        }
    }
    return count;
}

// Construye una lista a partir de un array de elementos
// Ej: ["a", "b", "c"] → "[a,b,c]"
// Si num_elems == 0, devuelve "[]"
static void construir_lista(char *salida, size_t tam_salida,
                            char elems[][64], int num_elems)
{
    salida[0] = '\0';
    if (num_elems <= 0)
    {
        strncpy(salida, "[]", tam_salida - 1);
        return;
    }

    size_t pos = 0;
    if (pos < tam_salida - 1)
        salida[pos++] = '[';

    for (int i = 0; i < num_elems && pos < tam_salida - 2; i++)
    {
        if (i > 0 && pos < tam_salida - 1)
            salida[pos++] = ',';
        size_t len = strlen(elems[i]);
        if (pos + len >= tam_salida - 1)
            break;
        memcpy(salida + pos, elems[i], len);
        pos += len;
    }

    if (pos < tam_salida - 1)
        salida[pos++] = ']';
    salida[pos] = '\0';
}

// Construye una sublista desde el índice 'desde' hasta el final
// Ej: sublista_desde("[a,b,c,d]", 2, ...) → "[c,d]"
static void sublista_desde(const char *lista, int desde,
                           char *salida, size_t tam_salida)
{
    char elems[64][64];
    int total = dividir_lista_en_elementos(lista, elems, 64);
    if (total < 0 || desde < 0 || desde > total)
    {
        strncpy(salida, "[]", tam_salida - 1);
        salida[tam_salida - 1] = '\0';
        return;
    }

    char sub[64][64];
    int num_sub = 0;
    for (int i = desde; i < total && num_sub < 64; i++)
    {
        strncpy(sub[num_sub], elems[i], 63);
        sub[num_sub][63] = '\0';
        num_sub++;
    }
    construir_lista(salida, tam_salida, sub, num_sub);
}

// Cuenta cuántos elementos tiene una "cabeza" de patrón [H|T].
// Si no tiene comas al nivel 0, retorna 1 (es un solo elemento).
// Si tiene comas, retorna la cantidad de elementos.
// Ej: "a" → 1, "a,b,c" → 3, "[H|T],X" → 2
static int contar_elementos_cabeza(const char *cabeza)
{
    if (!cabeza || !*cabeza)
        return 0;
    int nivel_corchete = 0, nivel_paren = 0;
    int count = 1;
    for (const char *p = cabeza; *p; p++)
    {
        if (*p == '[')
            nivel_corchete++;
        else if (*p == ']')
            nivel_corchete--;
        else if (*p == '(')
            nivel_paren++;
        else if (*p == ')')
            nivel_paren--;
        else if (*p == ',' && nivel_corchete == 0 && nivel_paren == 0)
            count++;
    }
    return count;
}

// Divide una cabeza (que puede tener comas al nivel 0) en sus elementos.
// Ej: "a,b,c" → ["a", "b", "c"]
// Ej: "H" → ["H"]
// Ej: "[H|T],X" → ["[H|T]", "X"]
static int dividir_cabeza_en_elementos(const char *cabeza, char elems[][64], int max_elems)
{
    if (!cabeza || !*cabeza)
        return 0;

    // Caso rápido: un solo elemento (sin comas al nivel 0)
    if (contar_elementos_cabeza(cabeza) == 1)
    {
        strncpy(elems[0], cabeza, 63);
        elems[0][63] = '\0';
        // Trim espacios
        char *q = elems[0];
        while (*q == ' ')
            q++;
        if (q != elems[0])
            memmove(elems[0], q, strlen(q) + 1);
        char *fin = elems[0] + strlen(elems[0]) - 1;
        while (fin > elems[0] && *fin == ' ')
        {
            *fin = '\0';
            fin--;
        }
        return 1;
    }

    // Caso general: dividir por comas al nivel 0
    int count = 0;
    const char *inicio = cabeza;
    int nivel_corchete = 0, nivel_paren = 0;
    for (const char *p = cabeza;; p++)
    {
        bool es_fin = (*p == '\0');
        bool es_coma = (*p == ',' && nivel_corchete == 0 && nivel_paren == 0);
        if (es_fin || es_coma)
        {
            size_t len = p - inicio;
            if (len >= 64)
                len = 63;
            strncpy(elems[count], inicio, len);
            elems[count][len] = '\0';
            // Trim espacios
            char *q = elems[count];
            while (*q == ' ')
                q++;
            if (q != elems[count])
                memmove(elems[count], q, strlen(q) + 1);
            char *fin = elems[count] + strlen(elems[count]) - 1;
            while (fin > elems[count] && *fin == ' ')
            {
                *fin = '\0';
                fin--;
            }
            count++;
            if (es_fin || count >= max_elems)
                break;
            inicio = p + 1;
        }
        else
        {
            if (*p == '[')
                nivel_corchete++;
            else if (*p == ']')
                nivel_corchete--;
            else if (*p == '(')
                nivel_paren++;
            else if (*p == ')')
                nivel_paren--;
        }
    }
    return count;
}

// Función principal de unificación de listas
// Retorna true si unifican, false si no
// Modifica 'bindings' con las nuevas ligaduras
static bool unificar_listas(const char *consulta, const char *hecho, Bindings *bindings)
{
    // Caso 1: Ambos son listas concretas (sin patrón [H|T])
    bool consulta_concreta = es_lista(consulta) && !es_patron_cabeza_cola(consulta);
    bool hecho_concreto = es_lista(hecho) && !es_patron_cabeza_cola(hecho);

    if (consulta_concreta && hecho_concreto)
    {
        char elems_c[64][64], elems_h[64][64];
        int n_c = dividir_lista_en_elementos(consulta, elems_c, 64);
        int n_h = dividir_lista_en_elementos(hecho, elems_h, 64);

        if (n_c != n_h)
            return false; // Diferente longitud → falla

        // Unificar elemento por elemento (recursivamente)
        for (int i = 0; i < n_c; i++)
        {
            if (!unificar_arg(elems_c[i], elems_h[i], bindings))
                return false;
        }
        return true;
    }

    // Caso 2: Consulta es patrón [H|T] (con cabeza simple o múltiple), hecho es lista concreta
    // Soporta: [H|T], [a,b|R], [a,X,c|R], [[H|T],X|R], etc.
    if (es_patron_cabeza_cola(consulta) && hecho_concreto)
    {
        char cabeza_c[256], cola_c[256]; // Aumentado para soportar cabezas múltiples
        if (!dividir_cabeza_cola(consulta, cabeza_c, sizeof(cabeza_c), cola_c, sizeof(cola_c)))
        {
            return false;
        }
        char elems_h[64][64];
        int n_h = dividir_lista_en_elementos(hecho, elems_h, 64);
        if (n_h < 0)
        {
            return false;
        }
        if (n_h == 0)
        {
            return false; // Lista vacía no puede unificar con patrón
        }

        // Dividir la cabeza en sus elementos (puede ser 1 o varios)
        char elems_cabeza[64][64];
        int n_cabeza = dividir_cabeza_en_elementos(cabeza_c, elems_cabeza, 64);

        // La cabeza no puede tener más elementos que el hecho
        if (n_cabeza > n_h)
        {
            return false;
        }

        // Unificar cada elemento de la cabeza con el correspondiente del hecho
        for (int i = 0; i < n_cabeza; i++)
        {
            if (!unificar_arg(elems_cabeza[i], elems_h[i], bindings))
                return false;
        }

        // Construir la sublista restante (desde el índice n_cabeza en adelante)
        char resto[256];
        sublista_desde(hecho, n_cabeza, resto, sizeof(resto));

        // Unificar la cola del patrón con el resto
        if (es_lista(resto) &&
            (es_patron_cabeza_cola(cola_c) || es_lista(cola_c)))
        {
            if (!unificar_listas(cola_c, resto, bindings))
                return false;
        }
        else
        {
            if (!unificar_arg(cola_c, resto, bindings))
                return false;
        }
        return true;
    }

    // Caso 3: Hecho es patrón [H|T], consulta es lista concreta
    if (consulta_concreta && es_patron_cabeza_cola(hecho))
    {
        // Simétrico al caso 2
        return unificar_listas(hecho, consulta, bindings);
    }

    // Caso 4: Ambos son patrones [H|T] - caso avanzado, lo simplificamos
    // Por ahora, si ambos son patrones, comparamos estructuralmente
    if (es_patron_cabeza_cola(consulta) && es_patron_cabeza_cola(hecho))
    {
        char c_c[256], t_c[256], c_h[256], t_h[256];
        dividir_cabeza_cola(consulta, c_c, sizeof(c_c), t_c, sizeof(t_c));
        dividir_cabeza_cola(hecho, c_h, sizeof(c_h), t_h, sizeof(t_h));

        if (!unificar_arg(c_c, c_h, bindings))
            return false;
        if (!unificar_arg(t_c, t_h, bindings))
            return false;
        return true;
    }

    return false;
}

// Parsea "predicado(arg1, arg2, ...)" en componentes
// RESPETA corchetes: no divide por comas dentro de listas
static bool parsear_predicado(const char *str, char *predicado,
                              char args[][64], int *num_args)
{
    *num_args = 0;
    const char *paren = strchr(str, '(');
    if (!paren)
    {
        strncpy(predicado, str, 63);
        predicado[63] = '\0';
        return true;
    }
    int len_pred = paren - str;
    if (len_pred >= 64)
        len_pred = 63;
    strncpy(predicado, str, len_pred);
    predicado[len_pred] = '\0';

    const char *cierre = strrchr(str, ')'); // strrchr para tomar el último ')'
    if (!cierre)
        return false;

    const char *inicio = paren + 1;
    const char *p = inicio;
    int nivel_corchete = 0;
    int nivel_paren = 0;

    while (p <= cierre && *num_args < MAX_ARGS)
    {
        if (*p == '[')
            nivel_corchete++;
        else if (*p == ']')
            nivel_corchete--;
        else if (*p == '(')
            nivel_paren++;
        else if (*p == ')')
            nivel_paren--;

        // Separador: coma al nivel 0 (fuera de corchetes y paréntesis)
        if ((*p == ',' && nivel_corchete == 0 && nivel_paren == 0) || p == cierre)
        {
            const char *fin = p;
            int len = fin - inicio;

            // Copiar el argumento (incluyendo corchetes si es una lista)
            char temp[128];
            if (len >= (int)sizeof(temp))
                len = sizeof(temp) - 1;
            strncpy(temp, inicio, len);
            temp[len] = '\0';

            // Limpiar espacios al inicio y final
            char *q = temp;
            while (*q == ' ')
                q++;
            char *fin_str = q + strlen(q) - 1;
            while (fin_str > q && *fin_str == ' ')
            {
                *fin_str = '\0';
                fin_str--;
            }

            // Guardar en args (si es lista, normalizar espacios)
            if (es_lista(q))
            {
                char norm[64];
                normalizar_lista(q, norm, sizeof(norm));
                strncpy(args[*num_args], norm, 63);
                args[*num_args][63] = '\0';
            }
            else
            {
                strncpy(args[*num_args], q, 63);
                args[*num_args][63] = '\0';
            }
            (*num_args)++;

            if (*p == ',')
                inicio = p + 1;
            else
                break;
        }
        p++;
    }
    return true;
}

static bool parsear_condicion_aritmetica(const char *str, Condicion *cond)
{
    if (!str || !cond)
        return false;

    // Buscar operadores como palabras completas (sin depender de espacios exactos al final)
    const char *ops[] = {"MAYOR IGUAL", "MENOR IGUAL", "MAYOR", "MENOR", "IGUAL", "DIFERENTE", "ES"};
    const int ops_lens[] = {11, 11, 5, 5, 5, 9, 2};
    const char *encontrado = NULL;
    int op_idx = -1;

    for (int i = 0; i < 7; i++)
    {
        const char *temp = str;
        while ((temp = strstr(temp, ops[i])) != NULL)
        {
            // Verificar que es una palabra completa (no parte de otra palabra)
            bool inicio_ok = (temp == str || !isalnum((unsigned char)*(temp - 1)));
            bool fin_ok = (!isalnum((unsigned char)*(temp + ops_lens[i])));
            if (inicio_ok && fin_ok)
            {
                encontrado = temp;
                op_idx = i;
                goto encontrado_op;
            }
            temp += ops_lens[i];
        }
    }

encontrado_op:
    if (!encontrado || op_idx < 0)
    {
        return false;
    }

    // Extraer lado izquierdo
    int len_izq = encontrado - str;
    char lado_izq[64];
    if (len_izq >= 64)
        len_izq = 63;
    strncpy(lado_izq, str, len_izq);
    lado_izq[len_izq] = '\0';

    // Limpiar espacios al inicio y final del lado izquierdo
    char *p = lado_izq + strlen(lado_izq) - 1;
    while (p > lado_izq && *p == ' ')
        *p-- = '\0';
    char *q = lado_izq;
    while (*q == ' ')
        q++;
    if (q != lado_izq)
        memmove(lado_izq, q, strlen(q) + 1);

    // Extraer lado derecho
    const char *lado_der_str = encontrado + ops_lens[op_idx];
    while (*lado_der_str == ' ')
        lado_der_str++;

    char lado_der[64];
    strncpy(lado_der, lado_der_str, 63);
    lado_der[63] = '\0';
    p = lado_der + strlen(lado_der) - 1;
    while (p > lado_der && *p == ' ')
        *p-- = '\0';

    cond->es_aritmetica = true;
    // Guardar operador CON un espacio al final para compatibilidad con evaluar_condicion_aritmetica
    snprintf(cond->operador, 16, "%s ", ops[op_idx]);
    strncpy(cond->predicado, "ARITMETICA", 63);
    cond->predicado[63] = '\0';
    cond->num_args = 2;
    strncpy(cond->args[0], lado_izq, 63);
    cond->args[0][63] = '\0';
    strncpy(cond->args[1], lado_der, 63);
    cond->args[1][63] = '\0';
    return true;
}

bool bc_agregar_hecho_desde_string(BaseConocimiento *bc, const char *str)
{
    if (!bc || !str)
        return false;
    if (bc->num_hechos >= MAX_HECHOS)
        return false;

    Hecho *h = &bc->hechos[bc->num_hechos];
    h->orden = contador_orden_global++;
    if (!parsear_predicado(str, h->predicado, h->args, &h->num_args))
    {
        return false;
    }
    bc->num_hechos++;
    return true;
}

bool bc_agregar_regla_desde_string(BaseConocimiento *bc, const char *str)
{
    if (!bc || !str)
        return false;
    if (bc->num_reglas >= MAX_REGLAS)
        return false;

    const char *si_pos = strstr(str, " SI ");
    const char *neck_pos = strstr(str, " :- ");
    const char *separador = NULL;
    int len_separador = 0;
    bool es_neck = false;

    if (si_pos && neck_pos)
    {
        if (si_pos < neck_pos)
        {
            separador = si_pos;
            len_separador = 4;
            es_neck = false;
        }
        else
        {
            separador = neck_pos;
            len_separador = 4;
            es_neck = true;
        }
    }
    else if (si_pos)
    {
        separador = si_pos;
        len_separador = 4;
        es_neck = false;
    }
    else if (neck_pos)
    {
        separador = neck_pos;
        len_separador = 4;
        es_neck = true;
    }

    if (!separador)
        return bc_agregar_hecho_desde_string(bc, str);

    // Extraer la cabeza de la regla
    int len_cabeza = separador - str;
    char cabeza_str[256];
    if (len_cabeza >= 256)
        len_cabeza = 255;
    strncpy(cabeza_str, str, len_cabeza);
    cabeza_str[len_cabeza] = '\0';

    // Extraer el cuerpo de la regla
    const char *cuerpo_str = separador + len_separador;
    
    // ============================================================
    // DETECTAR DISYUNCIÓN " O " o ";" EN EL CUERPO
    // ============================================================
    int nivel_paren = 0, nivel_corchete = 0;
    const char *disyuncion = NULL;
    
    for (const char *p = cuerpo_str; *p; p++)
    {
        if (*p == '(')
            nivel_paren++;
        else if (*p == ')')
            nivel_paren--;
        else if (*p == '[')
            nivel_corchete++;
        else if (*p == ']')
            nivel_corchete--;
        else if (nivel_paren == 0 && nivel_corchete == 0)
        {
            // Buscar " O " (con espacios)
            if (*p == ' ' && *(p+1) == 'O' && *(p+2) == ' ')
            {
                disyuncion = p + 1; // Apunta a la 'O'
                break;
            }
            // Buscar ";"
            else if (*p == ';')
            {
                disyuncion = p;
                break;
            }
        }
    }
    
    // Si hay disyunción, crear múltiples reglas
    if (disyuncion)
    {
        char cuerpo1[256], cuerpo2[256];
        int len1 = disyuncion - cuerpo_str;
        
        // Copiar primera parte del cuerpo
        if (len1 >= 256)
            len1 = 255;
        strncpy(cuerpo1, cuerpo_str, len1);
        cuerpo1[len1] = '\0';
        
        // Trim espacios al final de cuerpo1
        char *p = cuerpo1 + strlen(cuerpo1) - 1;
        while (p > cuerpo1 && *p == ' ')
        {
            *p = '\0';
            p--;
        }
        
        // Copiar segunda parte del cuerpo (saltando " O " o ";")
        const char *inicio_cuerpo2 = disyuncion;
        if (*disyuncion == 'O')
        {
            inicio_cuerpo2 += 2; // Saltar "O "
        }
        else if (*disyuncion == ';')
        {
            inicio_cuerpo2 += 1; // Saltar ";"
        }
        
        // Saltar espacios iniciales
        while (*inicio_cuerpo2 == ' ')
            inicio_cuerpo2++;
        
        strncpy(cuerpo2, inicio_cuerpo2, 255);
        cuerpo2[255] = '\0';
        
        // Trim espacios al final de cuerpo2
        p = cuerpo2 + strlen(cuerpo2) - 1;
        while (p > cuerpo2 && *p == ' ')
        {
            *p = '\0';
            p--;
        }
        
        // Reconstruir las dos reglas completas
        char regla1[512], regla2[512];
        snprintf(regla1, sizeof(regla1), "%s %s %s", 
                 cabeza_str, es_neck ? ":-" : "SI", cuerpo1);
        snprintf(regla2, sizeof(regla2), "%s %s %s", 
                 cabeza_str, es_neck ? ":-" : "SI", cuerpo2);
        
        // Agregar ambas reglas recursivamente
        bc_agregar_regla_desde_string(bc, regla1);
        bc_agregar_regla_desde_string(bc, regla2);
        return true;
    }

    // ============================================================
    // NO HAY DISYUNCIÓN: PROCESAR REGLA NORMAL
    // ============================================================
    Regla *r = &bc->reglas[bc->num_reglas];
    memset(r, 0, sizeof(Regla));
    r->orden = contador_orden_global++;
    r->usa_neck = es_neck;

    if (!parsear_predicado(cabeza_str, r->nombre, r->cabeza_args, &r->num_cabeza))
        return false;

    nivel_paren = 0;
    nivel_corchete = 0;
    const char *inicio = cuerpo_str;
    const char *p = cuerpo_str;

    while (*p && r->num_cuerpo < MAX_CONDICIONES)
    {
        if (*p == '(')
            nivel_paren++;
        else if (*p == ')')
            nivel_paren--;
        else if (*p == '[')
            nivel_corchete++;
        else if (*p == ']')
            nivel_corchete--;
        else if (*p == ',' && nivel_paren == 0 && nivel_corchete == 0)
        {
            int len = p - inicio;
            char token[256];
            if (len >= 256)
                len = 255;
            strncpy(token, inicio, len);
            token[len] = '\0';

            char *q = token;
            while (*q == ' ')
                q++;
            char *fin = q + strlen(q) - 1;
            while (fin > q && *fin == ' ')
                *fin-- = '\0';

            Condicion *c = &r->cuerpo[r->num_cuerpo];
            c->negacion = false;
            c->es_aritmetica = false;
            c->es_corte = false;

            // 1. DETECTAR CORTE
            if (strcmp(q, "!") == 0 || strcmp(q, "CORTE") == 0)
            {
                c->es_corte = true;
                strncpy(c->predicado, "CORTE", 63);
                c->predicado[63] = '\0';
                c->num_args = 0;
                r->num_cuerpo++;
                inicio = p + 1;
                p++;
                continue;
            }

            // 2. PROCESAMIENTO NORMAL
            if (strncmp(q, "NO ", 3) == 0)
            {
                c->negacion = true;
                q += 3;
                while (*q == ' ')
                    q++;
            }

            // Detectar unificación directa "X = Y" en el cuerpo de la regla
            char *igual_pos = strstr(q, " = ");
            if (igual_pos)
            {
                bool es_unificacion = true;
                if (igual_pos > q && (*(igual_pos - 1) == '>' || *(igual_pos - 1) == '<' || *(igual_pos - 1) == '!' || *(igual_pos - 1) == '='))
                    es_unificacion = false;
                if (*(igual_pos + 2) == '=')
                    es_unificacion = false;

                if (es_unificacion)
                {
                    c->es_aritmetica = false;
                    c->num_args = 2;
                    int len_izq = igual_pos - q;
                    strncpy(c->args[0], q, len_izq);
                    c->args[0][len_izq] = '\0';
                    strcpy(c->args[1], igual_pos + 3);

                    // Trim espacios
                    char *p_trim = c->args[0] + strlen(c->args[0]) - 1;
                    while (p_trim > c->args[0] && *p_trim == ' ')
                        *p_trim-- = '\0';
                    char *q_trim = c->args[0];
                    while (*q_trim == ' ')
                        q_trim++;
                    if (q_trim != c->args[0])
                        memmove(c->args[0], q_trim, strlen(q_trim) + 1);

                    p_trim = c->args[1] + strlen(c->args[1]) - 1;
                    while (p_trim > c->args[1] && *p_trim == ' ')
                        *p_trim-- = '\0';
                    q_trim = c->args[1];
                    while (*q_trim == ' ')
                        q_trim++;
                    if (q_trim != c->args[1])
                        memmove(c->args[1], q_trim, strlen(q_trim) + 1);

                    strcpy(c->predicado, "=");
                }
                else if (!parsear_condicion_aritmetica(q, c))
                {
                    parsear_predicado(q, c->predicado, c->args, &c->num_args);
                }
            }
            else if (!parsear_condicion_aritmetica(q, c))
            {
                parsear_predicado(q, c->predicado, c->args, &c->num_args);
            }

            r->num_cuerpo++;
            inicio = p + 1;
        }
        p++;
    }

    // ÚLTIMA CONDICIÓN
    if (inicio < p && r->num_cuerpo < MAX_CONDICIONES)
    {
        char token[256];
        strncpy(token, inicio, 255);
        token[255] = '\0';
        char *q = token;
        while (*q == ' ')
            q++;
        char *fin = q + strlen(q) - 1;
        while (fin > q && *fin == ' ')
            *fin-- = '\0';

        Condicion *c = &r->cuerpo[r->num_cuerpo];
        c->negacion = false;
        c->es_aritmetica = false;
        c->es_corte = false;

        if (strcmp(q, "!") == 0 || strcmp(q, "CORTE") == 0)
        {
            c->es_corte = true;
            strncpy(c->predicado, "CORTE", 63);
            c->predicado[63] = '\0';
            c->num_args = 0;
            r->num_cuerpo++;
        }
        else
        {
            if (strncmp(q, "NO ", 3) == 0)
            {
                c->negacion = true;
                q += 3;
                while (*q == ' ')
                    q++;
            }

            // Detectar unificación directa "X = Y" en el cuerpo de la regla
            char *igual_pos = strstr(q, " = ");
            if (igual_pos)
            {
                bool es_unificacion = true;
                if (igual_pos > q && (*(igual_pos - 1) == '>' || *(igual_pos - 1) == '<' || *(igual_pos - 1) == '!' || *(igual_pos - 1) == '='))
                    es_unificacion = false;
                if (*(igual_pos + 2) == '=')
                    es_unificacion = false;

                if (es_unificacion)
                {
                    c->es_aritmetica = false;
                    c->num_args = 2;
                    int len_izq = igual_pos - q;
                    strncpy(c->args[0], q, len_izq);
                    c->args[0][len_izq] = '\0';
                    strcpy(c->args[1], igual_pos + 3);

                    // Trim espacios
                    char *p_trim = c->args[0] + strlen(c->args[0]) - 1;
                    while (p_trim > c->args[0] && *p_trim == ' ')
                        *p_trim-- = '\0';
                    char *q_trim = c->args[0];
                    while (*q_trim == ' ')
                        q_trim++;
                    if (q_trim != c->args[0])
                        memmove(c->args[0], q_trim, strlen(q_trim) + 1);

                    p_trim = c->args[1] + strlen(c->args[1]) - 1;
                    while (p_trim > c->args[1] && *p_trim == ' ')
                        *p_trim-- = '\0';
                    q_trim = c->args[1];
                    while (*q_trim == ' ')
                        q_trim++;
                    if (q_trim != c->args[1])
                        memmove(c->args[1], q_trim, strlen(q_trim) + 1);

                    strcpy(c->predicado, "=");
                }
                else if (!parsear_condicion_aritmetica(q, c))
                {
                    parsear_predicado(q, c->predicado, c->args, &c->num_args);
                }
            }
            else if (!parsear_condicion_aritmetica(q, c))
            {
                parsear_predicado(q, c->predicado, c->args, &c->num_args);
            }

            r->num_cuerpo++;
        }
    }
    bc->num_reglas++;
    return true;
}

// ============================================================
// BINDINGS
// ============================================================
static void bindings_init(Bindings *b)
{
    b->count = 0;
}

static const char *bindings_buscar(const Bindings *b, const char *var)
{
    for (int i = 0; i < b->count; i++)
    {
        if (strcmp(b->items[i].variable, var) == 0)
        {
            return b->items[i].valor;
        }
    }
    return NULL;
}

static bool bindings_agregar(Bindings *b, const char *var, const char *valor)
{
    if (b->count >= MAX_BINDINGS)
        return false;

    strncpy(b->items[b->count].variable, var, 63);
    b->items[b->count].variable[63] = '\0';

    strncpy(b->items[b->count].valor, valor, 63);
    b->items[b->count].valor[63] = '\0';

    b->count++;

    return true;
}

// Resuelve una variable: si está en bindings, devuelve su valor; si no, devuelve el nombre original
static const char *resolver_var(const Bindings *b, const char *nombre)
{
    if (!es_variable(nombre))
        return nombre;
    const char *val = bindings_buscar(b, nombre);
    if (val)
    {
        // Evitar autoreferencias Y -> Y
        if (strcmp(val, nombre) == 0)
            return nombre;
        // Resolución recursiva (por si el valor es otra variable)
        return resolver_var(b, val);
    }
    return nombre;
}

// ============================================================
// OCCURS CHECK - Verifica si una variable ocurre dentro de un término
// ============================================================
// Evita unificaciones circulares como X = [X] o X = [a, X]
static bool ocurre_en(const char *var, const char *term, const Bindings *bindings)
{
    if (!var || !term)
        return false;

    // 1. Resolver el término actual (por si es una variable ya ligada)
    const char *t_resuelta = resolver_var(bindings, term);

    // 2. Si el término resuelto es exactamente la variable que buscamos, ¡ocurre!
    if (strcmp(var, t_resuelta) == 0)
        return true;

    // 3. Si el término resuelto es una lista, buscar recursivamente dentro de ella
    if (es_lista(t_resuelta))
    {
        // Caso 3a: Es un patrón [H|T]
        if (es_patron_cabeza_cola(t_resuelta))
        {
            char cabeza[256], cola[256];
            if (dividir_cabeza_cola(t_resuelta, cabeza, sizeof(cabeza), cola, sizeof(cola)))
            {
                if (ocurre_en(var, cabeza, bindings))
                    return true;
                if (ocurre_en(var, cola, bindings))
                    return true;
            }
        }
        // Caso 3b: Es una lista concreta [a, b, c]
        else if (!es_lista_vacia(t_resuelta))
        {
            char elems[64][64];
            int n = dividir_lista_en_elementos(t_resuelta, elems, 64);
            for (int i = 0; i < n; i++)
            {
                if (ocurre_en(var, elems[i], bindings))
                    return true;
            }
        }
    }

    // 4. Si es un átomo simple y no es la variable, entonces no ocurre.
    return false;
}

// Intenta unificar un argumento de consulta con un argumento de hecho/regla
// Devuelve true si unifica, false si no
// Con soporte para listas y variable anónima _
static bool unificar_arg(const char *consulta_arg, const char *hecho_arg,
                         Bindings *bindings)
{
    // Resolver variables en la consulta
    const char *c = resolver_var(bindings, consulta_arg);
    // Resolver variables en el hecho
    const char *h = resolver_var(bindings, hecho_arg);

    // === CASO ESPECIAL: VARIABLE ANÓNIMA _ ===
    // _ unifica con cualquier cosa sin crear binding
    if (strcmp(c, "_") == 0 || strcmp(h, "_") == 0)
    {
        return true; // Siempre unifica exitosamente
    }
    // === FIN CASO VARIABLE ANÓNIMA ===

    // === CASO ESPECIAL: LISTAS ===
    // Si alguno de los dos es una lista, usar unificación de listas
    bool c_es_lista = es_lista(c);
    bool h_es_lista = es_lista(h);

    if (c_es_lista || h_es_lista)
    {
        // Si ambos son listas (concretas o patrones [H|T])
        if (c_es_lista && h_es_lista)
        {
            return unificar_listas(c, h, bindings);
        }
        // Si uno es lista y el otro es variable libre → ligar (CON OCCURS CHECK)
        if (c_es_lista && es_variable(h) && !bindings_buscar(bindings, h))
        {
            if (ocurre_en(h, c, bindings))
                return false;
            return bindings_agregar(bindings, h, c);
        }
        if (h_es_lista && es_variable(c) && !bindings_buscar(bindings, c))
        {
            if (ocurre_en(c, h, bindings))
                return false;
            return bindings_agregar(bindings, c, h);
        }
        // Si uno es lista y el otro es constante → falla
        return false;
    }
    // === FIN CASO LISTAS ===

    // Si ambos son la misma constante
    if (!es_variable(c) && !es_variable(h))
    {
        return strcmp(c, h) == 0;
    }

    if (strcmp(c, h) == 0)
    {
        return true;
    }

    // Ambas variables ya son la misma: no crear binding
    if (es_variable(c) && es_variable(h) && strcmp(c, h) == 0)
    {
        return true;
    }

    // Si la consulta es variable libre, ligarla (CON OCCURS CHECK)
    if (es_variable(c) && !bindings_buscar(bindings, c))
    {
        if (ocurre_en(c, h, bindings))
            return false; // ¡Falla! La variable ocurre dentro del término (ej: X = [X])
        return bindings_agregar(bindings, c, h);
    }
    // Si el hecho es variable libre, ligarla (CON OCCURS CHECK)
    if (es_variable(h) && !bindings_buscar(bindings, h))
    {
        if (ocurre_en(h, c, bindings))
            return false; // ¡Falla! La variable ocurre dentro del término
        return bindings_agregar(bindings, h, c);
    }

    // Ambos resueltos: comparar
    return strcmp(c, h) == 0;
}

// ============================================================
// MOTOR DE INFERENCIA
// ============================================================
// Busca un predicado en hechos y reglas, devolviendo todas las soluciones
// resultado: array de Bindings (una por solución)
// num_resultados: cantidad de soluciones encontradas

static int resolver(BaseConocimiento *bc, const char *predicado,
                    char args[][64], int num_args,
                    Bindings *bindings_actual,
                    Bindings *resultados, int max_resultados, void *contexto,
                    bool *corte_activo);

// Función auxiliar para evaluar condiciones aritméticas
extern double evaluar_expresion_logica(const char *expr, Bindings *bindings, void *contexto);

static bool evaluar_condicion_aritmetica(Condicion *cond, Bindings *bindings, void *contexto)
{
    if (!cond->es_aritmetica)
        return false;

    char lado_izq[64], lado_der[64];
    strncpy(lado_izq, resolver_var(bindings, cond->args[0]), 63);
    lado_izq[63] = '\0';
    strncpy(lado_der, resolver_var(bindings, cond->args[1]), 63);
    lado_der[63] = '\0';

    double val_izq = evaluar_expresion_logica(lado_izq, bindings, contexto);
    double val_der = evaluar_expresion_logica(lado_der, bindings, contexto);

    if (strcmp(cond->operador, "MAYOR ") == 0)
        return val_izq > val_der;
    if (strcmp(cond->operador, "MENOR ") == 0)
        return val_izq < val_der;
    if (strcmp(cond->operador, "MAYOR IGUAL ") == 0)
        return val_izq >= val_der;
    if (strcmp(cond->operador, "MENOR IGUAL ") == 0)
        return val_izq <= val_der;
    if (strcmp(cond->operador, "IGUAL ") == 0)
        return val_izq == val_der;
    if (strcmp(cond->operador, "DIFERENTE ") == 0)
        return val_izq != val_der;

    if (strcmp(cond->operador, "ES ") == 0)
    {
        if (es_variable(cond->args[0]))
        {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%.15g", val_der);
            return bindings_agregar(bindings, cond->args[0], buffer);
        }
        return false;
    }
    return false;
}

// ============================================================
// RENOMBRADO INTELIGENTE DE VARIABLES EN EXPRESIONES
// ============================================================
static void renombrar_variables_en_expresion(const char *expr, int sufijo, char *out, size_t out_size)
{
    out[0] = '\0';
    if (!expr)
        return;

    const char *p = expr;
    while (*p)
    {
        if (isupper((unsigned char)*p))
        {
            // Inicio de una variable potencial
            const char *start = p;
            while (isalnum((unsigned char)*p) || *p == '_')
            {
                p++;
            }
            size_t len = p - start;
            char var_name[64];
            if (len >= sizeof(var_name))
                len = sizeof(var_name) - 1;
            strncpy(var_name, start, len);
            var_name[len] = '\0';
            // No renombrar palabras reservadas
            if (strcmp(var_name, "MOD") == 0 ||
                strcmp(var_name, "ES") == 0 ||
                strcmp(var_name, "SI") == 0 ||
                strcmp(var_name, "IGUAL") == 0 ||
                strcmp(var_name, "Y") == 0 ||
                strcmp(var_name, "O") == 0 ||
                strcmp(var_name, "NO") == 0)
            {
                size_t current_len = strlen(out);
                if (current_len + strlen(var_name) < out_size)
                    strcat(out, var_name);

                continue;
            }

            // Crear nombre renombrado (ej: "E" -> "E_1")
            char renamed[128];
            snprintf(renamed, sizeof(renamed), "%s_%d", var_name, sufijo);

            size_t current_len = strlen(out);
            if (current_len + strlen(renamed) < out_size)
            {
                strcat(out, renamed);
            }
        }
        else
        {
            // Carácter normal (espacio, operador, número), copiar tal cual
            char temp[2] = {*p, '\0'};
            size_t current_len = strlen(out);
            if (current_len + 1 < out_size)
            {
                strcat(out, temp);
            }
            p++;
        }
    }
}

// ============================================================
// BUILT-INS DEL MOTOR LÓGICO (MIEMBRO, LONGITUD, etc.)
// ============================================================
// Devuelve:
//   >= 0 : número de soluciones encontradas (es built-in)
//   -1   : no es un built-in reconocido (debe seguir buscando en BC)
static int resolver_builtin(BaseConocimiento *bc, const char *predicado,
                            char args[][64], int num_args,
                            Bindings *bindings_actual,
                            Bindings *resultados, int max_resultados,
                            void *contexto, bool *corte_activo)
{
    (void)bc;       // No se usa en built-ins
    (void)contexto; // No se usa en built-ins básicos

    // ========================================
    // PREDICADO = (UNIFICACIÓN DIRECTA)
    // Sintaxis: X = [1,2,3] o f(X) = g(Y)
    // ========================================
    if (strcmp(predicado, "=") == 0 && num_args == 2)
    {
        Bindings temp = *bindings_actual;
        if (unificar_arg(args[0], args[1], &temp))
        {
            resultados[0] = temp;
            return 1;
        }
        return 0;
    }

    // ========================================
    // MIEMBRO(X, Lista)
    // ========================================
    if (strcasecmp(predicado, "miembro") == 0 && num_args == 2)
    {
        // Resolver la lista (puede ser una variable ligada)
        const char *lista_str = resolver_var(bindings_actual, args[1]);

        // Si la lista sigue siendo una variable libre, no podemos iterar
        if (es_variable(lista_str))
            return 0; // Falla: lista no instanciada

        // Debe ser una lista
        if (!es_lista(lista_str))
            return 0; // Falla: no es una lista

        // Lista vacía → falla
        if (es_lista_vacia(lista_str))
            return 0;

        // Dividir la lista en elementos
        char elems[64][64];
        int n = dividir_lista_en_elementos(lista_str, elems, 64);
        if (n <= 0)
            return 0;

        int encontrados = 0;
        const char *x_str = resolver_var(bindings_actual, args[0]);
        bool x_es_variable = es_variable(x_str);

        for (int i = 0; i < n && encontrados < max_resultados; i++)
        {
            // Si hay corte activo, detener la generación de soluciones
            if (corte_activo && *corte_activo)
                break;

            Bindings temp = *bindings_actual;

            // Intentar unificar X con el elemento i-ésimo
            if (unificar_arg(args[0], elems[i], &temp))
            {
                // Si X era variable, generamos una solución
                // Si X era constante, solo generamos solución si coincide
                if (x_es_variable || strcmp(x_str, elems[i]) == 0)
                {
                    resultados[encontrados] = temp;
                    encontrados++;

                    // Si X era constante y ya encontramos la coincidencia,
                    // no tiene sentido seguir buscando (optimización)
                    if (!x_es_variable)
                        break;
                }
            }
        }
        return encontrados;
    }

    // ========================================
    // LONGITUD(Lista, N)
    // ========================================
    if (strcasecmp(predicado, "longitud") == 0 && num_args == 2)
    {
        const char *lista_str = resolver_var(bindings_actual, args[0]);

        // La lista debe estar instanciada
        if (es_variable(lista_str))
            return 0;

        if (!es_lista(lista_str))
            return 0;

        // Contar elementos
        int longitud = 0;
        if (es_lista_vacia(lista_str))
        {
            longitud = 0;
        }
        else if (es_patron_cabeza_cola(lista_str))
        {
            // Para patrones [H|T] no podemos contar sin más info
            return 0;
        }
        else
        {
            char elems[64][64];
            longitud = dividir_lista_en_elementos(lista_str, elems, 64);
            if (longitud < 0)
                return 0;
        }

        // Convertir la longitud a string
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%d", longitud);

        // Intentar unificar N con la longitud
        Bindings temp = *bindings_actual;
        if (unificar_arg(args[1], buffer, &temp))
        {
            resultados[0] = temp;
            return 1;
        }
        return 0;
    }

    // ========================================
    // ÚLTIMO(Lista, X)  — BONUS: último elemento
    // ========================================
    if (strcasecmp(predicado, "ultimo") == 0 && num_args == 2)
    {
        const char *lista_str = resolver_var(bindings_actual, args[0]);
        if (es_variable(lista_str) || !es_lista(lista_str))
            return 0;
        if (es_lista_vacia(lista_str) || es_patron_cabeza_cola(lista_str))
            return 0;

        char elems[64][64];
        int n = dividir_lista_en_elementos(lista_str, elems, 64);
        if (n <= 0)
            return 0;

        Bindings temp = *bindings_actual;
        if (unificar_arg(args[1], elems[n - 1], &temp))
        {
            resultados[0] = temp;
            return 1;
        }
        return 0;
    }

    // ========================================
    // PRIMER(Lista, X)  — BONUS: primer elemento
    // ========================================
    if (strcasecmp(predicado, "primer") == 0 && num_args == 2)
    {
        const char *lista_str = resolver_var(bindings_actual, args[0]);
        if (es_variable(lista_str) || !es_lista(lista_str))
            return 0;
        if (es_lista_vacia(lista_str) || es_patron_cabeza_cola(lista_str))
            return 0;

        char elems[64][64];
        int n = dividir_lista_en_elementos(lista_str, elems, 64);
        if (n <= 0)
            return 0;

        Bindings temp = *bindings_actual;
        if (unificar_arg(args[1], elems[0], &temp))
        {
            resultados[0] = temp;
            return 1;
        }
        return 0;
    }

    // ========================================
    // JUNTAR(L1, L2, L3) - Concatenar listas
    // Equivalente a APPEND en Prolog
    // ========================================
    if (strcasecmp(predicado, "juntar") == 0 && num_args == 3)
    {
        const char *l1_str = resolver_var(bindings_actual, args[0]);
        const char *l2_str = resolver_var(bindings_actual, args[1]);
        const char *l3_str = resolver_var(bindings_actual, args[2]);

        bool l1_inst = !es_variable(l1_str) && es_lista(l1_str);
        bool l2_inst = !es_variable(l2_str) && es_lista(l2_str);
        bool l3_inst = !es_variable(l3_str) && es_lista(l3_str);

        // Caso 1: L1 y L2 instanciadas → concatenar y unificar con L3
        if (l1_inst && l2_inst)
        {
            char elems1[64][64], elems2[64][64];
            int n1 = dividir_lista_en_elementos(l1_str, elems1, 64);
            int n2 = dividir_lista_en_elementos(l2_str, elems2, 64);

            if (n1 < 0 || n2 < 0)
                return 0;

            // Construir lista concatenada
            char elems_result[128][64];
            int n_result = 0;
            for (int i = 0; i < n1 && n_result < 128; i++)
            {
                strncpy(elems_result[n_result], elems1[i], 63);
                elems_result[n_result][63] = '\0';
                n_result++;
            }
            for (int i = 0; i < n2 && n_result < 128; i++)
            {
                strncpy(elems_result[n_result], elems2[i], 63);
                elems_result[n_result][63] = '\0';
                n_result++;
            }

            char lista_result[512];
            construir_lista(lista_result, sizeof(lista_result), elems_result, n_result);

            Bindings temp = *bindings_actual;
            if (unificar_arg(args[2], lista_result, &temp))
            {
                resultados[0] = temp;
                return 1;
            }
            return 0;
        }

        // Caso 2: L3 instanciada → dividir en todas las formas posibles
        if (l3_inst)
        {
            char elems3[64][64];
            int n3 = dividir_lista_en_elementos(l3_str, elems3, 64);
            if (n3 < 0)
                return 0;

            int encontrados = 0;
            // Generar todas las divisiones posibles: L1 = primeros i elementos, L2 = resto
            for (int i = 0; i <= n3 && encontrados < max_resultados; i++)
            {
                char elems_l1[64][64], elems_l2[64][64];
                int n_l1 = 0, n_l2 = 0;

                for (int j = 0; j < i; j++)
                {
                    strncpy(elems_l1[n_l1], elems3[j], 63);
                    elems_l1[n_l1][63] = '\0';
                    n_l1++;
                }
                for (int j = i; j < n3; j++)
                {
                    strncpy(elems_l2[n_l2], elems3[j], 63);
                    elems_l2[n_l2][63] = '\0';
                    n_l2++;
                }

                char lista_l1[512], lista_l2[512];
                construir_lista(lista_l1, sizeof(lista_l1), elems_l1, n_l1);
                construir_lista(lista_l2, sizeof(lista_l2), elems_l2, n_l2);

                Bindings temp = *bindings_actual;
                if (unificar_arg(args[0], lista_l1, &temp) &&
                    unificar_arg(args[1], lista_l2, &temp))
                {
                    resultados[encontrados] = temp;
                    encontrados++;
                }
            }
            return encontrados;
        }

        return 0; // No se puede resolver sin al menos una lista instanciada
    }

    // ========================================
    // REVERSA(L1, L2) - Invertir lista
    // Equivalente a REVERSE en Prolog
    // ========================================
    if (strcasecmp(predicado, "reversa") == 0 && num_args == 2)
    {
        const char *l1_str = resolver_var(bindings_actual, args[0]);
        const char *l2_str = resolver_var(bindings_actual, args[1]);

        bool l1_inst = !es_variable(l1_str) && es_lista(l1_str);
        bool l2_inst = !es_variable(l2_str) && es_lista(l2_str);

        // Necesita al menos una lista instanciada
        if (!l1_inst && !l2_inst)
            return 0;

        const char *lista_orig = l1_inst ? l1_str : l2_str;
        const char *arg_destino = l1_inst ? args[1] : args[0];

        char elems[64][64];
        int n = dividir_lista_en_elementos(lista_orig, elems, 64);
        if (n < 0)
            return 0;

        // Invertir
        char elems_inv[64][64];
        for (int i = 0; i < n; i++)
        {
            strncpy(elems_inv[i], elems[n - 1 - i], 63);
            elems_inv[i][63] = '\0';
        }

        char lista_inv[512];
        construir_lista(lista_inv, sizeof(lista_inv), elems_inv, n);

        Bindings temp = *bindings_actual;
        if (unificar_arg(arg_destino, lista_inv, &temp))
        {
            resultados[0] = temp;
            return 1;
        }
        return 0;
    }
    
    // No es un built-in reconocido
    return -1;
}

static int resolver(BaseConocimiento *bc, const char *predicado,
                    char args[][64], int num_args,
                    Bindings *bindings_actual,
                    Bindings *resultados, int max_resultados, void *contexto,
                    bool *corte_activo)
{
    int encontrados = 0;
    
    // ============================================================
    // INTERCEPTAR BUILT-INS ANTES DE BUSCAR EN LA BC
    // ============================================================
    int builtin_result = resolver_builtin(bc, predicado, args, num_args,
                                          bindings_actual, resultados, max_resultados,
                                          contexto, corte_activo);
    if (builtin_result >= 0)
    {
        // Es un built-in: devolver directamente sus soluciones
        return builtin_result;
    }

    // ============================================================
    // PREPARAR CHOICE POINTS (HECHOS Y REGLAS ORDENADOS CRONOLÓGICAMENTE)
    // ============================================================
    struct ChoicePoint
    {
        int es_regla; // 0 = hecho, 1 = regla
        int indice;
        int orden;
    };

    struct ChoicePoint choices[MAX_HECHOS + MAX_REGLAS];
    int num_choices = 0;

    // Recolectar hechos que coinciden con el predicado
    int idx_h[MAX_HECHOS], num_h = 0;
    for (int i = 0; i < bc->num_hechos; i++)
    {
        if (strcmp(bc->hechos[i].predicado, predicado) == 0 && bc->hechos[i].num_args == num_args)
        {
            idx_h[num_h++] = i;
        }
    }
    // Recolectar reglas que coinciden con el predicado
    int idx_r[MAX_REGLAS], num_r = 0;
    for (int i = 0; i < bc->num_reglas; i++)
    {
        if (strcmp(bc->reglas[i].nombre, predicado) == 0 && bc->reglas[i].num_cabeza == num_args)
        {
            idx_r[num_r++] = i;
        }
    }

    // Fusionar ambos arrays ordenados por su campo 'orden' (Merge sort de dos arrays)
    int ih = 0, ir = 0;
    while (ih < num_h && ir < num_r)
    {
        if (bc->hechos[idx_h[ih]].orden < bc->reglas[idx_r[ir]].orden)
        {
            choices[num_choices].es_regla = 0;
            choices[num_choices].indice = idx_h[ih++];
            choices[num_choices].orden = bc->hechos[choices[num_choices].indice].orden;
        }
        else
        {
            choices[num_choices].es_regla = 1;
            choices[num_choices].indice = idx_r[ir++];
            choices[num_choices].orden = bc->reglas[choices[num_choices].indice].orden;
        }
        num_choices++;
    }
    while (ih < num_h)
    {
        choices[num_choices].es_regla = 0;
        choices[num_choices].indice = idx_h[ih++];
        choices[num_choices].orden = bc->hechos[choices[num_choices].indice].orden;
        num_choices++;
    }
    while (ir < num_r)
    {
        choices[num_choices].es_regla = 1;
        choices[num_choices].indice = idx_r[ir++];
        choices[num_choices].orden = bc->reglas[choices[num_choices].indice].orden;
        num_choices++;
    }

    // ============================================================
    // EVALUAR CHOICE POINTS EN ORDEN ESTRICTO
    // ============================================================
    for (int c = 0; c < num_choices && encontrados < max_resultados; c++)
    {
        // === SI HAY CORTE ACTIVO, NO PROBAR MÁS ALTERNATIVAS (EFECTO GLOBAL) ===
        if (corte_activo && *corte_activo)
            break;

        if (choices[c].es_regla == 0)
        {
            // --- EVALUAR HECHO ---
            Hecho *h = &bc->hechos[choices[c].indice];

            // RENOMBRAR VARIABLES EN HECHOS (igual que en reglas)
            // Esto evita colisión de nombres entre variables del hecho y de la consulta
            contador_renombre_global++;
            int sufijo = contador_renombre_global;
            char args_renombrados[MAX_ARGS][64];
            for (int j = 0; j < num_args; j++)
            {
                renombrar_variables_en_expresion(h->args[j], sufijo, args_renombrados[j], 64);
            }

            Bindings temp = *bindings_actual;
            bool coincide = true;
            for (int j = 0; j < num_args; j++)
            {
                if (!unificar_arg(args[j], args_renombrados[j], &temp))
                {
                    coincide = false;
                    break;
                }
            }
            if (coincide)
            {
                resultados[encontrados++] = temp;
            }
        }
        else
        {
            // --- EVALUAR REGLA ---
            Regla *r = &bc->reglas[choices[c].indice];
            contador_renombre_global++;
            int sufijo = contador_renombre_global;
            char cabeza_renombrada[MAX_ARGS][64];
            Condicion cuerpo_renombrado[MAX_CONDICIONES];

            for (int j = 0; j < r->num_cabeza; j++)
            {
                renombrar_variables_en_expresion(r->cabeza_args[j], sufijo, cabeza_renombrada[j], 64);
            }
            for (int j = 0; j < r->num_cuerpo; j++)
            {
                cuerpo_renombrado[j] = r->cuerpo[j];
                for (int k = 0; k < r->cuerpo[j].num_args; k++)
                {
                    renombrar_variables_en_expresion(r->cuerpo[j].args[k], sufijo, cuerpo_renombrado[j].args[k], 64);                    
                }
            }

            Bindings temp = *bindings_actual;
            bool unifica = true;
            for (int j = 0; j < num_args; j++)
            {
                if (!unificar_arg(args[j], cabeza_renombrada[j], &temp))
                {
                    unifica = false;
                    break;
                }
            }
            if (!unifica)
                continue;

            Bindings cuerpo_bindings[MAX_SOLUCIONES];
            int num_cuerpo_sols = 1;
            cuerpo_bindings[0] = temp;

            for (int i_cond = 0; i_cond < r->num_cuerpo && num_cuerpo_sols > 0; i_cond++)
            {
                Condicion *cond = &cuerpo_renombrado[i_cond];
                Bindings nuevas_sols[MAX_SOLUCIONES];
                int num_nuevas = 0;

                for (int s = 0; s < num_cuerpo_sols && num_nuevas < MAX_SOLUCIONES; s++)
                {

                    // === MANEJAR EL OPERADOR DE CORTE ===
                    if (cond->es_corte)
                    {
                        nuevas_sols[num_nuevas++] = cuerpo_bindings[s];
                        if (corte_activo)
                            *corte_activo = true; // Efecto Global

                        // ¡EFECTO LOCAL DEL CORTE!
                        // Congelamos el backtracking. Solo mantenemos esta solución y descartamos el resto.
                        break;
                    }
                    else if (cond->es_aritmetica)
                    {
                        Bindings temp_bindings = cuerpo_bindings[s];
                        if (evaluar_condicion_aritmetica(cond, &temp_bindings, contexto))
                        {
                            nuevas_sols[num_nuevas++] = temp_bindings;
                        }
                    }
                    else if (cond->negacion)
                    {
                        Bindings sols_temp[MAX_SOLUCIONES];
                        bool corte_neg = false;
                        int num_temp = resolver(bc, cond->predicado, cond->args, cond->num_args,
                                                &cuerpo_bindings[s], sols_temp, MAX_SOLUCIONES, contexto, &corte_neg);
                        if (num_temp == 0)
                        {
                            nuevas_sols[num_nuevas++] = cuerpo_bindings[s];
                        }
                    }
                    else
                    {
                        // Llamada recursiva normal
                        // Nota: El corte de una regla anidada es local a esa regla, no se propaga hacia arriba.
                        bool corte_rec = false;
                        int sols = resolver(bc, cond->predicado, cond->args, cond->num_args,
                                            &cuerpo_bindings[s], nuevas_sols + num_nuevas,
                                            MAX_SOLUCIONES - num_nuevas, contexto, &corte_rec);
                        num_nuevas += sols;
                    }
                }
                num_cuerpo_sols = num_nuevas;
                for (int s = 0; s < num_cuerpo_sols; s++)
                {
                    cuerpo_bindings[s] = nuevas_sols[s];
                }
            }

            for (int s = 0; s < num_cuerpo_sols && encontrados < max_resultados; s++)
            {
                resultados[encontrados++] = cuerpo_bindings[s];
            }
        }
    }
    return encontrados;
}

// ============================================================
// CONSULTA PÚBLICA (NUEVA VERSIÓN QUE DEVUELVE BINDINGS)
// ============================================================
ResultadoConsulta bc_consultar(BaseConocimiento *bc, const char *str, void *contexto)
{
    ResultadoConsulta resultado;
    resultado.num_soluciones = 0;
    resultado.exito = false;

    if (!bc || !str)
    {
        return resultado;
    }

    // ============================================================
    // DIVIDIR LA CONSULTA EN CONDICIONES (separadas por comas al nivel 0)
    // ============================================================
    char condiciones[MAX_CONDICIONES][256];
    bool negaciones[MAX_CONDICIONES];
    int num_condiciones = 0;
    int nivel_paren = 0;
    int nivel_corchete = 0;
    const char *inicio = str;
    const char *p = str;
    while (*inicio == ' ')
        inicio++;
    while (*p && num_condiciones < MAX_CONDICIONES)
    {
        if (*p == '(')
            nivel_paren++;
        else if (*p == ')')
            nivel_paren--;
        else if (*p == '[')
            nivel_corchete++;
        else if (*p == ']')
            nivel_corchete--;
        else if (*p == ',' && nivel_paren == 0 && nivel_corchete == 0)
        {
            int len = p - inicio;
            if (len > 0 && len < 256)
            {
                strncpy(condiciones[num_condiciones], inicio, len);
                condiciones[num_condiciones][len] = '\0';

                char *q = condiciones[num_condiciones];
                while (*q == ' ')
                    q++;
                char *fin = q + strlen(q) - 1;
                while (fin > q && *fin == ' ')
                    *fin-- = '\0';

                negaciones[num_condiciones] = false;
                if (strncmp(q, "NO ", 3) == 0)
                {
                    negaciones[num_condiciones] = true;
                    q += 3;
                    while (*q == ' ')
                        q++;
                }

                if (strlen(q) > 0)
                {
                    if (q != condiciones[num_condiciones])
                        memmove(condiciones[num_condiciones], q, strlen(q) + 1);
                    num_condiciones++;
                }
            }
            inicio = p + 1;
        }
        p++;
    }

    if (inicio < p && num_condiciones < MAX_CONDICIONES)
    {
        strncpy(condiciones[num_condiciones], inicio, 255);
        condiciones[num_condiciones][255] = '\0';

        char *q = condiciones[num_condiciones];
        while (*q == ' ')
            q++;
        char *fin = q + strlen(q) - 1;
        while (fin > q && *fin == ' ')
            *fin-- = '\0';

        negaciones[num_condiciones] = false;
        if (strncmp(q, "NO ", 3) == 0)
        {
            negaciones[num_condiciones] = true;
            q += 3;
            while (*q == ' ')
                q++;
        }

        if (strlen(q) > 0)
        {
            if (q != condiciones[num_condiciones])
                memmove(condiciones[num_condiciones], q, strlen(q) + 1);
            num_condiciones++;
        }
    }

    if (num_condiciones == 0)
    {
        return resultado;
    }

    // ============================================================
    // RESOLVER CADA CONDICIÓN EN SECUENCIA
    // ============================================================
    Bindings bindings_actuales[MAX_SOLUCIONES];
    int num_sols_actuales = 1;
    bindings_init(&bindings_actuales[0]);

    for (int c = 0; c < num_condiciones && num_sols_actuales > 0; c++)
    {
        char predicado[64];
        char args[MAX_ARGS][64];
        int num_args = 0;

        // Intentar parsear como condición aritmética
        Condicion cond_temp;
        if (parsear_condicion_aritmetica(condiciones[c], &cond_temp))
        {
            // Es una condición aritmética
            Bindings nuevas_sols[MAX_SOLUCIONES];
            int num_nuevas = 0;
            for (int s = 0; s < num_sols_actuales && num_nuevas < MAX_SOLUCIONES; s++)
            {
                Bindings temp_bindings = bindings_actuales[s];
                if (evaluar_condicion_aritmetica(&cond_temp, &temp_bindings, contexto))
                {
                    nuevas_sols[num_nuevas] = temp_bindings;
                    num_nuevas++;
                }
            }
            num_sols_actuales = num_nuevas;
            for (int s = 0; s < num_sols_actuales; s++)
            {
                bindings_actuales[s] = nuevas_sols[s];
            }
        }
        // Detectar predicado especial "=" (unificación directa)
        // Patrón: "X = Y" o "f(X) = g(Y)"
        else if (strstr(condiciones[c], " = ") != NULL)
        {
            char *igual_pos = strstr(condiciones[c], " = ");
            int len_izq = igual_pos - condiciones[c];

            char lado_izq[256], lado_der[256];
            strncpy(lado_izq, condiciones[c], len_izq);
            lado_izq[len_izq] = '\0';
            strcpy(lado_der, igual_pos + 3);

            // Trim espacios
            char *p = lado_izq + strlen(lado_izq) - 1;
            while (p > lado_izq && *p == ' ')
                *p-- = '\0';
            char *q = lado_izq;
            while (*q == ' ')
                q++;
            if (q != lado_izq)
                memmove(lado_izq, q, strlen(q) + 1);

            p = lado_der + strlen(lado_der) - 1;
            while (p > lado_der && *p == ' ')
                *p-- = '\0';
            q = lado_der;
            while (*q == ' ')
                q++;
            if (q != lado_der)
                memmove(lado_der, q, strlen(q) + 1);

            // Crear args para el predicado "="
            strncpy(args[0], lado_izq, 63);
            args[0][63] = '\0';
            strncpy(args[1], lado_der, 63);
            args[1][63] = '\0';
            strcpy(predicado, "=");
            num_args = 2;

            // Procesar como consulta normal (caerá en el else de abajo)
            Bindings nuevas_sols[MAX_SOLUCIONES];
            int num_nuevas = 0;
            bool corte_activo_normal = false;
            for (int s = 0; s < num_sols_actuales && num_nuevas < MAX_SOLUCIONES; s++)
            {
                int sols = resolver(bc, predicado, args, num_args, &bindings_actuales[s],
                                    nuevas_sols + num_nuevas, MAX_SOLUCIONES - num_nuevas,
                                    contexto, &corte_activo_normal);
                num_nuevas += sols;
            }
            num_sols_actuales = num_nuevas;
            for (int s = 0; s < num_sols_actuales; s++)
            {
                bindings_actuales[s] = nuevas_sols[s];
            }
            continue; // Saltar al siguiente condición
        }

        else if (!parsear_predicado(condiciones[c], predicado, args, &num_args))
        {
            num_sols_actuales = 0;
            break;
        }
        else if (negaciones[c])
        {
            // NEGACIÓN POR FALLO
            Bindings nuevas_sols[MAX_SOLUCIONES];
            //bool corte_activo_neg = false;
            int num_nuevas = 0;
            for (int s = 0; s < num_sols_actuales && num_nuevas < MAX_SOLUCIONES; s++)
            {
                Bindings sols_temp[MAX_SOLUCIONES];
                bool corte_activo_neg = false;
                int num_temp = resolver(bc, predicado, args, num_args, &bindings_actuales[s],
                                        sols_temp, MAX_SOLUCIONES, contexto, &corte_activo_neg);
                if (num_temp == 0)
                {
                    nuevas_sols[num_nuevas] = bindings_actuales[s];
                    num_nuevas++;
                }
            }
            num_sols_actuales = num_nuevas;
            for (int s = 0; s < num_sols_actuales; s++)
            {
                bindings_actuales[s] = nuevas_sols[s];
            }
        }
        else
        {
            // CASO NORMAL
            Bindings nuevas_sols[MAX_SOLUCIONES];
            int num_nuevas = 0;
            bool corte_activo_normal = false;
            for (int s = 0; s < num_sols_actuales && num_nuevas < MAX_SOLUCIONES; s++)
            {
                int sols = resolver(bc, predicado, args, num_args, &bindings_actuales[s],
                                    nuevas_sols + num_nuevas, MAX_SOLUCIONES - num_nuevas,
                                    contexto, &corte_activo_normal);

                num_nuevas += sols;
            }
            num_sols_actuales = num_nuevas;
            for (int s = 0; s < num_sols_actuales; s++)
            {
                bindings_actuales[s] = nuevas_sols[s];
            }
        }
    }

    if (num_sols_actuales == 0)
    {
        return resultado;
    }

    // ============================================================
    // RECOLECTAR VARIABLES ORIGINALES (EXCLUYENDO CONDICIONES NEGADAS)
    // ============================================================
    char vars_originales[MAX_ARGS][64];
    int num_vars = 0;
    for (int c = 0; c < num_condiciones; c++)
    {
        if (negaciones[c])
            continue;
        char predicado[64];
        char args[MAX_ARGS][64];
        int num_args = 0;
        if (parsear_predicado(condiciones[c], predicado, args, &num_args))
        {
            for (int i = 0; i < num_args; i++)
            {
                recolectar_variables_en_arg(args[i], vars_originales, &num_vars);
            }
        }
    }

    // ============================================================
    // COPIAR SOLUCIONES AL RESULTADO
    // ============================================================
    resultado.num_soluciones = num_sols_actuales;
    resultado.exito = (num_sols_actuales > 0);
    for (int s = 0; s < num_sols_actuales && s < MAX_SOLUCIONES; s++)
    {
        resultado.soluciones[s] = bindings_actuales[s];
    }
    return resultado;
}

// ============================================================
// FUNCIÓN DE IMPRESIÓN (COMPATIBILIDAD)
// ============================================================
void bc_imprimir_soluciones(BaseConocimiento *bc, const char *str)
{
    ResultadoConsulta resultado = bc_consultar(bc, str, NULL);

    if (!resultado.exito)
    {
        printf("FALSO\n");
        return;
    }

    // Recolectar variables originales
    char vars_originales[MAX_ARGS][64];
    int num_vars = 0;

    // Parsear la consulta para extraer variables
    char condiciones[MAX_CONDICIONES][256];
    bool negaciones[MAX_CONDICIONES];
    int num_condiciones = 0;
    int nivel_paren = 0;
    const char *inicio = str;
    const char *p = str;

    while (*inicio == ' ')
        inicio++;

    while (*p && num_condiciones < MAX_CONDICIONES)
    {
        if (*p == '(')
            nivel_paren++;
        else if (*p == ')')
            nivel_paren--;
        else if (*p == ',' && nivel_paren == 0)
        {
            int len = p - inicio;
            if (len > 0 && len < 256)
            {
                strncpy(condiciones[num_condiciones], inicio, len);
                condiciones[num_condiciones][len] = '\0';

                char *q = condiciones[num_condiciones];
                while (*q == ' ')
                    q++;
                char *fin = q + strlen(q) - 1;
                while (fin > q && *fin == ' ')
                    *fin-- = '\0';

                negaciones[num_condiciones] = false;
                if (strncmp(q, "NO ", 3) == 0)
                {
                    negaciones[num_condiciones] = true;
                    q += 3;
                    while (*q == ' ')
                        q++;
                }

                if (strlen(q) > 0)
                {
                    if (q != condiciones[num_condiciones])
                        memmove(condiciones[num_condiciones], q, strlen(q) + 1);
                    num_condiciones++;
                }
            }
            inicio = p + 1;
        }
        p++;
    }

    if (inicio < p && num_condiciones < MAX_CONDICIONES)
    {
        strncpy(condiciones[num_condiciones], inicio, 255);
        condiciones[num_condiciones][255] = '\0';

        char *q = condiciones[num_condiciones];
        while (*q == ' ')
            q++;
        char *fin = q + strlen(q) - 1;
        while (fin > q && *fin == ' ')
            *fin-- = '\0';

        negaciones[num_condiciones] = false;
        if (strncmp(q, "NO ", 3) == 0)
        {
            negaciones[num_condiciones] = true;
            q += 3;
            while (*q == ' ')
                q++;
        }

        if (strlen(q) > 0)
        {
            if (q != condiciones[num_condiciones])
                memmove(condiciones[num_condiciones], q, strlen(q) + 1);
            num_condiciones++;
        }
    }

    for (int c = 0; c < num_condiciones; c++)
    {
        if (negaciones[c])
            continue;
        char predicado[64];
        char args[MAX_ARGS][64];
        int num_args = 0;
        if (parsear_predicado(condiciones[c], predicado, args, &num_args))
        {
            for (int i = 0; i < num_args; i++)
            {
                // Usar la función recursiva que busca variables DENTRO de listas
                recolectar_variables_en_arg(args[i], vars_originales, &num_vars);
            }
        }
    }

    if (num_vars == 0)
    {
        printf("VERDADERO\n");
        return;
    }

    // Imprimir soluciones
    for (int s = 0; s < resultado.num_soluciones; s++)
    {
        bool imprimio_algo = false;
        for (int i = 0; i < num_vars; i++)
        {
            const char *valor = resolver_var(&resultado.soluciones[s], vars_originales[i]);

            // OMITIR si la variable se resuelve a sí misma (ej: Y = Y)
            if (strcmp(vars_originales[i], valor) == 0)
                continue;

            if (!imprimio_algo)
                imprimio_algo = true;
            else
                printf(", ");

            printf("%s = %s", vars_originales[i], valor);
        }

        // Si todas las variables se resolvieron a sí mismas, imprimir VERDADERO
        if (!imprimio_algo)
        {
            printf("VERDADERO");
        }
        printf("\n");
    }
}

// ============================================================
// WRAPPERS PÚBLICOS (para uso desde evaluator.c)
// ============================================================
bool parsear_predicado_publico(const char *str, char *predicado, char args[][64], int *num_args)
{
    return parsear_predicado(str, predicado, args, num_args);
}

bool es_variable_publico(const char *s)
{
    return es_variable(s);
}

const char *resolver_var_publico(const Bindings *b, const char *nombre)
{
    return resolver_var(b, nombre);
}

void recolectar_variables_en_arg_publico(const char *arg, char vars[][64], int *num_vars)
{
    recolectar_variables_en_arg(arg, vars, num_vars);
}
