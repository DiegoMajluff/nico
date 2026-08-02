/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         CodeEditor.cpp
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Implementación del editor de código. Maneja el área de
               números de línea, activación/desactivación del resaltador
               de sintaxis y configuración visual (fuente, colores,
               tabulaciones).
*/
#include "CodeEditor.h"
#include <QPainter>
#include <QTextBlock>
#include <QResizeEvent>

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent), highlighter(nullptr)
{
    lineNumberArea = new LineNumberArea(this);
    
    // Por defecto, activar el resaltado
    setResaltadoActivo(true);

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);

    updateLineNumberAreaWidth(0);

    // Configuración del editor
    QFont font("Monospace", 12);
    font.setStyleHint(QFont::TypeWriter);
    setFont(font);
    setTabStopDistance(40);

    // Tema oscuro
    QPalette palette;
    palette.setColor(QPalette::Base, QColor(30, 30, 30));
    palette.setColor(QPalette::Text, QColor(220, 220, 220));
    setPalette(palette);
}

void CodeEditor::setResaltadoActivo(bool activo)
{
    if (activo && !highlighter) {
        // Crear el resaltador si no existe
        highlighter = new NicoSyntaxHighlighter(document());
    } else if (!activo && highlighter) {
        // Eliminar el resaltador si existe
        delete highlighter;
        highlighter = nullptr;
    }
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 10 + fontMetrics().horizontalAdvance('9') * digits;
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(40, 40, 40));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(100, 100, 100));
            painter.drawText(0, top, lineNumberArea->width() - 5, fontMetrics().height(),
                           Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
