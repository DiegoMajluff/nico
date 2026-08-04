/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         MainWindow.cpp
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Implementación de la ventana principal del IDE. Incluye
               gestión de archivos (nuevo, abrir, guardar, cerrar),
               ejecución/detención de programas Nico, búsqueda,
               ventana de salida flotante, consola REPL, manejo de
               eventos de cierre y persistencia de configuración.
*/
#include "MainWindow.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QApplication>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QSettings>
#include <QCloseEvent>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), archivoModificado(false), tamañoFuente(12),
      ventanaSalidaFlotante(nullptr), dialogoBusqueda(nullptr),
      tipoArchivoActual("ninguno"),
      accionEjecutar(nullptr), accionDetener(nullptr), accionLimpiarSalida(nullptr),
      ventanaREPL(nullptr), replPanel(nullptr)
{
    setWindowTitle("Nico IDE v2.1.0");
    resize(1024, 700);

    crearMenus();
    crearStatusBar();

    // Layout con splitter ultrafino
    splitter = new QSplitter(Qt::Vertical, this);
    splitter->setHandleWidth(2);
    setCentralWidget(splitter);

    editor = new CodeEditor(this);
    output = new OutputPanel(this);

    splitter->addWidget(editor);
    splitter->addWidget(output);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    // Runner
    runner = new NicoRunner(this);
    ansiRenderer = new AnsiRenderer(output->getOutput(), this);

    connect(runner, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, &MainWindow::onProcessFinished);
    connect(runner, &NicoRunner::errorOccurred, this, &MainWindow::onProcessError);
    connect(runner, &NicoRunner::readyReadStandardOutput, this, &MainWindow::onReadyReadStandardOutput);
    connect(runner, &NicoRunner::readyReadStandardError, this, &MainWindow::onReadyReadStandardError);

    connect(editor->document(), &QTextDocument::modificationChanged, [this](bool modified)
            {
        if (archivoModificado != modified) {
            archivoModificado = modified;
            actualizarTitulo();
        } });

    connect(output->getInput(), &QLineEdit::returnPressed, [this]() {
        QString texto = output->getInput()->text();
        if (!texto.isEmpty()) {
            output->getOutput()->appendPlainText("> " + texto);
        }
        runner->enviarInput(texto);
        output->getInput()->clear();
    });

    actualizarTitulo();
    actualizarFuente();
    actualizarEstadoMenus();
    editor->setResaltadoActivo(false);
    restaurarConfiguracion();
}

void MainWindow::abrirArchivoDesdeArgumentos(const QString &rutaArchivo)
{
    QFile file(rutaArchivo);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in.setCodec("UTF-8");
        editor->setPlainText(in.readAll());
        editor->document()->setModified(false);
        file.close();
        
        archivoActual = rutaArchivo;
        archivoModificado = false;
        
        // Detectar el tipo de archivo por la extensión
        if (rutaArchivo.endsWith(".nico", Qt::CaseInsensitive)) {
            tipoArchivoActual = "nico";
            editor->setResaltadoActivo(true);
        } else {
            tipoArchivoActual = "txt";
            editor->setResaltadoActivo(false);
        }
        
        actualizarTitulo();
        actualizarEstadoMenus();
        statusBar()->showMessage("Abierto: " + rutaArchivo);
    }
}

MainWindow::~MainWindow()
{
    // Asegurarse de que el proceso principal esté terminado
    if (runner && runner->state() != QProcess::NotRunning) {
        runner->kill();
        runner->waitForFinished(1000);
    }
    
    // Detener el REPL si está abierto
    if (replPanel) {
        replPanel->detener();
    }
}

