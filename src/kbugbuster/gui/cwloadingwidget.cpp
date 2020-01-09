/*
    cwloadingwidget.cpp  -  Widget to be shown while loading data

    copyright   : (c) 2001 by Martijn Klingens
    email       : klingens@kde.org

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "cwloadingwidget.h"

#include <qpainter.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QFrame>
#include <QResizeEvent>

#include <kstandarddirs.h>
#include <klocale.h>
#include <kglobalsettings.h>

using namespace KBugBusterMainWindow;

CWLoadingWidget::CWLoadingWidget( WidgetMode mode, QWidget *parent)
: QFrame( parent )
{
    init( mode );
}

CWLoadingWidget::CWLoadingWidget( const QString &text, WidgetMode mode,
                                  QWidget *parent )
: QFrame( parent)
{
    init( mode );
    setText( text );
}

void CWLoadingWidget::init( WidgetMode mode )
{
    m_mode = mode;

    QPalette pal = palette();
    pal.setColor( QPalette::Active,   QPalette::Background,
                  QColor( 49, 121, 173 ) );
    pal.setColor( QPalette::Inactive, QPalette::Background,
                  QColor( 49, 121, 173 ) );
    pal.setColor( QPalette::Disabled, QPalette::Background,
                  QColor( 49, 121, 173 ) );
    setPalette( pal );

    setFrameShape( StyledPanel );
    setFrameShadow( Sunken );
    setLineWidth( 2 );

    setAttribute(Qt::WA_NoSystemBackground, true);

    // Load images and prepare pixmap effect
    if( m_mode == TopFrame )
    {
        m_logoPixmap =
                new QPixmap( KStandardDirs::locate( "data", "kbugbuster/pics/logo.png" ) );
        m_topRightPixmap =
                new QPixmap( KStandardDirs::locate( "data", "kbugbuster/pics/top-right.png" ) );
        m_barsPixmap =
                new QPixmap( KStandardDirs::locate( "data", "kbugbuster/pics/bars.png" ) );
        m_toolsPixmap = 0L;
        m_toolsPixmapEffect = 0L;
    }
    else
    {
        m_toolsPixmap =
                new QPixmap( KStandardDirs::locate( "data", "kbugbuster/pics/tools.png" ) );

        m_toolsPixmapEffect = new QPixmap( m_toolsPixmap->size() );

        QPainter pb;
        pb.begin( m_toolsPixmapEffect );
        pb.fillRect( 0, 0, m_toolsPixmap->width(), m_toolsPixmap->height(),
		     QBrush(  QColor(  49,  121,  172 ) ) );
        pb.drawPixmap( 0, 0, *m_toolsPixmap );
        // ### - This might be the right replacement for KPE::fade()?
        //pb.setCompositionMode(QPainter::CompositionMode_SourceOver);
        //pb.fillRect( 0, 0, m_toolsPixmap->width(), m_toolsPixmap->height(),
        //             QBrush(  QColor( 255, 255, 255, 64 ) ) );
        pb.end();

        // FIXME - kdefx is deprecated, use QPainter
        //KPixmapEffect::fade( *m_toolsPixmapEffect, 0.75, Qt::red );

        m_logoPixmap = 0L;
        m_topRightPixmap = 0L;
        m_barsPixmap = 0L;
    }

    // Create and fill the buffer
    m_buffer = new QPixmap;
}

void CWLoadingWidget::resizeEvent( QResizeEvent * )
{
  updatePixmap();
}

void CWLoadingWidget::setText( const QString &text )
{
    m_text = text;
    updatePixmap();
    repaint();
}

void CWLoadingWidget::updatePixmap()
{
    QRect cr = contentsRect();
    cr.setWidth( cr.width() + 2 );
    cr.setHeight( cr.height() + 2 );
    *m_buffer = QPixmap( cr.width(), cr.height() );

    QPainter p( m_buffer );

    // fill background
    p.fillRect( 0, 0, cr.width(), cr.height(),
                QBrush( QColor( 49, 121, 173 ) ) );

    if( m_mode == TopFrame )
    {
        QFont bigFont = QFont( KGlobalSettings::generalFont().family(),
                               28, QFont::Bold, true );

        int xoffset = m_logoPixmap->width();

        // Draw bars tiled
        int xpos = xoffset;
        if( width() > xpos )
            p.drawTiledPixmap( xpos, 0, cr.width() - xpos,
                               m_barsPixmap->height(), *m_barsPixmap );

        // Draw logo
        p.drawPixmap(width() - m_topRightPixmap->width(), 0, *m_topRightPixmap);
        p.drawPixmap( 0, 0, *m_logoPixmap );

        // Draw title text
        p.setPen( Qt::black );
        p.drawText( 150, 84, cr.width() - 150, 108 - 84,
                    Qt::AlignLeft | Qt::AlignVCenter, m_text );

        // Draw intro text
        QString desc = i18n( "Welcome to KBugBuster, a tool to manage the "
                             "KDE Bug Report System. With KBugBuster you can "
                             "manage outstanding bug reports for KDE from a "
                             "convenient front end." );
        p.setPen( Qt::black );
        p.drawText( 28, 128, cr.width() - 28, 184 - 128,
                    Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, desc );

        // Draw the caption text
        QString caption = i18n( "KBugBuster" );
        p.setFont( bigFont );
        p.setPen( QColor(139, 183, 222) );
        p.drawText( 220, 60, caption );
        p.setPen( Qt::black );
        p.drawText( 217, 57, caption );
    }
    else
    {
        // draw tools image
        if( cr.height() <= 24 )
            return;

        int toolsEffectY = cr.height() - m_toolsPixmap->height();
        int toolsEffectX = cr.width()  - m_toolsPixmap->width();
        if ( toolsEffectX < 0)
            toolsEffectX = 0;
        if ( height() < 24 + m_toolsPixmap->height() )
            toolsEffectY = 24;

        p.drawPixmap( toolsEffectX, toolsEffectY, *m_toolsPixmap );

        // draw textbox
        if( cr.height() <= 24 + 50 )
            return;

        int fheight = fontMetrics().height();

        int boxX = 25;
        int boxY = 24 + 50;
        int boxW = cr.width() - 2 * boxX - 2 * 10;
        if( boxW > 500 )
            boxW = 500;

        QRect br = fontMetrics().boundingRect( boxX, boxY,
                   boxW, cr.height() - boxY - 10 - 2 * fheight,
                   Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, m_text );

        QRect box = br;
        box.setHeight( box.height() + 2 * fheight );
        box.setWidth( box.width() + 2 * 10 );
        if( box.width() < cr.width() - 2 * boxX )
            box.setWidth( qMin( cr.width() - 2 * boxX, 500 + 2 * 10 ) );
        if( box.height() < 100 )
            box.setHeight( qMin( cr.height() - boxY - 2 * fheight - 10, 100 ) );

        p.setClipRect( box );
        p.fillRect( box, QBrush( QColor( 204, 222, 234 ) ) );
        p.drawPixmap( toolsEffectX, toolsEffectY, *m_toolsPixmapEffect );

        p.setViewport( box );
        p.setWindow( 0, 0, box.width(), box.height() );

        p.drawText( 10, fheight, br.width(),
                    qMax( br.height(), box.height() - 2 * fheight ),
                    Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, m_text );
    }
}

CWLoadingWidget::~CWLoadingWidget()
{
    delete m_toolsPixmap;
    delete m_logoPixmap;
    delete m_topRightPixmap;
    delete m_barsPixmap;
    delete m_toolsPixmapEffect;
    delete m_buffer;

    m_toolsPixmap = 0L;
    m_logoPixmap = 0L;
    m_topRightPixmap = 0L;
    m_barsPixmap = 0L;
    m_toolsPixmapEffect = 0L;
    m_buffer = 0L;
}

void CWLoadingWidget::mouseReleaseEvent( QMouseEvent * )
{
    emit clicked();
}

void CWLoadingWidget::drawContents( QPainter *p )
{
    if( !m_buffer || m_buffer->isNull() )
        p->fillRect( contentsRect(), QBrush( QColor( 255, 121, 172 ) ) );
    else
        p->drawPixmap( QPoint( contentsRect().x(), contentsRect().y()),
                       *m_buffer, contentsRect() );
}

#include "cwloadingwidget.moc"

/* vim: set et ts=4 sw=4 softtabstop=4: */
