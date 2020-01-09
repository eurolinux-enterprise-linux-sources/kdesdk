/*
    packagelvi.h  -  Custom QListViewItem that holds a Package object
    
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

#ifndef PACKAGELVI_H
#define PACKAGELVI_H

#include <q3listview.h>

#include "package.h"

/**
 * @author Martijn Klingens
 */
class PackageLVI : public Q3ListViewItem
{
public:
    // Top-level package
    PackageLVI( Q3ListView *parent , const Package &pkg, const QString &component );
    // Child component
    PackageLVI( Q3ListViewItem *parent , const Package &pkg, const QString &component );

    ~PackageLVI();

    Package& package() { return m_package; }
    void setPackage( const Package &pkg ) { m_package = pkg; }

    QString component() const { return m_component; }
    void setComponent( const QString &component ) { m_component = component; }
    
private:
    Package m_package;
    QString m_component;
};
 
#endif // PACKAGELVI_H

/* vim: set et ts=4 softtabstop=4 sw=4: */

