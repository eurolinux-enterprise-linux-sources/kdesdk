/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMBRELLO_VERSION_H
#define UMBRELLO_VERSION_H

#include <kdeversion.h>

inline QByteArray umbrelloVersion()
{
#ifdef UMBRELLO_VERSION_STRING
    QString versionStr = QString(UMBRELLO_VERSION_STRING);
#else
    QString versionStr = QString("%1.%2.%3")
        .arg(KDE::versionMajor()-2)
        .arg(KDE::versionMinor())
        .arg(KDE::versionRelease());
#endif
    return versionStr.toLatin1();
}

#endif
