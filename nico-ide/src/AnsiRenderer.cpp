/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         AnsiRenderer.cpp
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Implementación del renderizador ANSI. Procesa secuencias
               CSI (ESC[...), interpreta códigos SGR (colores 8+8),
               comandos J (limpiar pantalla) y H (cursor). Mantiene
               buffer para secuencias incompletas.
*/
#include "AnsiRenderer.h"
#include <QTextCursor>
#include <QTextBlock>

AnsiRenderer::AnsiRenderer(QPlainTextEdit *output, QObject *parent)
    : QObject(parent), output(output),
      colorTexto(QColor(200, 200, 200)),
      colorFondo(QColor(20, 20, 20)),
      negrita(false), cursiva(false), subrayado(false)
{
}

void AnsiRenderer::limpiarPantalla()
{
    output->clear();
    bufferIncompleto.clear();
}

void AnsiRenderer::renderizar(const QString &texto)
{
    procesarTexto(bufferIncompleto + texto);
    bufferIncompleto.clear();
}

void AnsiRenderer::procesarTexto(const QString &texto)
{
    int i = 0;
    int len = texto.length();
    QString textoActual;

    while (i < len) {
        QChar ch = texto[i];

        // Detectar inicio de secuencia ANSI: ESC (carácter 27, o \033)
        if (ch == QChar(27)) {
            // Primero, escribir cualquier texto acumulado antes de la secuencia
            if (!textoActual.isEmpty()) {
                QTextCursor cursor(output->document());
                cursor.movePosition(QTextCursor::End);
                cursor.insertText(textoActual, obtenerFormatoActual());
                textoActual.clear();
            }

            // Verificar que hay suficientes caracteres para una secuencia
            if (i + 1 >= len) {
                bufferIncompleto = QString(ch);
                return;
            }

            QChar siguiente = texto[i + 1];

            if (siguiente == QChar('[')) {
                // Secuencia CSI: ESC [ ... letra_final
                int j = i + 2;
                QString parametros;
                bool secuenciaCompleta = false;

                while (j < len) {
                    QChar c = texto[j];
                    if (c.isLetter() || c == QChar('~')) {
                        // Letra final de la secuencia
                        secuenciaCompleta = true;
                        break;
                    }
                    parametros += c;
                    j++;
                }

                if (!secuenciaCompleta) {
                    // Secuencia incompleta, guardar en buffer
                    bufferIncompleto = texto.mid(i);
                    return;
                }

                QChar letraFinal = texto[j];

                // Interpretar la secuencia
                if (letraFinal == QChar('m')) {
                    // SGR (Select Graphic Rendition) - colores y estilos
                    QStringList codigos = parametros.split(';', Qt::SkipEmptyParts);
                    
                    if (codigos.isEmpty() || (codigos.size() == 1 && codigos[0] == "0")) {
                        // Reset
                        colorTexto = QColor(200, 200, 200);
                        colorFondo = QColor(20, 20, 20);
                        negrita = false;
                        cursiva = false;
                        subrayado = false;
                    } else {
                        for (const QString &codigo : codigos) {
                            int code = codigo.toInt();
                            
                            if (code == 0) {
                                colorTexto = QColor(200, 200, 200);
                                colorFondo = QColor(20, 20, 20);
                                negrita = false;
                                cursiva = false;
                                subrayado = false;
                            } else if (code == 1) {
                                negrita = true;
                            } else if (code == 3) {
                                cursiva = true;
                            } else if (code == 4) {
                                subrayado = true;
                            } else if (code == 22) {
                                negrita = false;
                            } else if (code == 23) {
                                cursiva = false;
                            } else if (code == 24) {
                                subrayado = false;
                            } else if (code >= 30 && code <= 37) {
                                colorTexto = ansiColorAQt(code - 30, false);
                            } else if (code >= 40 && code <= 47) {
                                colorFondo = ansiColorAQt(code - 40, true);
                            } else if (code >= 90 && code <= 97) {
                                colorTexto = ansiColorAQt(code - 90 + 8, false);
                            } else if (code >= 100 && code <= 107) {
                                colorFondo = ansiColorAQt(code - 100 + 8, true);
                            }
                        }
                    }
                } else if (letraFinal == QChar('J')) {
                    // Limpiar pantalla
                    if (parametros == "2" || parametros.isEmpty()) {
                        output->clear();
                    }
                } else if (letraFinal == QChar('H') || letraFinal == QChar('f')) {
                    // Mover cursor (simplificado: solo limpiamos si va al inicio)
                    if (parametros == "1;1" || parametros == "H") {
                        // Cursor al inicio - no hacemos nada por ahora
                        // (implementación completa requeriría un buffer de pantalla)
                    }
                } else if (letraFinal == QChar('K')) {
                    // Limpiar línea (ignoramos por simplicidad)
                }
                // Ignorar secuencias de cursor oculto/mostrar (?25l, ?25h)

                i = j + 1;
            } else {
                // Otra secuencia ESC que no es CSI, ignorar
                i += 2;
            }
        } else {
            // Carácter normal, acumular
            textoActual += ch;
            i++;
        }
    }

    // Escribir el texto restante
    if (!textoActual.isEmpty()) {
        QTextCursor cursor(output->document());
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(textoActual, obtenerFormatoActual());
    }
}

QTextCharFormat AnsiRenderer::obtenerFormatoActual() const
{
    QTextCharFormat formato;
    formato.setForeground(colorTexto);
    formato.setBackground(colorFondo);
    
    if (negrita) formato.setFontWeight(QFont::Bold);
    if (cursiva) formato.setFontItalic(true);
    if (subrayado) formato.setFontUnderline(true);
    
    return formato;
}

QColor AnsiRenderer::ansiColorAQt(int indice, bool esBrillante) const
{
    // Tabla de colores ANSI estándar (8 básicos + 8 brillantes)
    static const QColor coloresNormales[] = {
        QColor(0, 0, 0),         // 0: Negro
        QColor(205, 49, 49),     // 1: Rojo
        QColor(13, 188, 121),    // 2: Verde
        QColor(229, 229, 16),    // 3: Amarillo
        QColor(36, 114, 200),    // 4: Azul
        QColor(188, 63, 188),    // 5: Magenta
        QColor(17, 168, 205),    // 6: Cian
        QColor(229, 229, 229)    // 7: Blanco
    };
    
    static const QColor coloresBrillantes[] = {
        QColor(102, 102, 102),   // 8: Gris brillante
        QColor(241, 76, 76),     // 9: Rojo brillante
        QColor(35, 209, 139),    // 10: Verde brillante
        QColor(245, 245, 67),    // 11: Amarillo brillante
        QColor(59, 142, 234),    // 12: Azul brillante
        QColor(214, 112, 214),   // 13: Magenta brillante
        QColor(41, 184, 219),    // 14: Cian brillante
        QColor(255, 255, 255)    // 15: Blanco brillante
    };

    if (indice >= 0 && indice < 8) {
        return coloresNormales[indice];
    } else if (indice >= 8 && indice < 16) {
        return coloresBrillantes[indice - 8];
    }
    
    return QColor(200, 200, 200); // Default
}
