/*
 * Nico v2.1.0 - Intérprete Educativo de Scripting en Español
 * @file:         main.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Punto de entrada principal del intérprete. Gestiona argumentos
 *                de línea de comandos, modo interactivo (REPL) y ejecución de
 *                archivos .nico. Inicializa lexer, parser y evaluador, maneja
 *                señales (Ctrl+C) y limpieza de recursos al finalizar.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <signal.h>
#include "lexer.h"
#include "parser.h"
#include "evaluator.h"
#include <unistd.h>
#include "signal_handler.h"
#ifndef _WIN32
#include <sys/resource.h>
#include <readline/readline.h>
#include <readline/history.h>
#endif
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#endif

#define MAX_LINEA 1024
#define MAX_PROGRAMA 100000

// Forward declarations
void mostrar_ayuda(void);
void comando_rangos(void);
void evaluar_expresion(const char *expr);
void ejecutar_archivo(const char *ruta);
void repl(void);

// Variable global para el programa cargado
char *programa_cargado = NULL;
int archivo_cargado = 0;

// Buffer para almacenar funciones y subprogramas definidos en el REPL
char buffer_definiciones[MAX_PROGRAMA] = "";

// ============================================================
// FUNCIONES AUXILIARES CROSS-PLATFORM (Windows / Linux)
// ============================================================

// Función auxiliar para obtener el directorio donde está el ejecutable
void obtener_directorio_base(char *buffer, size_t tamano)
{
    buffer[0] = '\0';
#ifdef _WIN32
    GetModuleFileNameA(NULL, buffer, tamano);
    char *last_slash = strrchr(buffer, '\\');
    if (last_slash)
        *last_slash = '\0';
#else
    ssize_t len = readlink("/proc/self/exe", buffer, tamano - 1);
    if (len != -1)
    {
        buffer[len] = '\0';
        char *last_slash = strrchr(buffer, '/');
        if (last_slash)
            *last_slash = '\0';
    }
#endif
}

// Función para buscar un archivo .nico dinámicamente en la carpeta de ejemplos
char *buscar_en_ejemplos(const char *nombre_archivo)
{
    static char ruta_completa[4096] = "";
    char dir_ejecutable[1024] = "";
    obtener_directorio_base(dir_ejecutable, sizeof(dir_ejecutable));

    char candidatas[8][256];
    int count = 0;

    // 1. Desde el directorio del ejecutable (Funciona en ambos OS)
    if (strlen(dir_ejecutable) > 0)
    {
        snprintf(candidatas[count++], 256, "%s/ejemplos", dir_ejecutable);
        snprintf(candidatas[count++], 256, "%s/../ejemplos", dir_ejecutable);
    }

#ifndef _WIN32
    // 2. Rutas de instalación global en Linux (SOLO se compilan en Linux)
    snprintf(candidatas[count++], 256, "/usr/local/share/nico/ejemplos");
    snprintf(candidatas[count++], 256, "/usr/share/nico/ejemplos");
#endif

    // 3. Desde el directorio de trabajo actual (CWD) (Funciona en ambos OS)
    snprintf(candidatas[count++], 256, "./ejemplos");
    snprintf(candidatas[count++], 256, "../ejemplos");
    snprintf(candidatas[count++], 256, "../../ejemplos");

    // Probar cada ruta candidata
    for (int i = 0; i < count; i++)
    {
        snprintf(ruta_completa, sizeof(ruta_completa), "%s/%s", candidatas[i], nombre_archivo);
        FILE *f = fopen(ruta_completa, "r");
        if (f)
        {
            fclose(f);
            return ruta_completa; // ¡Encontrado!
        }
    }
    return NULL;
}

// ============================================================
// FIN DE FUNCIONES AUXILIARES
// ============================================================

void mostrar_ayuda(void)
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║          COMANDOS DEL INTÉRPRETE NICO v2.1.0           ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║  USAR archivo.nico   Carga un archivo .nico            ║\n");
    printf("║  CORRER              Ejecuta el programa cargado       ║\n");
    printf("║  CERRAR              Cierra el programa actual         ║\n");
    printf("║  RANGOS              Muestra rangos de variables       ║\n");
    printf("║  SALIR               Sale del intérprete               ║\n");
    printf("║  ?                   Muestra esta ayuda                ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void comando_rangos(void)
{
    printf("\n");
    printf("   RANGOS DE VARIABLES - NICO v2.1.0\n");
    printf("\n");
    printf("   VARIABLES ENTERAS:\n");
    printf("     Rango: %lld a %lld\n", LLONG_MIN, LLONG_MAX);
    printf("\n");
    printf("   VARIABLES ENTERAS SIN SIGNO:\n");
    printf("     Rango: 0 a %llu\n", ULLONG_MAX);
    printf("\n");
    printf("   VARIABLES DECIMALES:\n");
    printf("     Rango: %e a %e\n", -DBL_MAX, DBL_MAX);
    printf("\n");
    printf("   VARIABLES DECIMALES SIN SIGNO:\n");
    printf("     Rango: 0.0 a %e (mismo tipo que DECIMAL, solo valores >= 0)\n", DBL_MAX);
    printf("\n");
    printf("   VARIABLES CARACTER:\n");
    printf("     Rango: %d a %d (ASCII extendido, signed char)\n", SCHAR_MIN, SCHAR_MAX);
    printf("\n");
    printf("   VARIABLES CARACTER SIN SIGNO:\n");
    printf("     Rango: 0 a %u (ASCII extendido, unsigned char)\n", UCHAR_MAX);
    printf("\n");
    printf("   VARIABLES TEXTO y TEXTO EXTENSO:\n");
    printf("     Largo: Dinamico (limitado por RAM disponible)\n");
    printf("\n");
    printf("   VARIABLES LOGICA:\n");
    printf("     Para valores o estados logicos: VERDADERO/FALSO\n");
    printf("\n");
    printf("   ARCHIVOS:\n");
    printf("     Modos: ESCRITURA, AGREGAR, LECTURA, LECTOESCRITURA\n");
    printf("\n");
    printf("   FUNCIONES Y SUBPROGRAMAS:\n");
    printf("     Soporte completo con parametros y retorno\n");
    printf("\n");
}

void evaluar_expresion(const char *expr)
{
    Lexer *lexer = lexer_crear(expr);
    if (!lexer)
    {
        fprintf(stderr, "Error: No se pudo crear el lexer.\n");
        return;
    }

    Parser *parser = parser_crear(lexer);
    if (!parser)
    {
        fprintf(stderr, "Error: No se pudo crear el parser.\n");
        lexer_destruir(lexer);
        return;
    }

    Contexto *ctx = contexto_crear();
    if (!ctx)
    {
        fprintf(stderr, "Error: No se pudo crear el contexto.\n");
        parser_destruir(parser);
        lexer_destruir(lexer);
        return;
    }

    NodoAST *ast = parsear_expresion(parser);

    if (parser_tiene_error(parser))
    {
        fprintf(stderr, "Error de parsing: %s\n", parser_obtener_error(parser));
    }
    else if (ast)
    {
        Valor resultado = evaluar_nodo(ast, ctx);
        if (!ctx->hay_error)
        {
            valor_imprimir(resultado);
            printf("\n");
        }
        else
        {
            fprintf(stderr, "Error de ejecución.\n");
        }
        valor_destruir(&resultado);
        liberar_nodo(ast);
    }

    contexto_destruir(ctx);
    parser_destruir(parser);
    lexer_destruir(lexer);
}

void ejecutar_archivo(const char *ruta)
{
#ifndef _WIN32
    obtener_terminal_original();
#endif

    FILE *archivo = fopen(ruta, "r");
    if (!archivo)
    {
        fprintf(stderr, "Error: No se pudo abrir el archivo '%s'\n", ruta);
        return;
    }

    fseek(archivo, 0, SEEK_END);
    long tamano = ftell(archivo);
    fseek(archivo, 0, SEEK_SET);

    char *codigo = malloc(tamano + 1);
    if (!codigo)
    {
        fprintf(stderr, "Error: No se pudo asignar memoria.\n");
        fclose(archivo);
        return;
    }

    fread(codigo, 1, tamano, archivo);
    codigo[tamano] = '\0';
    fclose(archivo);

    Lexer *lexer = lexer_crear(codigo);
    Parser *parser = parser_crear(lexer);
    Contexto *ctx = contexto_crear();

    if (!lexer || !parser || !ctx)
    {
        fprintf(stderr, "Error: No se pudo inicializar el intérprete.\n");
        if (ctx)
            contexto_destruir(ctx);
        if (parser)
            parser_destruir(parser);
        if (lexer)
            lexer_destruir(lexer);
        free(codigo);
        return;
    }

    NodoAST *ast = parser_parsear(parser);

    if (parser_tiene_error(parser))
    {
        fprintf(stderr, "Error de parsing: %s\n", parser_obtener_error(parser));
    }
    else if (ast)
    {

        // Obtener nombre del programa desde el AST
        const char *nombre_prog = (ast->tipo == AST_PROGRAMA && ast->datos.programa.nombre)
                                      ? ast->datos.programa.nombre
                                      : ruta;

        printf("\n> Corriendo programa '%s'\n", nombre_prog);
        printf("\n");

        teclado_iniciar_modo_raw();
        evaluar_nodo(ast, ctx);

        // Restaurar terminal ANTES de imprimir mensajes finales
        teclado_restaurar_modo();
        printf("\n");

        // Verificar si hubo error durante la ejecución
        if (ctx->hay_error)
        {
            fprintf(stderr, "\n❌ Error de ejecución: %s\n", ctx->mensaje_error[0] ? ctx->mensaje_error : "Error desconocido");
        }
        else
        {
            fprintf(stderr, "\n> Programa '%s' finalizado\n", nombre_prog);
        }
        liberar_nodo(ast);
    }

    contexto_destruir(ctx);
    parser_destruir(parser);
    lexer_destruir(lexer);
    free(codigo);
}

void repl(void)
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║     INTÉRPRETE NICO v2.1.0 - Modo Interactivo          ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║  USAR archivo.nico   Carga un archivo .nico            ║\n");
    printf("║  CORRER              Ejecuta el programa cargado       ║\n");
    printf("║  CERRAR              Cierra el programa actual         ║\n");
    printf("║  RANGOS              Muestra rangos de variables       ║\n");
    printf("║  SALIR               Sale del intérprete               ║\n");
    printf("║  ?                   Muestra esta ayuda                ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║  BLOQUE              Modo multilínea (bucles/condic.)  ║\n");
    printf("║  LIMPIAR             Borra funciones/subprogramas      ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");

    Contexto *ctx = contexto_crear();
    if (!ctx)
    {
        fprintf(stderr, "Error fatal: No se pudo crear el contexto.\n");
        return;
    }

    int modo_bloque = 0;
    int modo_definicion = 0;
    char buffer_bloque[MAX_PROGRAMA] = "";
    char buffer_def_temp[MAX_PROGRAMA] = "";

    char comando[MAX_LINEA];

    while (1)
    {
#ifndef _WIN32
        char *linea = readline("nico>>> ");
        if (!linea)
        {
            printf("\n");
            break;
        }
        strncpy(comando, linea, MAX_LINEA - 1);
        comando[MAX_LINEA - 1] = '\0';
        free(linea);

        if (strlen(comando) > 0)
        {
            add_history(comando);
        }
#else
        printf("nico>>> ");
        fflush(stdout);
        if (!fgets(comando, MAX_LINEA, stdin))
        {
            printf("\n");
            break;
        }
#endif

        comando[strcspn(comando, "\n\r")] = '\0';
        char *ptr = comando;
        while (*ptr == ' ' || *ptr == '\t')
            ptr++;

        if (strlen(ptr) == 0)
            continue;

        // --- COMANDOS ESPECIALES DEL REPL ---
        if (strcmp(ptr, "?") == 0 || strcmp(ptr, "ayuda") == 0)
        {
            mostrar_ayuda();
            continue;
        }
        if (strcmp(ptr, "RANGOS") == 0)
        {
            comando_rangos();
            continue;
        }
        if (strcmp(ptr, "SALIR") == 0 || strcmp(ptr, "exit") == 0)
        {
            printf("> Intérprete finalizado.\n");
            break;
        }
        if (strcmp(ptr, "LIMPIAR") == 0)
        {
            buffer_definiciones[0] = '\0';
            printf("> Definiciones de funciones/subprogramas borradas.\n");
            continue;
        }
        // --- COMANDO USAR: Cargar un archivo .nico en memoria ---
        if (strncmp(ptr, "USAR ", 5) == 0 || strncmp(ptr, "usar ", 5) == 0)
        {
            char *ruta = ptr + 5;
            while (*ruta == ' ' || *ruta == '\t')
                ruta++;

            if (strlen(ruta) == 0)
            {
                printf("> Uso: USAR archivo.nico\n");
                printf("> Ejemplo: USAR tortura/TestTorturaExtrema.nico\n");
                continue;
            }

            // 1. Intentar abrir la ruta tal cual la escribió el usuario
            FILE *archivo = fopen(ruta, "r");

            // 2. Si no existe, buscar dinámicamente en la carpeta de ejemplos
            if (!archivo)
            {
                // Si el usuario escribió "ejemplos/..." o "ejemplos\...", lo quitamos para buscar dentro de la carpeta ejemplos
                char *ruta_limpia = ruta;
                if (strncmp(ruta, "ejemplos/", 9) == 0)
                    ruta_limpia = ruta + 9;
                else if (strncmp(ruta, "ejemplos\\", 9) == 0)
                    ruta_limpia = ruta + 9;

                char *ruta_en_ejemplos = buscar_en_ejemplos(ruta_limpia);
                if (ruta_en_ejemplos)
                {
                    archivo = fopen(ruta_en_ejemplos, "r");
                    if (archivo)
                    {
                        printf("> Archivo encontrado automáticamente en carpeta de ejemplos.\n");
                    }
                }
            }

            if (!archivo)
            {
                fprintf(stderr, "> Error: No se pudo abrir '%s'\n", ruta);
                continue;
            }

            fseek(archivo, 0, SEEK_END);
            long tamano = ftell(archivo);
            fseek(archivo, 0, SEEK_SET);

            if (programa_cargado)
                free(programa_cargado);
            programa_cargado = malloc(tamano + 1);
            if (!programa_cargado)
            {
                fprintf(stderr, "> Error: No hay memoria suficiente.\n");
                fclose(archivo);
                continue;
            }

            fread(programa_cargado, 1, tamano, archivo);
            programa_cargado[tamano] = '\0';
            fclose(archivo);
            archivo_cargado = 1;

            printf("> Archivo '%s' cargado correctamente (%ld bytes).\n", ruta, tamano);
            continue;
        }

        // --- COMANDO CORRER: Ejecutar el archivo cargado con USAR ---
        if (strcmp(ptr, "CORRER") == 0 || strcmp(ptr, "correr") == 0)
        {
            if (!archivo_cargado || !programa_cargado)
            {
                printf("> Error: No hay ningún archivo cargado. Use USAR primero.\n");
                continue;
            }

            Lexer *lexer = lexer_crear(programa_cargado);
            Parser *parser = parser_crear(lexer);
            NodoAST *ast = parser_parsear(parser);

            if (parser_tiene_error(parser))
            {
                fprintf(stderr, "> Error de sintaxis: %s\n", parser_obtener_error(parser));
            }
            else if (ast)
            {
                printf("\n> Corriendo programa cargado...\n\n");
                evaluar_nodo(ast, ctx);
                if (ctx->hay_error)
                {
                    fprintf(stderr, "\n> Error de ejecución: %s\n", ctx->mensaje_error);
                    ctx->hay_error = 0;
                }
                else
                {
                    printf("\n> Programa finalizado.\n");
                }
                liberar_nodo(ast);
            }

            parser_destruir(parser);
            lexer_destruir(lexer);
            continue;
        }

        // --- COMANDO CERRAR: Liberar el archivo cargado ---
        if (strcmp(ptr, "CERRAR") == 0 || strcmp(ptr, "cerrar") == 0)
        {
            if (programa_cargado)
            {
                free(programa_cargado);
                programa_cargado = NULL;
            }
            archivo_cargado = 0;
            printf("> Programa cerrado.\n");
            continue;
        }

        // --- MODO DEFINICIÓN (FUNCION / SUBPROGRAMA) ---
        if (modo_definicion)
        {
            strcat(buffer_def_temp, ptr);
            strcat(buffer_def_temp, "\n");

            // Detectar el fin de la definición
            if (strstr(ptr, "FIN FUNCION") != NULL || strstr(ptr, "FIN SUBPROGRAMA") != NULL)
            {
                strcat(buffer_definiciones, buffer_def_temp);
                strcat(buffer_definiciones, "\n"); // Separador extra por seguridad
                modo_definicion = 0;
                buffer_def_temp[0] = '\0';
                printf("> Definición guardada correctamente.\n");
            }
            continue;
        }

        // Detectar inicio de definición
        if (strncmp(ptr, "FUNCION", 7) == 0 || strncmp(ptr, "SUBPROGRAMA", 11) == 0)
        {
            modo_definicion = 1;
            buffer_def_temp[0] = '\0';
            strcat(buffer_def_temp, ptr);
            strcat(buffer_def_temp, "\n");
            printf("> Modo definición. Escriba 'FIN FUNCION' o 'FIN SUBPROGRAMA' al final.\n");
            continue;
        }

        // --- MODO BLOQUE (PARA, SI, MIENTRAS, etc.) ---
        if (modo_bloque)
        {
            if (strcmp(ptr, "FINBLOQUE") == 0)
            {
                // Construir programa completo respetando la sintaxis de Nico
                char programa_completo[MAX_PROGRAMA * 2];
                if (strlen(buffer_definiciones) > 0)
                {
                    snprintf(programa_completo, sizeof(programa_completo),
                             "PROGRAMA REPL_INTERACTIVO\n%s\nBLOQUE PRINCIPAL\n%s\nFIN PRINCIPAL\nFINAL",
                             buffer_definiciones, buffer_bloque);
                }
                else
                {
                    snprintf(programa_completo, sizeof(programa_completo),
                             "PROGRAMA REPL_INTERACTIVO\nBLOQUE PRINCIPAL\n%s\nFIN PRINCIPAL\nFINAL",
                             buffer_bloque);
                }

                Lexer *lexer = lexer_crear(programa_completo);
                Parser *parser = parser_crear(lexer);
                NodoAST *ast_prog = parser_parsear(parser);

                if (parser_tiene_error(parser))
                {
                    fprintf(stderr, "Error de sintaxis: %s\n", parser_obtener_error(parser));
                }
                else if (ast_prog)
                {
                    evaluar_nodo(ast_prog, ctx);
                    if (ctx->hay_error)
                    {
                        fprintf(stderr, "Error de ejecución: %s\n", ctx->mensaje_error);
                        ctx->hay_error = 0;
                    }
                    liberar_nodo(ast_prog);
                }

                parser_destruir(parser);
                lexer_destruir(lexer);

                modo_bloque = 0;
                buffer_bloque[0] = '\0';
                printf("> Bloque ejecutado.\n");
                continue;
            }

            strcat(buffer_bloque, ptr);
            strcat(buffer_bloque, "\n");
            continue;
        }

        // Iniciar modo bloque
        if (strcmp(ptr, "BLOQUE") == 0)
        {
            modo_bloque = 1;
            buffer_bloque[0] = '\0';
            printf("> Modo bloque activado. Escriba FINBLOQUE para ejecutar.\n");
            continue;
        }

        // --- EVALUACIÓN DE LÍNEA SIMPLE ---
        // 1. Intentar como expresión directa (ej: 2 + 2)
        Lexer *lexer = lexer_crear(ptr);
        Parser *parser = parser_crear(lexer);
        NodoAST *ast_expr = parsear_expresion(parser);

        if (!parser_tiene_error(parser) && ast_expr)
        {
            Valor resultado = evaluar_nodo(ast_expr, ctx);
            if (!ctx->hay_error)
            {
                valor_imprimir(resultado);
                printf("\n");
            }
            else
            {
                fprintf(stderr, "Error: %s\n", ctx->mensaje_error);
                ctx->hay_error = 0;
            }
            valor_destruir(&resultado);
            liberar_nodo(ast_expr);
            parser_destruir(parser);
            lexer_destruir(lexer);
            continue;
        }

        // 2. Si no es expresión, envolver como instrucción dentro del esqueleto de Nico
        parser_destruir(parser);
        lexer_destruir(lexer);

        char programa_completo[MAX_PROGRAMA * 2];
        if (strlen(buffer_definiciones) > 0)
        {
            snprintf(programa_completo, sizeof(programa_completo),
                     "PROGRAMA REPL_INTERACTIVO\n%s\nBLOQUE PRINCIPAL\n%s\nFIN PRINCIPAL\nFINAL",
                     buffer_definiciones, ptr);
        }
        else
        {
            snprintf(programa_completo, sizeof(programa_completo),
                     "PROGRAMA REPL_INTERACTIVO\nBLOQUE PRINCIPAL\n%s\nFIN PRINCIPAL\nFINAL", ptr);
        }

        lexer = lexer_crear(programa_completo);
        parser = parser_crear(lexer);
        NodoAST *ast_prog = parser_parsear(parser);

        if (parser_tiene_error(parser))
        {
            fprintf(stderr, "Error de sintaxis: %s\n", parser_obtener_error(parser));
        }
        else if (ast_prog)
        {
            evaluar_nodo(ast_prog, ctx);
            if (ctx->hay_error)
            {
                fprintf(stderr, "Error de ejecución: %s\n", ctx->mensaje_error);
                ctx->hay_error = 0;
            }
            liberar_nodo(ast_prog);
        }

        parser_destruir(parser);
        lexer_destruir(lexer);
    }

    contexto_destruir(ctx);
}

int main(int argc, char *argv[])
{

#ifdef _WIN32
    SetConsoleOutputCP(65001); // Forzar salida UTF-8
    SetConsoleCP(65001);       // Forzar entrada UTF-8
#endif

#ifndef _WIN32
    struct rlimit rl;
    getrlimit(RLIMIT_STACK, &rl);
#ifdef DEBUG
    printf("Stack actual: cur=%lu, max=%lu\n", (unsigned long)rl.rlim_cur, (unsigned long)rl.rlim_max);
#endif
    if (rl.rlim_cur < 256 * 1024 * 1024)
    {
        rl.rlim_cur = 256 * 1024 * 1024;
        if (rl.rlim_cur > rl.rlim_max)
        {
            rl.rlim_max = 256 * 1024 * 1024;
        }
#ifdef DEBUG
        int result = setrlimit(RLIMIT_STACK, &rl);
        if (result != 0)
        {
            printf("Error al aumentar stack: %d (errno=%d)\n", result, errno);
        }
        else
        {
            printf("Stack aumentado a 256MB\n");
        }
#else
        setrlimit(RLIMIT_STACK, &rl);
#endif
    }
#endif

#ifdef _WIN32
    // Configurar consola Windows para UTF-8
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    inicializar_manejo_senales();
    // Registrar función de restauración de terminal para ejecución normal
    atexit(teclado_restaurar_modo);

    // Modo expresión: ./nico -e "expresión"
    if (argc == 3 && strcmp(argv[1], "-e") == 0)
    {
        evaluar_expresion(argv[2]);
        return 0;
    }
    // Ayuda
    else if (argc == 2 && (strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "--ayuda") == 0))
    {
        {
            printf("Uso: nico [opciones] [archivo.nico]\n");
            printf("Opciones:\n");
            printf("  -e <expr>    Evaluar expresión y salir\n");
            printf("  -a, --ayuda  Mostrar esta ayuda\n");
            printf("Sin opciones:  Iniciar REPL interactivo\n");
            return 0;
        }
    }

    // Modo archivo: ./nico archivo.nico
    if (argc == 2)
    {
        char *ext = strrchr(argv[1], '.');
        if (!ext || strcmp(ext, ".nico") != 0)
        {
            fprintf(stderr, "Error: El archivo debe tener extensión .nico\n");
            return 1;
        }
        ejecutar_archivo(argv[1]);
        return 0;
    }

    // Modo REPL: ./nico
    if (argc == 1)
    {
        repl();
        return 0;
    }

    fprintf(stderr, "Uso: nico [opciones] [archivo.nico]\n");
    fprintf(stderr, "Escribí 'nico -h' para más información.\n");
    return 1;
}
