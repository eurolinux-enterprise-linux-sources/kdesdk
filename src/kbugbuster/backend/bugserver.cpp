/*
    This file is part of KBugBuster.
    Copyright (c) 2002,2003 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "bugserver.h"
#include "kbbprefs.h"
#include "rdfprocessor.h"
#include "bugcache.h"
#include "bugcommand.h"
#include "mailsender.h"
#include "bugserverconfig.h"
#include "htmlparser.h"

#include <kstandarddirs.h>
#include <kconfig.h>
#include <kdebug.h>

BugServer::BugServer()
{
  init();
}

BugServer::BugServer( const BugServerConfig &cfg )
  : mServerConfig( cfg )
{
  init();
}

void BugServer::init()
{
  mCache = new BugCache( identifier() );

  QString commandsFile = KStandardDirs::locateLocal( "appdata", identifier() + "commands" );
  mCommandsFile = new KConfig( commandsFile, KConfig::SimpleConfig);

  QString bugzilla = mServerConfig.bugzillaVersion();

  if ( bugzilla == "KDE" ) mProcessor = new DomProcessor( this );
  else if ( bugzilla == "2.10" ) mProcessor = new HtmlParser_2_10( this );
  else if ( bugzilla == "2.14.2" ) mProcessor = new HtmlParser_2_14_2( this );
  else if ( bugzilla == "2.17.1" ) mProcessor = new HtmlParser_2_17_1( this );
  else mProcessor = new HtmlParser( this );

  loadCommands();
}

BugServer::~BugServer()
{
  saveCommands();

  CommandsMap::ConstIterator it;
  for(it = mCommands.constBegin(); it != mCommands.constEnd(); ++it ) {
    QListIterator<BugCommand*> cmdIt( *it );
    while ( cmdIt.hasNext() )
        delete cmdIt.next();
  }
  delete mProcessor;
  delete mCommandsFile;
  delete mCache;
}

void BugServer::setServerConfig( const BugServerConfig &cfg )
{
  mServerConfig = cfg;
}

BugServerConfig &BugServer::serverConfig()
{
  return mServerConfig;
}

QString BugServer::identifier()
{
  QString id = mServerConfig.baseUrl().host();
  return id;
}

Processor *BugServer::processor() const
{
  return mProcessor;
}

KUrl BugServer::packageListUrl()
{
  KUrl url = mServerConfig.baseUrl();

  mProcessor->setPackageListQuery( url );

  return url;
}

KUrl BugServer::bugListUrl( const Package &product, const QString &component )
{
  KUrl url = mServerConfig.baseUrl();

  mProcessor->setBugListQuery( url, product, component );

  return url;
}

KUrl BugServer::bugDetailsUrl( const Bug &bug )
{
  KUrl url = mServerConfig.baseUrl();

  mProcessor->setBugDetailsQuery( url, bug );

  return url;
}

KUrl BugServer::bugLink( const Bug &bug )
{
  KUrl url = mServerConfig.baseUrl();

  url.setFileName( "show_bug.cgi" );
  url.setQuery( "id=" + bug.number() );

  kDebug() << "URL: " << url.url();

  return url;
}

KUrl BugServer::attachmentViewLink( const QString &id )
{
  KUrl url = mServerConfig.baseUrl();

  url.setFileName( "attachment.cgi" );
  url.setQuery( "id=" + id + "&action=view" );

  return url;
}

KUrl BugServer::attachmentEditLink( const QString &id )
{
  KUrl url = mServerConfig.baseUrl();

  url.setFileName( "attachment.cgi" );
  url.setQuery( "id=" + id + "&action=edit" );

  return url;
}

Bug::Status BugServer::bugStatus( const QString &str )
{
  if ( str == "UNCONFIRMED" ) {
    return Bug::Unconfirmed;
  } else if ( str == "NEW" ) {
    return Bug::New;
  } else if ( str == "ASSIGNED" ) {
    return Bug::Assigned;
  } else if ( str == "REOPENED" ) {
    return Bug::Reopened;
  } else if ( str == "RESOLVED" ) {
    return Bug::Closed;
  } else if ( str == "VERIFIED" ) {
    return Bug::Closed;
  } else if ( str == "CLOSED" ) {
    return Bug::Closed;
  } else {
    return Bug::StatusUndefined;
  }
}

Bug::Severity BugServer::bugSeverity( const QString &str )
{
  if ( str == "critical" ) {
    return Bug::Critical;
  } else if ( str == "grave" ) {
    return Bug::Grave;
  } else if ( str == "major" ) {
    return Bug::Major;
  } else if ( str == "crash" ) {
    return Bug::Crash;
  } else if ( str == "normal" ) {
    return Bug::Normal;
  } else if ( str == "minor" ) {
    return Bug::Minor;
  } else if ( str == "wishlist" ) {
    return Bug::Wishlist;
  } else {
    return Bug::SeverityUndefined;
  }
}

void BugServer::readConfig( KConfig * /*config*/ )
{
}

