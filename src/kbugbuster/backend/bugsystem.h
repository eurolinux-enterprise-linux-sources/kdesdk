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
#ifndef BUGSYSTEM_H
#define BUGSYSTEM_H

#include "package.h"
#include "bug.h"
#include "bugdetails.h"
#include "bugcache.h"

#include <kurl.h>

#include <QObject>
#include <QList>
#include <QMap>
#include <QPair>

class KConfig;

class BugCommand;
class BugServer;
class BugServerConfig;
class BugJob;

class BugSystem : public QObject
{
    Q_OBJECT
    friend class BugJob;
    friend class BugSystemPrivate;
  private:
    BugSystem();
    virtual ~BugSystem();
  public:
    static BugSystem *self();

    BugCache *cache()const;
    BugServer *server() const { return mServer; }

    /**
      BugSystem takes ownership of the BugServerConfig objects.
    */
    void setServerList( const QList<BugServerConfig> &servers );
    QList<BugServer *> serverList();

    void setCurrentServer( const QString & );

    void retrievePackageList();
    void retrieveBugList( const Package &, const QString &component );
    void retrieveBugDetails( const Bug & );

    /**
     * Load the bugs the user reported himself, or for which he is the assigned to person
     */
    void retrieveMyBugsList();

    /**
      Queue a new command.
    */
    void queueCommand( BugCommand * );
    /**
      Forget all commands for a given bug.
    */
    void clearCommands( const QString &bug );
    /**
      Forget all commands for all bugs.
    */
    void clearCommands();
    /**
      Send all commands (generate the mails).
    */
    void sendCommands();

    void setDisconnected( bool );
    bool disconnected() const;

    Package::List packageList() const;

    Package package( const QString &pkgname ) const;
    Bug bug( const Package &pkg, const QString &component, const QString &number ) const;

    static void saveQuery( const KUrl &url );
    static void saveResponse( const QByteArray &d );
    static QString lastResponse();

    void readConfig( KConfig * );
    void writeConfig( KConfig * );

  signals:
    void packageListAvailable( const Package::List &pkgs );
    void bugListAvailable( const Package &pkg, const QString &component, const Bug::List & );
    void bugListAvailable( const QString &label, const Bug::List & );
    void bugDetailsAvailable( const Bug &, const BugDetails & );

    void packageListLoading();
    void bugListLoading( const Package &, const QString &component );
    void bugListLoading( const QString &label );
    void bugDetailsLoading( const Bug & );

    void packageListCacheMiss();
    void bugListCacheMiss( const Package &package );
    void bugListCacheMiss( const QString &label );
    void bugDetailsCacheMiss( const Bug & );

    void bugDetailsLoadingError();

    void infoMessage( const QString &message );
    void infoPercent( unsigned long percent );

    void commandQueued( BugCommand * );
    void commandCanceled( const QString & );

    void loadingError( const QString &text );

  protected:
    BugServer *findServer( const QString &name );

    void registerJob( BugJob * );

    void connectJob( BugJob * );

    void killAllJobs();

  protected slots:
    void unregisterJob( BugJob * );

  private slots:
    void setPackageList( const Package::List &pkgs );
    void setBugList( const Package &pkg, const QString &component, const Bug::List &bugs );
    void setBugDetails( const Bug &bug, const BugDetails &details );

  private:
    bool m_disconnected;

    BugServer *mServer;

    QList<BugServer *> mServerList;

    QList<BugJob *> mJobs;

    static QString mLastResponse;
};

#endif

/*
 * vim:sw=4:ts=4:et
 */
