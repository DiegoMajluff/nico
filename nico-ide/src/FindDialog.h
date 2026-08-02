/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         FindDialog.h
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Declaración del diálogo de búsqueda. Permite buscar texto
               en el editor con opciones de coincidencia de mayúsculas
               y palabras completas, con navegación siguiente/anterior.
*/
#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QPlainTextEdit>

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QPlainTextEdit *editor, QWidget *parent = nullptr);

private slots:
    void onBuscarSiguiente();
    void onBuscarAnterior();
    void onTextChanged(const QString &text);

private:
    void buscar(bool haciaAdelante);

    QPlainTextEdit *editor;
    QLineEdit *campoBusqueda;
    QPushButton *btnSiguiente;
    QPushButton *btnAnterior;
    QPushButton *btnCerrar;
    QCheckBox *chkCoincidirMayusculas;
    QCheckBox *chkPalabraCompleta;
};

#endif // FINDDIALOG_H
