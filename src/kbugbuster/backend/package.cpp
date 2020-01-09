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
#include "package.h"

#include "packageimpl.h"

Package::Package()
{
}

Package::Package( PackageImpl *impl )
    : m_impl( impl )
{
}

Package::Package( const Package &other )
{
    (*this) = other;
}

Package &Package::operator=( const Package &rhs )
{
    m_impl = rhs.m_impl;
    return *this;
}

Package::~Package()
{
}

QString Package::name() const
{
    if ( !m_impl )
        return QString();
    
    return m_impl->name;
}

QString Package::description() const
{
    if ( !m_impl )
        return QString();
    
    return m_impl->description;
}

uint Package::numberOfBugs() const
{
    if ( !m_impl )
        return 0;

    return m_impl->numberOfBugs;
}

Person Package::maintainer() const
{
    if ( !m_impl )
        return Person();

    return m_impl->maintainer;
}

const QStringList Package::components() const
{
    if ( !m_impl )
        return QStringList();
    
    return m_impl->components;
}

bool Package::operator==( const Package &rhs )
{
    return m_impl == rhs.m_impl;
}

bool Package::operator<( const Package &rhs ) const
{
    return m_impl < rhs.m_impl;
}

/**
 * vim:ts=4:sw=4:et
 */
