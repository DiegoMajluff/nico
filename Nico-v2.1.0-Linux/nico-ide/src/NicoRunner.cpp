/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         NicoRunner.cpp
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Implementación del wrapper de QProcess. Configura canales
               separados para stdout/stderr, inicia el intérprete con
               el archivo indicado y permite enviar texto a stdin.
*/
#include "NicoRunner.h"
#include <QIODevice>

NicoRunner::NicoRunner(QObject *parent) : QProcess(parent)
{
    // Configurar canales separados para stdout y stderr
    setProcessChannelMode(QProcess::SeparateChannels);
}

void NicoRunner::ejecutar(const QString &nicoPath, const QString &archivo)
{
    QStringList arguments;
    arguments << archivo;
    
    // Iniciar con ReadWrite para poder escribir en stdin
    start(nicoPath, arguments);
    
    // Esperar para que el proceso arranque
    waitForStarted(1000);
}

void NicoRunner::enviarInput(const QString &texto)
{
    if (state() == QProcess::Running) {
        // Escribir el texto seguido de salto de línea
        QByteArray data = (texto + "\n").toUtf8();
        write(data);
        waitForBytesWritten(100); // Esperar a que se escriba
    }
}
