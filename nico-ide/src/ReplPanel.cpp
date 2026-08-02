/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         ReplPanel.cpp
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Implementación del panel REPL. Busca el intérprete Nico
               en rutas estándar, inicia el proceso, filtra el prompt
               duplicado, gestiona el historial de comandos y comandos
               especiales del IDE (salir, limpiar, variables).
*/
#include "ReplPanel.h"
#include <QKeyEvent>
#include <QDir>
#include <QCoreApplication>

ReplPanel::ReplPanel(QWidget *parent)
    : QWidget(parent), indiceHistorial(-1)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);

    // Área de historial (solo lectura)
    areaHistorial = new QPlainTextEdit(this);
    areaHistorial->setReadOnly(true);
    areaHistorial->setFrameShape(QFrame::NoFrame);
    
    QFont font("Monospace", 11);
    font.setStyleHint(QFont::TypeWriter);
    areaHistorial->setFont(font);

    QPalette palette;
    palette.setColor(QPalette::Base, QColor(20, 20, 20));
    palette.setColor(QPalette::Text, QColor(200, 200, 200));
    areaHistorial->setPalette(palette);

    // Renderer ANSI para colores
    renderer = new AnsiRenderer(areaHistorial, this);

    // Línea de comando con prompt
    lineaComando = new QLineEdit(this);
    lineaComando->setFont(font);
    lineaComando->setPalette(palette);
    lineaComando->setFrame(false);
    lineaComando->setStyleSheet(
        "QLineEdit {"
        "  background-color: rgb(40, 40, 40);"
        "  color: rgb(220, 220, 220);"
        "  border: 1px solid rgb(60, 60, 60);"
        "  padding: 5px;"
        "}"
    );
    lineaComando->setPlaceholderText("nico>>>");

    layout->addWidget(areaHistorial, 1);
    layout->addWidget(lineaComando, 0);

    // Conexiones
    connect(lineaComando, &QLineEdit::returnPressed, this, &ReplPanel::onComandoIngresado);
    lineaComando->installEventFilter(this);

    // Iniciar proceso Nico
    iniciar();
}

ReplPanel::~ReplPanel()
{
    detener();
}

void ReplPanel::iniciar()
{
    procesoNico = new NicoRunner(this);
    connect(procesoNico, &NicoRunner::readyReadStandardOutput, this, &ReplPanel::onSalidaDisponible);
    connect(procesoNico, &NicoRunner::readyReadStandardError, this, &ReplPanel::onErrorDisponible);
    connect(procesoNico, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ReplPanel::onProcesoFinalizado);

    QString nicoPath;
    QStringList candidatas;

#ifdef Q_OS_WIN
    // En Windows, el ejecutable se llama nico.exe
    candidatas << QDir::currentPath() + "/nico.exe"
               << QDir::currentPath() + "/../nico.exe"
               << QDir::currentPath() + "/../../nico.exe"
               << "C:/Program Files/Nico/bin/nico.exe"
               << "C:/Program Files (x86)/Nico/bin/nico.exe";
#else
    // En Linux el ejecutable se llama nico (sin extensión)
    candidatas << QDir::currentPath() + "/nico"
               << QDir::currentPath() + "/../nico"
               << QDir::currentPath() + "/../../nico"
               << "/usr/local/bin/nico"
               << "/usr/bin/nico";
#endif

    for (const QString &candidata : candidatas)
    {
        QFileInfo fi(candidata);
        if (fi.exists() && fi.isExecutable())
        {
            nicoPath = fi.absoluteFilePath();
            break;
        }
    }

    if (nicoPath.isEmpty())
    {
        mostrarMensaje("╔══════════════════════════════════════════════╗\n", QColor(255, 100, 100));
        mostrarMensaje("║  ERROR: No se encontró el intérprete 'nico'  ║\n", QColor(255, 100, 100));
        mostrarMensaje("╚══════════════════════════════════════════════╝\n", QColor(255, 100, 100));
        return;
    }

    QStringList args;
    procesoNico->start(nicoPath, args);

    if (!procesoNico->waitForStarted(3000))
    {
        mostrarMensaje("\n[ERROR] No se pudo iniciar el proceso Nico.\n", QColor(255, 100, 100));
        return;
    }

    mostrarMensaje("REPL iniciado correctamente.\n", QColor(100, 255, 100));
}