void MainWindow::crearMenus()
{
    QMenuBar *menuBar = this->menuBar();

    // --- Menú Archivo ---
    QMenu *menuArchivo = menuBar->addMenu("&Archivo");
    menuArchivo->addAction("&Nuevo", this, &MainWindow::onNuevo, QKeySequence::New);
    menuArchivo->addAction("&Abrir...", this, &MainWindow::onAbrir, QKeySequence::Open);
    menuArchivo->addSeparator();
    menuArchivo->addAction("&Guardar", this, &MainWindow::onGuardar, QKeySequence::Save);
    menuArchivo->addAction("Guardar &como...", this, &MainWindow::onGuardarComo, QKeySequence::SaveAs);
    menuArchivo->addSeparator();
    menuArchivo->addAction("&Cerrar", this, &MainWindow::onCerrar, QKeySequence("Ctrl+W")); // ← NUEVO
    menuArchivo->addSeparator();
    menuArchivo->addAction("&Salir", this, &MainWindow::onSalir, QKeySequence::Quit);

    // --- Menú Edición ---
    QMenu *menuEdicion = menuBar->addMenu("&Edición");
    menuEdicion->addAction("Cor&tar", this, &MainWindow::onCortar, QKeySequence::Cut);
    menuEdicion->addAction("&Copiar", this, &MainWindow::onCopiar, QKeySequence::Copy);
    menuEdicion->addAction("&Pegar", this, &MainWindow::onPegar, QKeySequence::Paste);
    menuEdicion->addSeparator();
    menuEdicion->addAction("Seleccionar &todo", this, &MainWindow::onSeleccionarTodo, QKeySequence::SelectAll);
    menuEdicion->addSeparator();
    menuEdicion->addAction("&Buscar...", this, &MainWindow::onBuscar, QKeySequence::Find);

    // --- Menú Ejecutar ---
    QMenu *menuEjecutar = menuBar->addMenu("E&jecutar");
    accionEjecutar = menuEjecutar->addAction("&Ejecutar", this, &MainWindow::onEjecutar, QKeySequence(Qt::Key_F5));
    accionDetener = menuEjecutar->addAction("&Detener", this, &MainWindow::onDetener, QKeySequence(Qt::Key_F6));
    menuEjecutar->addSeparator();
    accionLimpiarSalida = menuEjecutar->addAction("&Limpiar salida", this, &MainWindow::onLimpiarSalida, QKeySequence("Ctrl+L"));

    // --- Menú Ver ---
    QMenu *menuVer = menuBar->addMenu("&Ver");
    menuVer->addAction("Aumentar tamaño de fuente", this, &MainWindow::onAumentarFuente, QKeySequence::ZoomIn);
    menuVer->addAction("Disminuir tamaño de fuente", this, &MainWindow::onDisminuirFuente, QKeySequence::ZoomOut);
    menuVer->addSeparator();
    menuVer->addAction("Maximizar / Restaurar", this, &MainWindow::onMaximizarRestaurar, QKeySequence(Qt::Key_F11)); // ← NUEVO
    menuVer->addSeparator();
    menuVer->addAction("Ventana de salida flotante", this, &MainWindow::onVentanaSalidaFlotante);
    menuVer->addAction("Consola REPL", this, &MainWindow::onConsolaREPL, QKeySequence("Ctrl+R"));

    // --- Menú Ayuda ---
    QMenu *menuAyuda = menuBar->addMenu("A&yuda");
    menuAyuda->addAction("&Acerca de...", this, &MainWindow::onAcercaDe);
}

void MainWindow::crearStatusBar()
{
    statusBar()->showMessage("Listo");
}

void MainWindow::actualizarTitulo()
{
    QString titulo = "Nico IDE v2.1.0";
    if (!archivoActual.isEmpty()) {
        QFileInfo fi(archivoActual);
        titulo = fi.fileName() + (archivoModificado ? " *" : "") + " - Nico IDE v2.1.0";
    } else if (archivoModificado) {
        titulo = "Sin título * - Nico IDE v2.1.0";
    }
    setWindowTitle(titulo);
}

void MainWindow::actualizarEstadoMenus()
{
    bool esNico = (tipoArchivoActual == "nico");
    
    if (accionEjecutar) {
        accionEjecutar->setEnabled(esNico);
    }
    if (accionDetener) {
        accionDetener->setEnabled(esNico);
    }
    if (accionLimpiarSalida) {
        accionLimpiarSalida->setEnabled(esNico);
    }
}

void MainWindow::actualizarFuente()
{
    QFont font("Monospace", tamañoFuente);
    font.setStyleHint(QFont::TypeWriter);
    editor->setFont(font);
    
    output->getOutput()->setFont(font);
    output->getInput()->setFont(font);
}

