/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         OutputPanel.cpp
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Implementación del panel de salida. Configura el layout
               vertical con tema oscuro, fuente monoespaciada y
               estilos personalizados para el campo de entrada.
*/
#include "OutputPanel.h"

OutputPanel::OutputPanel(QWidget *parent) : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0); // Sin márgenes externos
    layout->setSpacing(2);

    // Panel de salida (solo lectura)
    output = new QPlainTextEdit(this);
    output->setReadOnly(true);
    output->setFrameShape(QFrame::NoFrame);
    
    QFont font("Monospace", 11);
    font.setStyleHint(QFont::TypeWriter);
    output->setFont(font);

    QPalette palette;
    palette.setColor(QPalette::Base, QColor(20, 20, 20));
    palette.setColor(QPalette::Text, QColor(200, 200, 200));
    output->setPalette(palette);

    // Campo de entrada para enviar input al proceso
    input = new QLineEdit(this);
    input->setPlaceholderText("Escribí aquí y presioná ENTER para enviar input al programa...");
    input->setFont(font);
    input->setPalette(palette);
    input->setFrame(false); // Elimina el borde del input también
    
    // Estilo del input
    input->setStyleSheet(
        "QLineEdit {"
        "  background-color: rgb(40, 40, 40);"
        "  color: rgb(220, 220, 220);"
        "  border: 1px solid rgb(60, 60, 60);"
        "  padding: 3px;"
        "}"
    );

    layout->addWidget(output, 1);
    layout->addWidget(input, 0);
    setStyleSheet("OutputPanel { border: none; background-color: rgb(20, 20, 20); }");
}

void OutputPanel::appendPlainText(const QString &text)
{
    output->appendPlainText(text);
}

void OutputPanel::clear()
{
    output->clear();
}
