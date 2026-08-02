/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         NicoSyntaxHighlighter.h
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Declaración del resaltador de sintaxis para el lenguaje
               Nico. Basado en QSyntaxHighlighter, define reglas para
               palabras clave, variables, strings, números y comentarios.
*/
#ifndef NICOSYNTAXHIGHLIGHTER_H
#define NICOSYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class NicoSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit NicoSyntaxHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat variableFormat;
};

#endif // NICOSYNTAXHIGHLIGHTER_H
