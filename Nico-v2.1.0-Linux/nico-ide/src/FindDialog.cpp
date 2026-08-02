/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         FindDialog.cpp
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Implementación del diálogo de búsqueda. Construye la UI
               con campo de texto, checkboxes de opciones y botones de
               navegación. Maneja el wrap-around al llegar al inicio/final
               del documento.
*/
#include "FindDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QTextDocument>

FindDialog::FindDialog(QPlainTextEdit *editor, QWidget *parent)
    : QDialog(parent), editor(editor)
{
    setWindowTitle("Buscar");
    setMinimumWidth(400);

    QVBoxLayout *layoutPrincipal = new QVBoxLayout(this);

    // Campo de búsqueda
    QHBoxLayout *layoutBusqueda = new QHBoxLayout();
    layoutBusqueda->addWidget(new QLabel("Buscar:"));
    campoBusqueda = new QLineEdit();
    layoutBusqueda->addWidget(campoBusqueda);
    layoutPrincipal->addLayout(layoutBusqueda);

    // Opciones
    QHBoxLayout *layoutOpciones = new QHBoxLayout();
    chkCoincidirMayusculas = new QCheckBox("Coincidir mayúsculas/minúsculas");
    chkPalabraCompleta = new QCheckBox("Solo palabras completas");
    layoutOpciones->addWidget(chkCoincidirMayusculas);
    layoutOpciones->addWidget(chkPalabraCompleta);
    layoutPrincipal->addLayout(layoutOpciones);

    // Botones
    QHBoxLayout *layoutBotones = new QHBoxLayout();
    btnSiguiente = new QPushButton("Siguiente");
    btnAnterior = new QPushButton("Anterior");
    btnCerrar = new QPushButton("Cerrar");
    
    layoutBotones->addWidget(btnSiguiente);
    layoutBotones->addWidget(btnAnterior);
    layoutBotones->addStretch();
    layoutBotones->addWidget(btnCerrar);
    layoutPrincipal->addLayout(layoutBotones);

    // Conexiones
    connect(campoBusqueda, &QLineEdit::textChanged, this, &FindDialog::onTextChanged);
    connect(campoBusqueda, &QLineEdit::returnPressed, this, &FindDialog::onBuscarSiguiente);
    connect(btnSiguiente, &QPushButton::clicked, this, &FindDialog::onBuscarSiguiente);
    connect(btnAnterior, &QPushButton::clicked, this, &FindDialog::onBuscarAnterior);
    connect(btnCerrar, &QPushButton::clicked, this, &QDialog::accept);

    // Estado inicial
    btnSiguiente->setEnabled(false);
    btnAnterior->setEnabled(false);
}

void FindDialog::onTextChanged(const QString &text)
{
    bool habilitar = !text.isEmpty();
    btnSiguiente->setEnabled(habilitar);
    btnAnterior->setEnabled(habilitar);
}

void FindDialog::onBuscarSiguiente()
{
    buscar(true);
}

void FindDialog::onBuscarAnterior()
{
    buscar(false);
}

void FindDialog::buscar(bool haciaAdelante)
{
    QString texto = campoBusqueda->text();
    if (texto.isEmpty()) return;

    QTextDocument::FindFlags flags;
    
    if (!haciaAdelante) {
        flags |= QTextDocument::FindBackward;
    }
    
    if (chkCoincidirMayusculas->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    
    if (chkPalabraCompleta->isChecked()) {
        flags |= QTextDocument::FindWholeWords;
    }

    bool encontrado = editor->find(texto, flags);
    
    if (!encontrado) {
        // Si no encontró, preguntar si quiere volver al inicio/final
        QMessageBox::StandardButton respuesta = QMessageBox::question(
            this, "Buscar",
            "No se encontraron más coincidencias.\n¿Desea volver al " + 
            QString(haciaAdelante ? "inicio" : "final") + " del documento?",
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (respuesta == QMessageBox::Yes) {
            QTextCursor cursor = editor->textCursor();
            if (haciaAdelante) {
                cursor.movePosition(QTextCursor::Start);
            } else {
                cursor.movePosition(QTextCursor::End);
            }
            editor->setTextCursor(cursor);
            editor->find(texto, flags);
        }
    }
}
