/* This file is part of kdesdk / KBugBuster.

   Copyright 2008  KBugBuster Authors <kde-ev-board@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy 
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef PACKAGE_H
#define PACKAGE_H

#include "person.h"


#include <ksharedptr.h>

class PackageImpl;

class Package
{
public:
    typedef QList<Package> List;

    Package();
    Package( PackageImpl *impl );
    Package( const Package &other );
    Package &operator=( const Package &rhs );
    ~Package();

    QString name() const;
    QString description() const;
    uint numberOfBugs() const;
    Person maintainer() const;
    const QStringList components() const;

    bool isNull() const { return !m_impl ; }

    PackageImpl *impl() { return m_impl.data(); }

    bool operator==( const Package &rhs );
    bool operator<( const Package &rhs ) const;

private:
    KSharedPtr<PackageImpl> m_impl;
};

#endif

/* vim: set sw=4 ts=4 et softtabstop=4: */