void BugServer::writeConfig( KConfig * /*config*/ )
{
}

bool BugServer::queueCommand( BugCommand *cmd )
{
    QListIterator<BugCommand*> cmdIt( mCommands[cmd->bug().number()] );
    while ( cmdIt.hasNext() )
        if ( cmdIt.next()->type() == cmd->type() )
            return false;
    mCommands[cmd->bug().number()].append( cmd );
    return true;
}

QList<BugCommand*> BugServer::queryCommands( const Bug &bug ) const
{
    CommandsMap::ConstIterator it = mCommands.find( bug.number() );
    if (it == mCommands.end()) return QList<BugCommand*>();
    else return *it;
}

bool BugServer::hasCommandsFor( const Bug &bug ) const
{
    return mCommands.contains( bug.number() );
}

void BugServer::sendCommands( MailSender *mailer, const QString &senderName,
                              const QString &senderEmail, bool sendBCC,
                              const QString &recipient )
{
    // Disable mail commands for non-KDE servers
    if ( mServerConfig.baseUrl() != KUrl( "http://bugs.kde.org" ) ) return;

    QString controlText;

    // For each bug that has commands.....
    CommandsMap::ConstIterator it;
    for(it = mCommands.constBegin(); it != mCommands.constEnd(); ++it ) {
        Bug bug;
        Package pkg;
        // And for each command....
        QListIterator<BugCommand*> cmdIt( *it );
        while ( cmdIt.hasNext() ) {
            BugCommand* cmd = cmdIt.next();
            bug = cmd->bug();
            if (!cmd->package().isNull())
                pkg = cmd->package();
            if (!cmd->controlString().isNull()) {
                kDebug() << "control@bugs.kde.org: " << cmd->controlString();
                controlText += cmd->controlString() + '\n';
            } else {
                kDebug() << cmd->mailAddress() << ": " << cmd->mailText();
                // ### hm, should probably re-use the existing mailer instance and
                // implement message queueing for smtp
                MailSender *directMailer = mailer->clone();
#if 0
                connect( directMailer, SIGNAL( status( const QString & ) ),
                         this, SIGNAL( infoMessage( const QString & ) ) );
#endif
                if (!directMailer->send( senderName, senderEmail, cmd->mailAddress(),
                                    cmd->bug().title().prepend( "Re: " ),
                                    cmd->mailText(), sendBCC, recipient )) {
                    delete mailer;
                    return;
                }
            }
        }
        if (!bug.isNull()) {
            mCommandsFile->deleteGroup( bug.number() ); // done, remove command
            mCache->invalidateBugDetails( bug );
            if ( !pkg.isNull() ) {
                mCache->invalidateBugList( pkg, QString() ); // the status of the bug comes from the buglist...

                QStringList::ConstIterator it2;
                for (it2 = pkg.components().constBegin();it2 != pkg.components().constEnd();++it2) {
                    mCache->invalidateBugList( pkg, (*it2) ); // the status of the bug comes from the buglist...
                }
            }
        }
    }

    if (!controlText.isEmpty()) {
        kDebug() << "control@bugs.kde.org doesn't work anymore";
#if 0        
        if ( !mailer->send( senderName, senderEmail, "control@bugs.kde.org",
                      i18n("Mail generated by KBugBuster"), controlText,
                      sendBCC, recipient ))
            return;
#endif
    } else {
        delete mailer;
    }

    for(it = mCommands.constBegin(); it != mCommands.constEnd(); ++it ) {
      QListIterator<BugCommand*> cmdIt( *it );
      while ( cmdIt.hasNext() )
          delete cmdIt.next();
    }
    mCommands.clear();
}