bool MainWindow::preguntarGuardar()
{
    if (!archivoModificado)
        return true;

    QString nombreArchivo = archivoActual.isEmpty() ? "Sin título" : QFileInfo(archivoActual).fileName();

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Nico IDE - Cambios sin guardar");
    msgBox.setText("El archivo \"" + nombreArchivo + "\" tiene cambios sin guardar.");
    msgBox.setInformativeText("¿Qué desea hacer antes de continuar?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setIcon(QMessageBox::Warning);

    int respuesta = msgBox.exec();

    if (respuesta == QMessageBox::Save)
    {
        onGuardar();
        // Si el usuario canceló el guardado (por ejemplo, cerró el diálogo de "Guardar como"),
        // entonces no continuar
        return !archivoModificado;
    }
    else if (respuesta == QMessageBox::Discard)
    {
        return true; // Descartar cambios y continuar
    }
    else
    {
        return false; // Cancelar la operación
    }
}

void MainWindow::onNuevo()
{
    if (!preguntarGuardar())
        return;

    // Mostrar diálogo para elegir el tipo de archivo
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Nuevo archivo");
    msgBox.setText("¿Qué tipo de archivo desea crear?");

    QPushButton *btnNico = msgBox.addButton("Archivo Nico (.nico)", QMessageBox::AcceptRole);
    QPushButton *btnTxt = msgBox.addButton("Archivo de texto (.txt)", QMessageBox::AcceptRole);
    QPushButton *btnCancelar = msgBox.addButton(QMessageBox::Cancel);

    msgBox.setDefaultButton(btnNico);
    msgBox.exec();

    if (msgBox.clickedButton() == btnCancelar)
    {
        return;
    }

    // Limpiar completamente
    editor->clear();
    editor->document()->setModified(false);
    archivoActual.clear();
    archivoModificado = false;

    // Limpiar panel de salida
    output->clear();

    // Configurar el resaltado según el tipo de archivo
    if (msgBox.clickedButton() == btnNico)
    {
        tipoArchivoActual = "nico";
        editor->setResaltadoActivo(true);
    }
    else
    {
        tipoArchivoActual = "txt";
        editor->setResaltadoActivo(false);
    }

    actualizarTitulo();
    actualizarEstadoMenus();
    statusBar()->showMessage("Nuevo archivo " + tipoArchivoActual.toUpper(), 3000);
}

void MainWindow::onAbrir()
{
    if (!preguntarGuardar()) return;

    QString fileName = QFileDialog::getOpenFileName(
        this, "Abrir archivo", "",
        "*.nico;;*.txt;;*.*"
    );

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            in.setCodec("UTF-8");
            editor->setPlainText(in.readAll());
            editor->document()->setModified(false);
            file.close();
            
            archivoActual = fileName;
            archivoModificado = false;
            
            // Detectar el tipo de archivo por la extensión
            if (fileName.endsWith(".nico", Qt::CaseInsensitive)) {
                tipoArchivoActual = "nico";
                editor->setResaltadoActivo(true);
            } else {
                tipoArchivoActual = "txt";
                editor->setResaltadoActivo(false);
            }
            
            actualizarTitulo();
            actualizarEstadoMenus();
            statusBar()->showMessage("Abierto: " + fileName);
        }
    }
}

void MainWindow::onCerrar()
{
    if (!preguntarGuardar())
        return;

    // Limpiar completamente el estado
    editor->clear();
    editor->document()->setModified(false); // Resetear el flag de modificación del documento

    archivoActual.clear();
    archivoModificado = false;
    tipoArchivoActual = "ninguno";

    // Desactivar resaltado de sintaxis
    editor->setResaltadoActivo(false);

    // Actualizar interfaz
    actualizarTitulo();
    actualizarEstadoMenus();

    // Limpiar el panel de salida también
    output->clear();

    statusBar()->showMessage("Archivo cerrado", 3000); // Mensaje temporal (3 segundos)
}

void MainWindow::onGuardar()
{
    if (archivoActual.isEmpty())
    {
        onGuardarComo();
        return;
    }

    QFile file(archivoActual);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QString errorMsg;
        QFile::FileError err = file.error();
        if (err == QFile::PermissionsError)
        {
            errorMsg = "No tiene permisos de escritura sobre el archivo.\n\n"
                       "Archivo: " +
                       archivoActual + "\n\n"
                                       "Solución: Verifique los permisos del archivo o guarde con otro nombre.";
        }
        else
        {
            errorMsg = "No se pudo abrir el archivo para escritura.\n\n"
                       "Archivo: " +
                       archivoActual + "\n\n"
                                       "Error: " +
                       file.errorString();
        }
        QMessageBox::critical(this, "Nico IDE - Error al guardar", errorMsg);
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << editor->toPlainText();
    file.close();

    editor->document()->setModified(false);
    archivoModificado = false;
    actualizarTitulo();
    statusBar()->showMessage("Guardado: " + archivoActual);
}

