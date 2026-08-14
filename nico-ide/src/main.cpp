/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         main.cpp
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Punto de entrada del IDE Qt5. Inicializa QApplication,
               carga traducciones al español (qt y qtbase) y muestra
               la ventana principal. Soporta abrir archivos .nico desde
               argumentos de línea de comandos.
*/
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include "MainWindow.h"
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qputenv("QT_LOGGING_RULES", "qt.qpa.wayland=false");
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    app.setApplicationName("Nico IDE");
    app.setApplicationVersion("2.1.0");

    QTranslator traductorQt;
    if (traductorQt.load(QLocale(QLocale::Spanish, QLocale::LatinAmerica),
                         "qt", "_",
                         QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
    {
        app.installTranslator(&traductorQt);
    }

    // Traductor para qtbase (mensajes del sistema)
    QTranslator traductorBase;
    if (traductorBase.load(QLocale(QLocale::Spanish, QLocale::LatinAmerica),
                           "qtbase", "_",
                           QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
    {
        app.installTranslator(&traductorBase);
    }

    MainWindow window;
    window.show();

    if (argc > 1)
    {
        QString rutaArchivo = QString::fromUtf8(argv[1]);
        window.abrirArchivoDesdeArgumentos(rutaArchivo);
    }

    return app.exec();
}