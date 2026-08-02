/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         NicoRunner.h
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Declaración del wrapper de QProcess para ejecutar el
               intérprete Nico. Provee métodos para iniciar la ejecución
               con un archivo y enviar input al proceso en ejecución.
*/
#ifndef NICORUNNER_H
#define NICORUNNER_H

#include <QProcess>

class NicoRunner : public QProcess
{
    Q_OBJECT

public:
    explicit NicoRunner(QObject *parent = nullptr);
    void ejecutar(const QString &nicoPath, const QString &archivo);
    void enviarInput(const QString &texto);
};

#endif // NICORUNNER_H