void MainWindow::onGuardarComo()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, "Guardar archivo", "",
        "*.nico;;*.txt;;*.*");

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".nico") && !fileName.endsWith(".txt"))
        fileName += ".nico";

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QString errorMsg = "No se pudo crear/escribir el archivo.\n\n"
                           "Archivo: " +
                           fileName + "\n\n"
                                      "Error: " +
                           file.errorString();
        QMessageBox::critical(this, "Nico IDE - Error al guardar", errorMsg);
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << editor->toPlainText();
    file.close();

    editor->document()->setModified(false);
    archivoActual = fileName;

    if (fileName.endsWith(".nico", Qt::CaseInsensitive))
    {
        tipoArchivoActual = "nico";
        editor->setResaltadoActivo(true);
    }
    else
    {
        tipoArchivoActual = "txt";
        editor->setResaltadoActivo(false);
    }

    archivoModificado = false;
    actualizarTitulo();
    actualizarEstadoMenus();
    statusBar()->showMessage("Guardado: " + fileName);
}

QString MainWindow::buscarInterprete(const QString &dirArchivo)
{
    QStringList candidatas;
    
#ifdef Q_OS_WIN
    // En Windows, el ejecutable se llama nico.exe
    candidatas 
        << dirArchivo + "/nico.exe"
        << dirArchivo + "/../../nico.exe"
        << dirArchivo + "/../../../nico.exe"
        << QDir::currentPath() + "/nico.exe"
        << QDir::currentPath() + "/../nico.exe"
        << QDir::currentPath() + "/../../nico.exe"
        << "C:/Program Files/Nico/bin/nico.exe"
        << "C:/Program Files (x86)/Nico/bin/nico.exe";
#else
    // En Linux/macOS, el ejecutable se llama nico (sin extensión)
    candidatas 
        << dirArchivo + "/nico"
        << dirArchivo + "/../../nico"
        << dirArchivo + "/../../../nico"
        << QDir::currentPath() + "/nico"
        << QDir::currentPath() + "/../nico"
        << QDir::currentPath() + "/../../nico"
        << "/usr/local/bin/nico"
        << "/usr/bin/nico";
#endif

    for (const QString &candidata : candidatas) {
        QFileInfo fi(candidata);
        if (fi.exists() && fi.isExecutable()) {
            return fi.absoluteFilePath();
        }
    }
    return QString();
}

void MainWindow::onEjecutar()
{
    if (!archivoActual.isEmpty() && archivoModificado) {
        onGuardar();
    }

    if (archivoActual.isEmpty()) {
        QMessageBox::warning(this, "Nico IDE", "Guarde el archivo antes de ejecutar.");
        return;
    }

    QFileInfo fiArchivo(archivoActual);
    QString dirArchivo = fiArchivo.absolutePath();
    QString nicoPath = buscarInterprete(dirArchivo);

    if (nicoPath.isEmpty()) {
        QMessageBox::critical(this, "Nico IDE - Error",
            "No se encontró el intérprete 'nico'.\n\n"
            "Solución: Copie el binario 'nico' al directorio de su archivo .nico\n"
            "O ejecute el IDE desde la carpeta raíz del proyecto.");
        return;
    }

    output->clear();
    ansiRenderer->limpiarPantalla();
    QFileInfo fi(archivoActual);
    output->getOutput()->appendPlainText("// Ejecutando: " + fi.fileName());
    statusBar()->showMessage("Ejecutando...");

    runner->ejecutar(nicoPath, archivoActual);
}

void MainWindow::onDetener()
{
    if (runner->state() == QProcess::Running) {
        runner->terminate();
        if (!runner->waitForFinished(3000)) {
            runner->kill();
            runner->waitForFinished(1000);
        }
        output->getOutput()->appendPlainText("\n>>> Programa detenido por el usuario.");
        statusBar()->showMessage("Detenido");
    }
}

void MainWindow::onLimpiarSalida()
{
    output->clear();
}

