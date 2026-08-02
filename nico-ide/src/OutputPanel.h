/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         OutputPanel.h
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Declaración del panel de salida del IDE. Combina un área
               de texto de solo lectura (salida del programa) con un
               campo de entrada (para enviar input al proceso en ejecución).
*/
#ifndef OUTPUTPANEL_H
#define OUTPUTPANEL_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>

class OutputPanel : public QWidget
{
    Q_OBJECT

public:
    explicit OutputPanel(QWidget *parent = nullptr);
    
    QPlainTextEdit* getOutput() { return output; }
    QLineEdit* getInput() { return input; }
    
    void appendPlainText(const QString &text);
    void clear();

private:
    QPlainTextEdit *output;
    QLineEdit *input;
    QVBoxLayout *layout;
};

#endif // OUTPUTPANEL_H
