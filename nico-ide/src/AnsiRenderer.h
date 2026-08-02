/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         AnsiRenderer.h
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Declaración del renderizador de secuencias ANSI. Traduce
               códigos de escape ANSI (colores, negrita, cursor, limpiar
               pantalla) a formato Qt para mostrarlos correctamente en
               el panel de salida.
*/
#ifndef ANSIRENDERER_H
#define ANSIRENDERER_H

#include <QObject>
#include <QPlainTextEdit>
#include <QTextCharFormat>
#include <QColor>
#include <QRegularExpression>

class AnsiRenderer : public QObject
{
    Q_OBJECT

public:
    explicit AnsiRenderer(QPlainTextEdit *output, QObject *parent = nullptr);
    
    // Método principal: recibe texto crudo con códigos ANSI y lo renderiza
    void renderizar(const QString &texto);
    
    // Limpiar pantalla (cuando se detecta \033[2J)
    void limpiarPantalla();

private:
    void procesarTexto(const QString &texto);
    void aplicarFormatoActual();
    QTextCharFormat obtenerFormatoActual() const;
    QColor ansiColorAQt(int codigo, bool esFondo) const;

    QPlainTextEdit *output;
    
    // Estado actual del formato
    QColor colorTexto;
    QColor colorFondo;
    bool negrita;
    bool cursiva;
    bool subrayado;
    
    // Buffer para secuencias ANSI incompletas
    QString bufferIncompleto;
};

#endif // ANSIRENDERER_H
