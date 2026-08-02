/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         CodeEditor.h
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Editor de código personalizado basado en QPlainTextEdit.
               Provee números de línea, resaltado de sintaxis opcional
               mediante NicoSyntaxHighlighter y configuración de fuente
               monoespaciada con tema oscuro.
*/
#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include "NicoSyntaxHighlighter.h"

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);
    
    // Método para controlar el resaltado
    void setResaltadoActivo(bool activo);
    bool getResaltadoActivo() const { return highlighter != nullptr; }

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QWidget *lineNumberArea;
    NicoSyntaxHighlighter *highlighter;

    int lineNumberAreaWidth();
    void lineNumberAreaPaintEvent(QPaintEvent *event);

    friend class LineNumberArea;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor) {}

    QSize sizeHint() const override { return QSize(codeEditor->lineNumberAreaWidth(), 0); }

protected:
    void paintEvent(QPaintEvent *event) override { codeEditor->lineNumberAreaPaintEvent(event); }

private:
    CodeEditor *codeEditor;
};

#endif // CODEEDITOR_H
