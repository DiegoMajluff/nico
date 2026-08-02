/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         ReplPanel.h
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Declaración del panel REPL (Read-Eval-Print Loop).
               Consola interactiva con historial de comandos, navegación
               con flechas arriba/abajo y renderizado ANSI para la salida.
*/
#ifndef REPLPANEL_H
#define REPLPANEL_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QStringList>
#include "NicoRunner.h"
#include "AnsiRenderer.h"

class ReplPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ReplPanel(QWidget *parent = nullptr);
    ~ReplPanel();

    void iniciar();
    void detener();
    bool estaActivo() const;

private slots:
    void onComandoIngresado();
    void onSalidaDisponible();
    void onErrorDisponible();
    void onProcesoFinalizado(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void mostrarMensaje(const QString &mensaje, const QColor &color);
    void navegarHistorial(bool haciaArriba);
    void procesarComandoEspecial(const QString &comando);

    bool eventFilter(QObject *obj, QEvent *event) override;

    QPlainTextEdit *areaHistorial;
    QLineEdit *lineaComando;
    NicoRunner *procesoNico;
    AnsiRenderer *renderer;
    
    QStringList historialComandos;
    int indiceHistorial;
    QString comandoActual;
};

#endif // REPLPANEL_H
