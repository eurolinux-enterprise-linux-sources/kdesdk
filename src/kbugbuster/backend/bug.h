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
#ifndef BUG_H
#define BUG_H

#include "person.h"

#include <QtCore/QList>

#include <ksharedptr.h>

class BugImpl;

class Bug
{
public:
    typedef QList<Bug> List;
    typedef QList<int> BugMergeList;

    enum Severity { SeverityUndefined, Critical, Grave, Major, Crash, Normal,
                    Minor, Wishlist };
    enum Status { StatusUndefined, Unconfirmed, New, Assigned, Reopened,
                  Closed };

    Bug();
    Bug( BugImpl *impl );
    Bug( const Bug &other );
    Bug &operator=( const Bug &rhs );
    ~Bug();

    static QString severityLabel( Severity s );
    /**
     * Return string representation of severity. This function is symmetric to
     * stringToSeverity().
     */
    static QString severityToString( Severity s );
    /**
     * Return severity code of string representation. This function is symmetric
     * to severityToString().
     */
    static Severity stringToSeverity( const QString &, bool *ok = 0 );

    static QList<Severity> severities();

    uint age() const;
    void setAge( uint days );

    QString title() const;
    void setTitle( QString title );
    Person submitter() const;
    QString number() const;
    Severity severity() const;
    void setSeverity( Severity severity );
    QString severityAsString() const;
    Person developerTODO() const;

    BugMergeList mergedWith() const;

    /**
     * Status of a bug. Currently open or closed.
     * TODO: Should we add a status 'deleted' here ?
     */
    Status status() const;
    void setStatus( Status newStatus );

    static QString statusLabel( Status s );
    /**
     * Return string representation of status. This function is symmetric to
     * stringToStatus().
     */
    static QString statusToString( Status s );
    /**
     * Return status code of string representation. This function is symmetric
     * to statusToString().
     */
    static Status stringToStatus( const QString &, bool *ok = 0 );

    bool operator==( const Bug &rhs );
    bool operator<( const Bug &rhs ) const;

    bool isNull() const { return !m_impl; }

    static Bug fromNumber( const QString &bugNumber );

private:
    BugImpl *impl() { return m_impl.data(); }

    KSharedPtr<BugImpl> m_impl;
};

#endif

/* vim: set sw=4 ts=4 et softtabstop=4: */