void MainWindow::onSalir()
{
    // Si hay un proceso en ejecución, advertir
    if (runner->state() == QProcess::Running)
    {
        QMessageBox::warning(this, "Nico IDE",
                             "Hay un programa en ejecución.\n\n"
                             "Detenga el programa antes de salir, o el proceso continuará en segundo plano.");
        return;
    }

    if (preguntarGuardar())
    {
        qApp->quit();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Si hay un proceso en ejecución, preguntar
    if (runner->state() == QProcess::Running)
    {
        QMessageBox::StandardButton respuesta = QMessageBox::question(
            this, "Nico IDE",
            "Hay un programa en ejecución.\n\n"
            "¿Desea detenerlo y salir?",
            QMessageBox::Yes | QMessageBox::No);
        if (respuesta == QMessageBox::No)
        {
            event->ignore();
            return;
        }
        runner->terminate();
        if (!runner->waitForFinished(3000)) {
            runner->kill();
            runner->waitForFinished(1000);
        }
    }
    
    // Detener el REPL si está abierto
    if (replPanel) {
        replPanel->detener();
    }
    
    // Preguntar si hay cambios sin guardar
    if (preguntarGuardar())
    {
        guardarConfiguracion();
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        // Si la ventana pasa de maximizada a normal, guardar la geometría
        if (!isMaximized() && !geometriaNormal.isNull())
        {
            // La geometría ya fue restaurada automáticamente por Qt
        }
        else if (isMaximized())
        {
            // Antes de maximizar, guardar la geometría actual
            geometriaNormal = normalGeometry();
        }
    }
    QMainWindow::changeEvent(event);
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);

#ifdef Q_OS_WIN
    // En Windows, interceptar el doble clic en la barra de título
    if (eventType == "windows_generic_MSG")
    {
        MSG *msg = static_cast<MSG *>(message);
        if (msg->message == WM_NCLBUTTONDBLCLK)
        {
            if (isMaximized())
            {
                showNormal();
            }
            else
            {
                showMaximized();
            }
            return true;
        }
    }
#endif

    // En Linux, el doble clic lo maneja el compositor del escritorio (gsettings)
    // No necesitamos interceptarlo aquí
    return QMainWindow::nativeEvent(eventType, message, result);
}

bool MainWindow::event(QEvent *event)
{
    // Capturar doble clic en la barra de título (área no-cliente)
    if (event->type() == QEvent::NonClientAreaMouseButtonDblClick)
    {
        if (isMaximized())
        {
            showNormal();
        }
        else
        {
            showMaximized();
        }
        return true; // Evento manejado
    }
    return QMainWindow::event(event);
}

void MainWindow::guardarConfiguracion()
{
    QSettings settings("NicoIDE", "Nico");

    // Guardar geometría (solo si no está maximizada)
    if (!isMaximized())
    {
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
    }
    settings.setValue("maximized", isMaximized());

    // Guardar tamaño de fuente
    settings.setValue("tamanioFuente", tamañoFuente);

    // Guardar posición del splitter
    settings.setValue("splitterState", splitter->saveState());

    settings.sync();
}

void MainWindow::restaurarConfiguracion()
{
    QSettings settings("NicoIDE", "Nico");

    // Restaurar geometría
    if (settings.contains("geometry"))
    {
        restoreGeometry(settings.value("geometry").toByteArray());
        restoreState(settings.value("windowState").toByteArray());
    }

    if (settings.value("maximized", false).toBool())
    {
        showMaximized();
    }

    // Restaurar tamaño de fuente
    int tamanioGuardado = settings.value("tamanioFuente", 12).toInt();
    if (tamanioGuardado >= 8 && tamanioGuardado <= 40)
    {
        tamañoFuente = tamanioGuardado;
        actualizarFuente();
    }

    // Restaurar posición del splitter
    if (settings.contains("splitterState"))
    {
        splitter->restoreState(settings.value("splitterState").toByteArray());
    }
}

void MainWindow::onAcercaDe()
{
    QMessageBox::about(this, "Acerca de Nico IDE",
        "Nico IDE v2.1.0\n\n"
        "Entorno de desarrollo integrado para el lenguaje Nico.\n\n"
        "Diseño y Arquitectura: Diego Alejandro Majluff\n"
        "Implementación: Qwen (Alibaba Cloud)\n\n"
        "Licencia: MIT / Uso Educativo");
}

void MainWindow::onCortar() { editor->cut(); }
void MainWindow::onCopiar() { editor->copy(); }
void MainWindow::onPegar() { editor->paste(); }
void MainWindow::onSeleccionarTodo() { editor->selectAll(); }