void ReplPanel::detener()
{
    if (procesoNico && procesoNico->state() == QProcess::Running) {
        procesoNico->terminate();
        procesoNico->waitForFinished(1000);
    }
}

bool ReplPanel::estaActivo() const
{
    return procesoNico && procesoNico->state() == QProcess::Running;
}

void ReplPanel::onComandoIngresado()
{
    QString comando = lineaComando->text().trimmed();
    
    if (comando.isEmpty()) {
        return;
    }

    // Agregar al historial
    if (historialComandos.isEmpty() || historialComandos.last() != comando) {
        historialComandos.append(comando);
    }
    indiceHistorial = historialComandos.size();

    // Mostrar el comando en el historial
    mostrarMensaje("nico>>> " + comando + "\n", QColor(100, 255, 100));

    // Procesar comandos especiales del IDE
    if (comando == "salir" || comando == "exit" || comando == "quit") {
        procesoNico->enviarInput(comando + "\n");
        detener();
        return;
    } else if (comando == "limpiar" || comando == "clear" || comando == "cls") {
        renderer->limpiarPantalla();
        lineaComando->clear();
        return;
    } else if (comando == "variables" || comando == "vars") {
        mostrarMensaje("Comando 'variables' no implementado aún.\n", QColor(255, 200, 100));
        lineaComando->clear();
        return;
    }

    // Enviar comando al proceso Nico
    procesoNico->enviarInput(comando + "\n");
    lineaComando->clear();
}

void ReplPanel::onSalidaDisponible()
{
    QByteArray data = procesoNico->readAllStandardOutput();
    QString texto = QString::fromUtf8(data);

    // Filtrar el prompt del intérprete para que no se duplique con el del IDE
    texto.remove("nico>>> ");
    texto.remove("nico>>>");
    texto.remove(">>> ");

    // Si después de filtrar solo quedan espacios o saltos de línea, no mostrar nada
    if (texto.trimmed().isEmpty())
    {
        return;
    }

    // Renderizar con soporte ANSI
    renderer->renderizar(texto);
}

void ReplPanel::onErrorDisponible()
{
    QByteArray data = procesoNico->readAllStandardError();
    QString texto = QString::fromUtf8(data);
    
    if (!texto.trimmed().isEmpty()) {
        mostrarMensaje(texto, QColor(255, 100, 100));
    }
}

void ReplPanel::onProcesoFinalizado(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);
    mostrarMensaje("\nREPL finalizado (código: " + QString::number(exitCode) + ")\n", QColor(200, 200, 200));
}

void ReplPanel::mostrarMensaje(const QString &mensaje, const QColor &color)
{
    QTextCursor cursor(areaHistorial->document());
    cursor.movePosition(QTextCursor::End);
    
    QTextCharFormat formato;
    formato.setForeground(color);
    cursor.insertText(mensaje, formato);
    
    areaHistorial->setTextCursor(cursor);
    areaHistorial->ensureCursorVisible();
}

void ReplPanel::navegarHistorial(bool haciaArriba)
{
    if (historialComandos.isEmpty()) {
        return;
    }

    if (haciaArriba) {
        if (indiceHistorial > 0) {
            if (indiceHistorial == historialComandos.size()) {
                comandoActual = lineaComando->text();
            }
            indiceHistorial--;
            lineaComando->setText(historialComandos[indiceHistorial]);
        }
    } else {
        if (indiceHistorial < historialComandos.size() - 1) {
            indiceHistorial++;
            lineaComando->setText(historialComandos[indiceHistorial]);
        } else if (indiceHistorial == historialComandos.size() - 1) {
            indiceHistorial++;
            lineaComando->setText(comandoActual);
        }
    }
}

bool ReplPanel::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == lineaComando && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        
        if (keyEvent->key() == Qt::Key_Up) {
            navegarHistorial(true);
            return true;
        } else if (keyEvent->key() == Qt::Key_Down) {
            navegarHistorial(false);
            return true;
        }
    }
    
    return QWidget::eventFilter(obj, event);
}
