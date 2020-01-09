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
#include "bugjob.h"

#include "kbbprefs.h"

#include <kio/job.h>

#include <klocale.h>
#include <kdebug.h>

BugJob::BugJob( BugServer *server )
    : Job( ), mServer( server )
{
}

BugJob::~BugJob()
{
}

void BugJob::start( const KUrl &url )
{
    kDebug() << "BugJob::start(): " << url.url();

    if ( KBBPrefs::instance()->mDebugMode ) {
        BugSystem::saveQuery( url );
    }

    // ### obey post, if necessary

    KIO::Job *job = KIO::get( url, KIO::Reload /*always reload, we have our own cache*/, KIO::HideProgressInfo );

    connect( job, SIGNAL( result( KJob * ) ),
             this, SLOT( ioResult( KJob * ) ) );
    connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
             this, SLOT( ioData( KIO::Job *, const QByteArray & ) ) );
    connect( job, SIGNAL( infoMessage( KJob *, const QString &, const QString & ) ),
             this, SLOT( ioInfoMessage( KJob *, const QString &,const QString & ) ) );
    connect( job, SIGNAL( percent( KJob *, unsigned long ) ),
             this, SLOT( ioInfoPercent( KJob *, unsigned long ) ) );
}

void BugJob::ioResult( KJob *job )
{
    if ( job->error() )
    {
        emit error( job->errorText() );
        BugSystem::self()->unregisterJob(this);
        this->kill();
        return;
    }

    infoMessage( i18n( "Parsing..." ) );

#if 0
    kDebug() << "--START:" << m_data << ":END--";
#endif

    if ( KBBPrefs::instance()->mDebugMode ) {
        BugSystem::saveResponse( m_data );
    }

    process( m_data );
    infoMessage( i18n( "Ready." ) );

    emit jobEnded( this );

    delete this;
}

void BugJob::ioData( KIO::Job *, const QByteArray &data )
{
    unsigned int start = m_data.size();

    m_data.resize( m_data.size() + data.size() );
    memcpy( m_data.data() + start, data.data(), data.size() );
}

void BugJob::ioInfoMessage( KJob *, const QString &_text, const QString & )
{
    QString text = _text;
    emit infoMessage( text );
}

void BugJob::ioInfoPercent( KJob *, unsigned long percent )
{
    emit infoPercent( percent );
}

#include "bugjob.moc"

/*
 * vim:sw=4:ts=4:et
 */
