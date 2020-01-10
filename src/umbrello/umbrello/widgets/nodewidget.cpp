/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "nodewidget.h"

// app includes
#include "debug_utils.h"
#include "node.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"

// qt includes
#include <QPainter>
#include <QPolygon>

DEBUG_REGISTER_DISABLED(NodeWidget)

NodeWidget::NodeWidget(UMLScene * scene, UMLNode *n )
  : UMLWidget(scene, WidgetBase::wt_Node, n)
{
    setSize(100, 30);
    setZValue(1);  // above box but below UMLWidget because may embed widgets
}

NodeWidget::~NodeWidget()
{
}

void NodeWidget::draw(QPainter & p, int offsetX, int offsetY)
{
    setPenFromSettings(p);
    if ( UMLWidget::useFillColor() ) {
        p.setBrush( UMLWidget::fillColor() );
    } else {
        p.setBrush( m_scene->activeView()->viewport()->palette().color(QPalette::Background) );
    }
    const int w = width();
    const int h = height();
    const int wDepth = (w/3 > DEPTH ? DEPTH : w/3);
    const int hDepth = (h/3 > DEPTH ? DEPTH : h/3);
    const int bodyOffsetY = offsetY + hDepth;
    const int bodyWidth = w - wDepth;
    const int bodyHeight = h - hDepth;
    QFont font = UMLWidget::font();
    font.setBold(true);
    const QFontMetrics &fm = getFontMetrics(FT_BOLD);
    const int fontHeight  = fm.lineSpacing();
    QString nameStr = name();

    QPolygon pointArray(5);
    pointArray.setPoint(0, offsetX, bodyOffsetY);
    pointArray.setPoint(1, offsetX + wDepth, offsetY);
    pointArray.setPoint(2, offsetX + w - 1, offsetY);
    pointArray.setPoint(3, offsetX + w - 1, offsetY + bodyHeight );
    pointArray.setPoint(4, offsetX + bodyWidth, offsetY + h - 1);
    p.drawPolygon(pointArray);
    p.drawRect(offsetX, bodyOffsetY, bodyWidth, bodyHeight);
    p.drawLine(offsetX + w - 1, offsetY, offsetX + bodyWidth - 2, bodyOffsetY + 1);

    p.setPen(textColor());
    p.setFont(font);

    int lines = 1;
    if (m_umlObject) {
        QString stereotype = m_umlObject->stereotype();
        if (!stereotype.isEmpty()) {
            p.drawText(offsetX, bodyOffsetY + (bodyHeight/2) - fontHeight,
                       bodyWidth, fontHeight, Qt::AlignCenter, m_umlObject->stereotype(true));
            lines = 2;
        }
    }

    if ( UMLWidget::isInstance() ) {
        font.setUnderline(true);
        p.setFont(font);
        nameStr = UMLWidget::instanceName() + " : " + nameStr;
    }

    if (lines == 1) {
        p.drawText(offsetX, bodyOffsetY + (bodyHeight/2) - (fontHeight/2),
                   bodyWidth, fontHeight, Qt::AlignCenter, nameStr);
    } else {
        p.drawText(offsetX, bodyOffsetY + (bodyHeight/2),
                   bodyWidth, fontHeight, Qt::AlignCenter, nameStr);
    }

    if(m_selected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

UMLSceneSize NodeWidget::minimumSize()
{
    if (m_umlObject == NULL) {
        DEBUG(DBG_SRC) << "m_umlObject is NULL";
        return UMLWidget::minimumSize();
    }

    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight  = fm.lineSpacing();

    QString name = m_umlObject->name();
    if ( UMLWidget::isInstance() ) {
        name = UMLWidget::instanceName() + " : " + name;
    }

    int width = fm.width(name);

    int tempWidth = 0;
    if (!m_umlObject->stereotype().isEmpty()) {
        tempWidth = fm.width(m_umlObject->stereotype(true));
    }
    if (tempWidth > width)
        width = tempWidth;
    width += DEPTH;

    int height = (2*fontHeight) + DEPTH;

    return UMLSceneSize(width, height);
}

void NodeWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement conceptElement = qDoc.createElement("nodewidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

