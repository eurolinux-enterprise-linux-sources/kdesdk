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
#ifndef BUGIMPL_H
#define BUGIMPL_H

#include "person.h"
#include "bug.h"

#include <kurl.h>
#include <ksharedptr.h>

class BugImpl : public KShared
{
public:
    BugImpl( const QString &_title, const Person &_submitter, const QString &_number,
        uint _age, Bug::Severity _severity, const Person &_developerTODO,
        Bug::Status _status, const Bug::BugMergeList& _mergedWith  )
    : age( _age ), title( _title ), submitter( _submitter ), number( _number ),
        severity( _severity ), developerTODO( _developerTODO ),
        status( _status ), mergedWith( _mergedWith )
    {
    }

    uint age;
    QString title;
    Person submitter;
    QString number;
    Bug::Severity severity;
    Person developerTODO;
    Bug::Status status;

    Bug::BugMergeList mergedWith;
};

#endif

// vim: set sw=4 ts=4 sts=4 et:

