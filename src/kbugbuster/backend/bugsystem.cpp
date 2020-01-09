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
#include "bugsystem.h"

#include "packagelistjob.h"
#include "buglistjob.h"
#include "bugmybugsjob.h"
#include "bugdetailsjob.h"
#include "bugcommand.h"

#include <kdebug.h>
#include <klocale.h>
#include <kemailsettings.h>
#include <kstandarddirs.h>
#include <kconfig.h>

#include "packageimpl.h"
#include "bugimpl.h"
#include "bugdetailsimpl.h"
#include "mailsender.h"
#include "kbbprefs.h"
#include "bugserver.h"
#include "bugserverconfig.h"
#include "bugcache.h"

QString BugSystem::mLastResponse;

class BugSystemPrivate
{
public:
   BugSystem instance;
};

K_GLOBAL_STATIC(BugSystemPrivate, bugSystemPrivate)

BugSystem *BugSystem::self()
{
    return &bugSystemPrivate->instance;
}

BugSystem::BugSystem()
    : m_disconnected( false )
{
    mServer = 0;
}

BugSystem::~BugSystem()
{
    killAllJobs();
    qDeleteAll(mServerList);
}

BugCache *BugSystem::cache()const
{
  return mServer->cache();
}

void BugSystem::setDisconnected( bool disconnected )
{
    m_disconnected = disconnected;
}

bool BugSystem::disconnected() const
{
    return m_disconnected;
}

void BugSystem::retrievePackageList()
{
    mServer->setPackages( mServer->cache()->loadPackageList() );

    if( !mServer->packages().isEmpty() ) {
        emit packageListAvailable( mServer->packages() );
    } else {
        emit packageListCacheMiss();

        if ( !m_disconnected )
        {
            emit packageListLoading();

            PackageListJob *job = new PackageListJob( mServer );
            connect( job, SIGNAL( packageListAvailable( const Package::List & ) ),
                     this, SIGNAL( packageListAvailable( const Package::List & ) ) );
            connect( job, SIGNAL( packageListAvailable( const Package::List & ) ),
                     this, SLOT( setPackageList( const Package::List & ) ) );
            connect( job, SIGNAL( error( const QString & ) ),
                     this, SIGNAL( loadingError( const QString & ) ) );
            connectJob( job );

            registerJob( job );

            job->start();
        }
    }
}

void BugSystem::retrieveBugList( const Package &pkg, const QString &component )
{
    kDebug() << "BugSystem::retrieveBugList(): " << pkg.name();

    if ( pkg.isNull() )
        return;

    mServer->setBugs( pkg, component,
                      mServer->cache()->loadBugList( pkg, component,
                                                     m_disconnected ) );

    // Since the GUI stops showing the splash widget after this signal,
    // we should not emit anything on a cache miss...
    if( !mServer->bugs( pkg, component ).isEmpty() )
        emit bugListAvailable( pkg, component, mServer->bugs( pkg, component ) );
    else
    {
        emit bugListCacheMiss( pkg );

        if ( !m_disconnected )
        {
            kDebug() << "BugSystem::retrieveBugList() starting job";
            emit bugListLoading( pkg, component );

            BugListJob *job = new BugListJob( mServer );
            connect( job, SIGNAL( bugListAvailable( const Package &, const QString &, const Bug::List & ) ),
                     this, SIGNAL( bugListAvailable( const Package &, const QString &, const Bug::List & ) ) );
            connect( job, SIGNAL( bugListAvailable( const Package &, const QString &, const Bug::List & ) ),
                     this, SLOT( setBugList( const Package &, const QString &, const Bug::List & ) ) );
            connect( job, SIGNAL( error( const QString & ) ),
                     this, SIGNAL( loadingError( const QString & ) ) );
            connectJob( job );

            registerJob( job );

            job->start( pkg, component );
        }
    }
}

void BugSystem::retrieveMyBugsList()
{
    kDebug() ;

    if ( m_disconnected )
    {
        // This function is not cached for now
        emit bugListCacheMiss( i18n( "My Bugs" ) );
    }
    else
    {
        kDebug() << "Starting job";

        emit bugListLoading( i18n( "Retrieving My Bugs list..." ) );

        BugMyBugsJob *job = new BugMyBugsJob( mServer );

        connect( job, SIGNAL( bugListAvailable( const QString &, const Bug::List & ) ),
                 this, SIGNAL( bugListAvailable( const QString &, const Bug::List & ) ) );
        connect( job, SIGNAL( error( const QString & ) ),
                 this, SIGNAL( loadingError( const QString & ) ) );
        connectJob( job );

        registerJob( job );

        job->start();
    }
}

void BugSystem::retrieveBugDetails( const Bug &bug )
{
    if ( bug.isNull() )
        return;

    kDebug() << "BugSystem::retrieveBugDetails(): " << bug.number();

    mServer->setBugDetails( bug, mServer->cache()->loadBugDetails( bug ) );

    if ( !mServer->bugDetails( bug ).isNull() ) {
//        kDebug() << "Found in cache.";
        emit bugDetailsAvailable( bug, mServer->bugDetails( bug ) );
    } else {
//        kDebug() << "Not found in cache.";
        emit bugDetailsCacheMiss( bug );

        if ( !m_disconnected ) {
            emit bugDetailsLoading( bug );

            BugDetailsJob *job = new BugDetailsJob( mServer );
            connect( job, SIGNAL( bugDetailsAvailable( const Bug &, const BugDetails & ) ),
                     this, SIGNAL( bugDetailsAvailable( const Bug &, const BugDetails & ) ) );
            connect( job, SIGNAL( bugDetailsAvailable( const Bug &, const BugDetails & ) ),
                     this, SLOT( setBugDetails( const Bug &, const BugDetails & ) ) );
            connect( job, SIGNAL( error( const QString & ) ),
                     this, SIGNAL( bugDetailsLoadingError() ) );
            connectJob( job );

            registerJob( job );

            job->start( bug );
        }
    }
}

void BugSystem::connectJob( BugJob *job )
{
    connect( job, SIGNAL( infoMessage( const QString & ) ),
             this, SIGNAL( infoMessage( const QString & ) ) );
    connect( job, SIGNAL( infoPercent( unsigned long ) ),
             this, SIGNAL( infoPercent( unsigned long ) ) );
    connect( job, SIGNAL( jobEnded( BugJob * ) ),
             SLOT( unregisterJob( BugJob * ) ) );
}

void BugSystem::setPackageList( const Package::List &pkgs )
{
    mServer->setPackages( pkgs );

    mServer->cache()->savePackageList( pkgs );
}

void BugSystem::setBugList( const Package &pkg, const QString &component, const Bug::List &bugs )
{
    mServer->setBugs( pkg, component, bugs );
    mServer->cache()->saveBugList( pkg, component, bugs );
}

void BugSystem::setBugDetails( const Bug &bug, const BugDetails &details )
{
    mServer->setBugDetails( bug , details );

    mServer->cache()->saveBugDetails( bug, details );
}

Package::List BugSystem::packageList() const
{
    return mServer->packages();
}

Package BugSystem::package( const QString &pkgname ) const
{
    Package::List::ConstIterator it;
    Q_FOREACH( const Package &package, mServer->packages() ) {
        if( pkgname == package.name() ) return package;
    }
    return Package();
}

Bug BugSystem::bug( const Package &pkg, const QString &component, const QString &number ) const
{
    Bug::List bugs = mServer->bugs( pkg, component );

    Q_FOREACH( const Bug &bug, bugs ) {
        if( number == bug.number() ) return bug;
    }
    return Bug();
}

void BugSystem::queueCommand( BugCommand *cmd )
{
    if ( mServer->queueCommand( cmd ) ) emit commandQueued( cmd );
}

void BugSystem::clearCommands( const QString &bug )
{
    mServer->clearCommands( bug );

    emit commandCanceled( bug );
}

void BugSystem::clearCommands()
{
    Q_FOREACH( const QString &bug, mServer->bugsWithCommands() ) {
        clearCommands( bug );
    }
}

