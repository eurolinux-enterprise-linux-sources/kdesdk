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
#include "bugdetailsjob.h"

#include "bug.h"
#include "bugdetails.h"
#include "bugdetailsimpl.h"
#include "packageimpl.h"
#include "bugserver.h"
#include "processor.h"

#include <assert.h>
#include <kdebug.h>


BugDetailsJob::BugDetailsJob( BugServer *server )
  : BugJob( server )
{
}

BugDetailsJob::~BugDetailsJob()
{
}

void BugDetailsJob::start( const Bug &bug )
{
    m_bug = bug;

    KUrl bugUrl = server()->bugDetailsUrl( bug );

    kDebug() << "BugDetailsJob::start(): " << bugUrl.url();
    BugJob::start( bugUrl );
}

void BugDetailsJob::process( const QByteArray &data )
{
    BugDetails bugDetails;

    KBB::Error err = server()->processor()->parseBugDetails( data, bugDetails );

    if ( err ) {
        emit error( i18n("Bug %1: %2", m_bug.number() ,
                                        err.message() ) );
    } else {
        emit bugDetailsAvailable( m_bug, bugDetails );
    }
}

#include "bugdetailsjob.moc"

/*
 * vim:sw=4:ts=4:et
 */