void BugServer::clearCommands( const QString &bug )
{
    mCommands.remove( bug );
    mCommandsFile->deleteGroup( bug );
}

bool BugServer::commandsPending() const
{
    return ( mCommands.count() > 0 );
}

QStringList BugServer::listCommands() const
{
    QStringList result;
    CommandsMap::ConstIterator it;
    for(it = mCommands.constBegin(); it != mCommands.constEnd(); ++it ) {
        QListIterator<BugCommand*> cmdIt( *it );
        while ( cmdIt.hasNext() ) {
            BugCommand* cmd = cmdIt.next();
            if (!cmd->controlString().isNull())
                result.append( i18n("Control command: %1", cmd->controlString()) );
            else
                result.append( i18n("Mail to %1", cmd->mailAddress()) );
        }
    }
    return result;
}

QStringList BugServer::bugsWithCommands() const
{
    QStringList bugs;

    CommandsMap::ConstIterator it;
    for(it = mCommands.constBegin(); it != mCommands.constEnd(); ++it ) {
        bugs.append( it.key() );
    }

    return bugs;
}

void BugServer::saveCommands() const
{
    CommandsMap::ConstIterator it;
    for(it = mCommands.constBegin(); it != mCommands.constEnd(); ++it ) {
        QListIterator<BugCommand*> cmdIt( *it );
        while ( cmdIt.hasNext() ) {
            KConfigGroup grp( mCommandsFile, it.key() );
            BugCommand* cmd = cmdIt.next();
            cmd->save( &grp );
        }
    }

    mCommandsFile->sync();
}

void BugServer::loadCommands()
{
    mCommands.clear();

    QStringList bugs = mCommandsFile->groupList();
    QStringList::ConstIterator it;
    for( it = bugs.constBegin(); it != bugs.constEnd(); ++it ) {
        QMap<QString, QString> entries = mCommandsFile->entryMap ( *it );
        QMap<QString, QString>::ConstIterator it;
        for( it = entries.constBegin(); it != entries.constEnd(); ++it ) {
            QString type = it.key();
            KConfigGroup grp( mCommandsFile, *it );
            BugCommand *cmd = BugCommand::load( &grp, type );
            if ( cmd ) {
                mCommands[cmd->bug().number()].append(cmd);
            }
        }
    }
}

void BugServer::setPackages( const Package::List &packages )
{
  mPackages = packages;
}

const Package::List &BugServer::packages() const
{
  return mPackages;
}

void BugServer::setBugs( const Package &pkg, const QString &component,
                         const Bug::List &bugs )
{
  QPair<Package, QString> pkg_key = QPair<Package, QString>(pkg, component);
  mBugs[ pkg_key ] = bugs;
}

const Bug::List &BugServer::bugs( const Package &pkg, const QString &component )
{
  QPair<Package, QString> pkg_key = QPair<Package, QString>(pkg, component);
  return mBugs[ pkg_key ];
}

void BugServer::setBugDetails( const Bug &bug, const BugDetails &details )
{
  mBugDetails[ bug ] = details;
}

const BugDetails &BugServer::bugDetails( const Bug &bug )
{
  return mBugDetails[ bug ];
}

/*
 * vim:sw=4:ts=4:et
 */