void BugSystem::sendCommands()
{
    QString recipient = KBBPrefs::instance()->mOverrideRecipient;
    bool sendBCC = KBBPrefs::instance()->mSendBCC;

    KEMailSettings emailSettings;
    QString senderName = emailSettings.getSetting( KEMailSettings::RealName );
    QString senderEmail = emailSettings.getSetting( KEMailSettings::EmailAddress );
    QString smtpServer = emailSettings.getSetting( KEMailSettings::OutServer );

    MailSender::MailClient client = (MailSender::MailClient)KBBPrefs::instance()->mMailClient;

    // ### connect to signals
    MailSender *mailer = new MailSender( client, smtpServer );
    connect( mailer, SIGNAL( status( const QString & ) ),
             SIGNAL( infoMessage( const QString & ) ) );

    mServer->sendCommands( mailer, senderName, senderEmail, sendBCC, recipient );
}

void BugSystem::setServerList( const QList<BugServerConfig> &servers )
{
    if ( servers.isEmpty() ) return;

    QString currentServer;
    if ( mServer ) currentServer = mServer->serverConfig().name();
    else currentServer = KBBPrefs::instance()->mCurrentServer;

    killAllJobs();

    QList<BugServer *>::ConstIterator serverIt;
    for( serverIt = mServerList.constBegin(); serverIt != mServerList.constEnd();
         ++serverIt ) {
        delete *serverIt;
    }
    mServerList.clear();

    QList<BugServerConfig>::ConstIterator cfgIt;
    for( cfgIt = servers.constBegin(); cfgIt != servers.constEnd(); ++cfgIt ) {
        mServerList.append( new BugServer( *cfgIt ) );
    }

    setCurrentServer( currentServer );
}

QList<BugServer *> BugSystem::serverList()
{
    return mServerList;
}

void BugSystem::setCurrentServer( const QString &name )
{
    killAllJobs();

    BugServer *server = findServer( name );
    if ( server ) {
        mServer = server;
    } else {
        kError() << "Server '" << name << "' not known." << endl;
        if ( mServerList.isEmpty() ) {
            kError() << "Fatal error: server list empty." << endl;
        } else {
            mServer = mServerList.first();
        }
    }

    if ( mServer ) {
      KBBPrefs::instance()->mCurrentServer = mServer->serverConfig().name();
    }
}

BugServer *BugSystem::findServer( const QString &name )
{
    Q_FOREACH( BugServer *bs, mServerList ) {
        if ( bs->serverConfig().name() == name ) return bs;
    }
    return 0;
}

void BugSystem::saveQuery( const KUrl &url )
{
  mLastResponse = "Query: " + url.url();
  mLastResponse += "\n\n";
}

void BugSystem::saveResponse( const QByteArray &response )
{
  mLastResponse += response;
}

QString BugSystem::lastResponse()
{
  return mLastResponse;
}

void BugSystem::readConfig( KConfig *config )
{
  KConfigGroup group = config->group("Servers");
  QStringList servers = group.readEntry( "Servers" , QStringList() );

  QList<BugServerConfig> serverList;

  if ( servers.isEmpty() ) {
    serverList.append( BugServerConfig() );
  } else {
    Q_FOREACH( const QString &srv, servers ) {
      BugServerConfig cfg;
      cfg.readConfig( config, srv );
      serverList.append( cfg );
    }
  }

  setServerList( serverList );
}

void BugSystem::writeConfig( KConfig *config )
{
  QStringList servers;
  Q_FOREACH( BugServer *bs, BugSystem::self()->serverList() ) {
    BugServerConfig serverConfig = bs->serverConfig();
    servers.append( serverConfig.name() );
    serverConfig.writeConfig( config );
  }

  KConfigGroup group = config->group("Servers");
  group.writeEntry( "Servers", servers );
}

void BugSystem::registerJob( BugJob *job )
{
  mJobs.append( job );
}

void BugSystem::unregisterJob( BugJob *job )
{
  mJobs.removeAll( job );
}

void BugSystem::killAllJobs()
{
  while( mJobs.count() ) {
    BugJob *job = mJobs.last();
    job->kill();
    unregisterJob( job );
  }
}

#include "bugsystem.moc"

/*
 * vim:sw=4:ts=4:et
 */
