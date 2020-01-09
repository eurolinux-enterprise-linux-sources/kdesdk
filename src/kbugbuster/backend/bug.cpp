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
#include "bug.h"

#include "bugimpl.h"

#include <assert.h>
#include <kdebug.h>

Bug::Bug()
: m_impl( NULL )
{
}

Bug::Bug( BugImpl *impl )
: m_impl( impl )
{
}

Bug::Bug( const Bug &other )
{
    (*this) = other;
}

Bug Bug::fromNumber( const QString &bugNumber )
{
    return new BugImpl( QString::null, Person(), bugNumber, 0xFFFFFFFF, Normal, Person(),	//krazy:exclude=nullstrassign for old broken gcc
                        Unconfirmed, Bug::BugMergeList() );
}

Bug &Bug::operator=( const Bug &rhs )
{
    m_impl = rhs.m_impl;
    return *this;
}

Bug::~Bug()
{
}

QString Bug::severityLabel( Bug::Severity s )
{
    switch ( s )
    {
        case Critical: return i18n("Critical");
        case Grave: return i18n("Grave");
        case Major: return i18n("Major");
        case Crash: return i18n("Crash");
        case Normal: return i18n("Normal");
        case Minor: return i18n("Minor");
        case Wishlist: return i18n("Wishlist");
        default:
        case SeverityUndefined: return i18n("Undefined");
    }
}

QString Bug::severityToString( Bug::Severity s )
{
  switch ( s ) {
    case Critical: return QString::fromLatin1( "critical" );
    case Grave: return QString::fromLatin1( "grave" );
    case Major: return QString::fromLatin1( "major" );
    case Crash: return QString::fromLatin1( "crash" );
    case Normal: return QString::fromLatin1( "normal" );
    case Minor: return QString::fromLatin1( "minor" );
    case Wishlist: return QString::fromLatin1( "wishlist" );
    default:
      kWarning() << "Bug::severityToString invalid severity " << s;
      return QString::fromLatin1( "<invalid>" );
   }
}

Bug::Severity Bug::stringToSeverity( const QString &s, bool *ok )
{
   if ( ok )
      *ok = true;

   if ( s == "critical" ) return Critical;
   else if ( s == "grave" ) return Grave;
   else if ( s == "major" ) return Major;
   else if ( s == "crash" || s == "drkonqi" ) return Crash;
   else if ( s == "normal" ) return Normal;
   else if ( s == "minor" ) return Minor;
   else if ( s == "wishlist" ) return Wishlist;

   kWarning() << "Bug::stringToSeverity: invalid severity: " << s;
   if ( ok )
       *ok = false;
   return SeverityUndefined;
}

QList<Bug::Severity> Bug::severities()
{
    QList<Severity> s;
    s << Critical << Grave << Major << Crash << Normal << Minor << Wishlist;
    return s;
}

QString Bug::statusLabel( Bug::Status s )
{
    switch ( s )
    {
        case Unconfirmed: return i18n("Unconfirmed");
        case New: return i18n("New");
        case Assigned: return i18n("Assigned");
        case Reopened: return i18n("Reopened");
        case Closed: return i18n("Closed");
        default:
        case StatusUndefined: return i18n("Undefined");
    }
}

QString Bug::statusToString( Bug::Status s )
{
  switch ( s ) {
    case Unconfirmed: return QString::fromLatin1( "unconfirmed" );
    case New: return QString::fromLatin1( "new" );
    case Assigned: return QString::fromLatin1( "assigned" );
    case Reopened: return QString::fromLatin1( "reopened" );
    case Closed: return QString::fromLatin1( "closed" );
    default:
      kWarning() << "Bug::statusToString invalid status " << s;
      return QString::fromLatin1( "<invalid>" );
   }
}

Bug::Status Bug::stringToStatus( const QString &s, bool *ok )
{
   if ( ok )
      *ok = true;

   if ( s == "unconfirmed" ) return Unconfirmed;
   else if ( s == "new" ) return New;
   else if ( s == "assigned" ) return Assigned;
   else if ( s == "reopened" ) return Reopened;
   else if ( s == "closed" ) return Closed;

   kWarning() << "Bug::stringToStatus: invalid status: " << s;
   if ( ok )
       *ok = false;
   return StatusUndefined;
}

QString Bug::title() const
{
    if ( !m_impl )
        return QString();

    return m_impl->title;
}

void Bug::setTitle( QString title)
{
    if ( m_impl )
        m_impl->title = title;
}

uint Bug::age() const
{
    if ( !m_impl )
        return 0;

    return m_impl->age;
}

void Bug::setAge( uint age )
{
    if ( m_impl )
        m_impl->age = age;
}

struct Person Bug::submitter() const
{
    if ( !m_impl )
        return Person( QString::null, QString() );	//krazy:exclude=nullstrassign for old broken gcc

    return m_impl->submitter;
}

QString Bug::number() const
{
    if ( !m_impl )
        return QString();

    return m_impl->number;
}

Bug::Severity Bug::severity() const
{
    if ( !m_impl )
        return Normal;

    return m_impl->severity;
}

void Bug::setSeverity( Bug::Severity severity )
{
    if ( m_impl )
        m_impl->severity = severity;
}

Bug::BugMergeList Bug::mergedWith() const
{
    if ( !m_impl )
        return Bug::BugMergeList();

    return m_impl->mergedWith;
}


Bug::Status Bug::status() const
{
    if ( !m_impl )
        return StatusUndefined;

    return m_impl->status;
}

QString Bug::severityAsString() const
{
    return severityToString( severity() );
}

Person Bug::developerTODO() const
{
  return (!m_impl) ? Person( QString::null, QString() ) :	//krazy:exclude=nullstrassign for old broken gcc
                            m_impl->developerTODO;
}

bool Bug::operator==( const Bug &rhs )
{
    return m_impl == rhs.m_impl;
}

bool Bug::operator<( const Bug &rhs ) const
{
    return m_impl < rhs.m_impl;
}

/* vim: set ts=4 sw=4 et softtabstop=4: */

