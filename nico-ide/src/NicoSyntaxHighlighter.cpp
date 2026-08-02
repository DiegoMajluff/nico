/*
Nico IDE v2.1.0 - Entorno de Desarrollo Integrado
@file:         NicoSyntaxHighlighter.cpp
@author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
@ai_assist:    Qwen (Alibaba Cloud) - Implementación
@license:      MIT / Personal Use (ver LICENSE)
@description:  Implementación del resaltador de sintaxis. Define patrones
               regex para palabras clave (azul), variables $ (amarillo),
               strings (naranja), números (verde) y comentarios // (verde
               itálica). Incluye comandos del motor lógico (Prolog).
*/
#include "NicoSyntaxHighlighter.h"

NicoSyntaxHighlighter::NicoSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Palabras clave (azul)
    keywordFormat.setForeground(QColor(86, 156, 214));
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bPROGRAMA\\b" << "\\bFINAL\\b" << "\\bBLOQUE\\b" << "\\bPRINCIPAL\\b"
                    << "\\bFIN\\b" << "\\bSI\\b" << "\\bSINO\\b" << "\\bSINOSI\\b" << "\\bENTONCES\\b"
                    << "\\bPARA\\b" << "\\bHACER\\b" << "\\bDESDE\\b" << "\\bHASTA\\b" << "\\bPASO\\b"
                    << "\\bMIENTRAS\\b" << "\\bREALIZAR\\b" << "\\bSEGUN\\b" << "\\bCASO\\b" << "\\bDEFECTO\\b"
                    << "\\bFUNCION\\b" << "\\bSUBPROGRAMA\\b" << "\\bLLAMAR\\b" << "\\bRETORNAR\\b"
                    << "\\bSALTAR\\b" << "\\bETIQUETA\\b" << "\\bCORTE\\b" << "\\bINCLUIR\\b"
                    << "\\bVARIABLE\\b" << "\\bCONSTANTE\\b" << "\\bLISTA\\b" << "\\bMATRIZ\\b"
                    << "\\bENTERA\\b" << "\\bDECIMAL\\b" << "\\bTEXTO\\b" << "\\bCARACTER\\b"
                    << "\\bARCHIVO\\b" << "\\bLOGICA\\b" << "\\bEXTENSO\\b" << "\\bSIGNO\\b" << "\\bSIN\\b"
                    << "\\bMAYOR\\b" << "\\bMENOR\\b" << "\\bIGUAL\\b" << "\\bDIFERENTE\\b"
                    << "\\bMAYORIGUAL\\b" << "\\bMENORIGUAL\\b" << "\\bY\\b" << "\\bO\\b" << "\\bNO\\b"
                    << "\\bMOD\\b" << "\\bVERDADERO\\b" << "\\bFALSO\\b"
                    << "\\bCALCULAR\\b" << "\\bASIGNAR\\b" << "\\bEN\\b" << "\\bRESULTADO\\b"
                    << "\\bESCRIBIR\\b" << "\\bMOSTRAR\\b" << "\\bLEER\\b" << "\\bLEERCARACTER\\b"
                    << "\\bLEERHASTA\\b" << "\\bLEERTECLA\\b"
                    << "\\bLIMPIARPANTALLA\\b" << "\\bCURSOR\\b" << "\\bPOSICIONAR\\b"
                    << "\\bOCULTARCURSOR\\b" << "\\bMOSTRARCURSOR\\b"
                    << "\\bANCHOTERMINAL\\b" << "\\bALTOTERMINAL\\b"
                    << "\\bESPERAR\\b" << "\\bTIEMPOMS\\b" << "\\bHORAACTUAL\\b" << "\\bFECHAACTUAL\\b"
                    << "\\bCOLORTEXTO\\b" << "\\bCOLORFONDO\\b" << "\\bTEXTONEGRITA\\b"
                    << "\\bTEXTOCURSIVA\\b" << "\\bTEXTOSUBRAYADO\\b"
                    << "\\bRESETTEXTO\\b" << "\\bRESETCOLOR\\b"
                    << "\\bABRIRARCHIVO\\b" << "\\bCERRARARCHIVO\\b" << "\\bESCRIBIRARCHIVO\\b"
                    << "\\bLEERARCHIVO\\b" << "\\bUSARARCHIVO\\b" << "\\bSISTEMA\\b"
                    << "\\bDIBUJARLINEA\\b" << "\\bDIBUJARCIRCULO\\b" << "\\bRELLENARRECTANGULO\\b"
                    << "\\bCOLISIONRECTANGULOS\\b"
                    << "\\bCONFIGURARPIN\\b" << "\\bESTADOPIN\\b" << "\\bLEERPIN\\b"
                    << "\\bGENERARPWM\\b" << "\\bDETENERPWM\\b"
                    << "\\bCONECTARBD\\b" << "\\bCERRARBD\\b" << "\\bEJECUTARBD\\b" << "\\bCONSULTARBD\\b"
                    << "\\bINICIARSERVER\\b" << "\\bDETENERSERVER\\b" << "\\bALERTA\\b" << "\\bINTENTAR\\b" << "\\bATRAPAR\\b" << "\\bSALTO\\b"
                    // Motor lógico (estilo Prolog)
                    << "\\bAGREGARHECHO\\b" << "\\bAGREGARREGLA\\b" << "\\bCONSULTAR\\b"
                    << "\\bLIMPIARLOGICA\\b" << "\\bLISTARHECHOS\\b" << "\\bLISTARREGLAS\\b"
                    << "\\bBUSCARTODOS\\b"
                    // Built-ins del motor lógico
                    << "\\bJUNTAR\\b" << "\\bREVERSA\\b"
                    << "\\bMIEMBRO\\b" << "\\bLONGITUD\\b" << "\\bPRIMER\\b" << "\\bULTIMO\\b";
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Variables (amarillo)
    variableFormat.setForeground(QColor(220, 220, 120));
    rule.pattern = QRegularExpression("\\$\\w+");
    rule.format = variableFormat;
    highlightingRules.append(rule);

    // Strings (naranja)
    stringFormat.setForeground(QColor(206, 145, 120));
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // Números (verde claro)
    numberFormat.setForeground(QColor(181, 206, 168));
    rule.pattern = QRegularExpression("(?<![.\\w])[0-9]+(?:\\.[0-9]+)?(?![.\\w])");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // Comentarios (verde)
    commentFormat.setForeground(QColor(106, 153, 85));
    commentFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);
}

void NicoSyntaxHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