void MainWindow::onBuscar()
{
    if (!dialogoBusqueda) {
        dialogoBusqueda = new FindDialog(editor, this);
    }
    dialogoBusqueda->show();
    dialogoBusqueda->raise();
    dialogoBusqueda->activateWindow();
}

void MainWindow::onAumentarFuente()
{
    tamañoFuente++;
    actualizarFuente();
}

void MainWindow::onDisminuirFuente()
{
    if (tamañoFuente > 8) {
        tamañoFuente--;
        actualizarFuente();
    }
}

void MainWindow::onMaximizarRestaurar()
{
    if (isMaximized())
    {
        showNormal();
    }
    else
    {
        showMaximized();
    }
}

void MainWindow::onVentanaSalidaFlotante()
{
    if (!ventanaSalidaFlotante) {
        ventanaSalidaFlotante = new QDialog(this);
        ventanaSalidaFlotante->setWindowTitle("Consola de Salida - Nico IDE");
        ventanaSalidaFlotante->resize(800, 400);
        ventanaSalidaFlotante->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
        ventanaSalidaFlotante->setStyleSheet("QDialog { border: 1px solid #444; background-color: rgb(20, 20, 20); }");
        QVBoxLayout *layout = new QVBoxLayout(ventanaSalidaFlotante);
        
        // setParent() automáticamente remueve el widget del splitter anterior
        output->setParent(ventanaSalidaFlotante);
        layout->addWidget(output);
        
        // Cuando se cierre la ventana flotante, lo devolvemos al splitter
        connect(ventanaSalidaFlotante, &QDialog::finished, [this](int) {
            output->setParent(splitter);
            splitter->addWidget(output);
            splitter->setStretchFactor(0, 3);
            splitter->setStretchFactor(1, 1);
            output->show();
            ventanaSalidaFlotante = nullptr;
        });
    }
    ventanaSalidaFlotante->show();
    ventanaSalidaFlotante->raise();
    ventanaSalidaFlotante->activateWindow();
}

void MainWindow::onConsolaREPL()
{
    if (!ventanaREPL)
    {
        ventanaREPL = new QDialog(this);
        ventanaREPL->setAttribute(Qt::WA_DeleteOnClose);
        ventanaREPL->setWindowTitle("Consola REPL - Nico IDE");
        ventanaREPL->resize(800, 500);
        ventanaREPL->setWindowFlags(
            ventanaREPL->windowFlags() |
            Qt::WindowMinMaxButtonsHint |
            Qt::WindowCloseButtonHint);
        QVBoxLayout *layout = new QVBoxLayout(ventanaREPL);
        layout->setContentsMargins(0, 0, 0, 0);
        replPanel = new ReplPanel(ventanaREPL);
        layout->addWidget(replPanel);
        // Cuando se cierre la ventana, limpiar referencias
        // (el proceso se detiene en el destructor de ReplPanel)
        connect(ventanaREPL, &QDialog::finished, [this](int)
                {
            ventanaREPL = nullptr;
            replPanel = nullptr;
        });
    }
    ventanaREPL->show();
    ventanaREPL->raise();
    ventanaREPL->activateWindow();
}

// ==========================================
// MANEJO DEL PROCESO
// ==========================================

void MainWindow::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);
    output->getOutput()->appendPlainText("\n// Fin (" + QString::number(exitCode) + ")");
    statusBar()->showMessage("Listo");
}

void MainWindow::onProcessError(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error) {
        case QProcess::FailedToStart: errorMsg = "No se pudo iniciar el proceso."; break;
        case QProcess::Crashed: errorMsg = "El proceso terminó inesperadamente."; break;
        case QProcess::Timedout: errorMsg = "Tiempo de espera agotado."; break;
        default: errorMsg = "Error desconocido.";
    }
    output->getOutput()->appendPlainText("\n[ERROR] " + errorMsg + "\n");
    statusBar()->showMessage("Error");
}

void MainWindow::onReadyReadStandardOutput()
{
    QByteArray data = runner->readAllStandardOutput();
    // Pasar el texto crudo al renderer ANSI en lugar de appendPlainText directo
    ansiRenderer->renderizar(QString::fromUtf8(data));
}

void MainWindow::onReadyReadStandardError()
{
    QByteArray data = runner->readAllStandardError();
    QString errorText = QString::fromUtf8(data).trimmed();
    if (!errorText.isEmpty()) {
        ansiRenderer->renderizar(errorText + "\n");
    }
}
