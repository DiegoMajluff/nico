/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         MainWindow.h
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Declaración de la ventana principal del IDE. Gestiona menús,
               editor de código, panel de salida, ejecución de programas
               Nico mediante QProcess, resaltado de sintaxis, búsqueda,
               consola REPL y configuración persistente (geometría,
               tamaño de fuente, splitter).
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QLabel>
#include <QTimer>
#include <QDialog>
#include <QCloseEvent>
#include <QRect>
#include "CodeEditor.h"
#include "OutputPanel.h"
#include "NicoRunner.h"
#include "FindDialog.h"
#include "AnsiRenderer.h"
#include "ReplPanel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void abrirArchivoDesdeArgumentos(const QString &rutaArchivo);

private slots:
    // Archivo
    void onNuevo();
    void onAbrir();
    void onGuardar();
    void onGuardarComo();
    void onCerrar();
    void onSalir();

    // Ejecutar
    void onEjecutar();
    void onDetener();
    void onLimpiarSalida();

    // Edición
    void onCortar();
    void onCopiar();
    void onPegar();
    void onSeleccionarTodo();
    void onBuscar();

    // Vista
    void onAumentarFuente();
    void onDisminuirFuente();
    void onMaximizarRestaurar();
    void onVentanaSalidaFlotante();

    // Ayuda
    void onAcercaDe();

    // Proceso
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();

    void onConsolaREPL();

protected:
    // Event handlers de Qt (van en protected)
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
    bool event(QEvent *event) override;

private:
    void crearMenus();
    void crearStatusBar();
    void actualizarTitulo();
    bool preguntarGuardar();
    QString buscarInterprete(const QString &dirArchivo);
    void actualizarFuente();
    void actualizarEstadoMenus();
    void guardarConfiguracion();
    void restaurarConfiguracion();

    CodeEditor *editor;
    OutputPanel *output;
    NicoRunner *runner;
    AnsiRenderer *ansiRenderer;
    QSplitter *splitter;

    QString archivoActual;
    bool archivoModificado;
    int tamañoFuente;
    QString tipoArchivoActual;

    QDialog *ventanaSalidaFlotante;
    FindDialog *dialogoBusqueda;

    QAction *accionEjecutar;
    QAction *accionDetener;
    QAction *accionLimpiarSalida;

    QRect geometriaNormal;

    QDialog *ventanaREPL;
    ReplPanel *replPanel;
};

#endif // MAINWINDOW_H