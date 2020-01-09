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
#ifndef KBB_BUGJOB_H
#define KBB_BUGJOB_H

#include <kio/jobclasses.h>

#include "bugserver.h"

class BugJob : public KIO::Job
{
    Q_OBJECT
  public:
    BugJob( BugServer * );
    virtual ~BugJob();

    BugServer *server() const { return mServer; }

  signals:
    void infoMessage( const QString &text );
    void infoPercent( unsigned long percent );
    void error( const QString &text );
    void jobEnded( BugJob * );

  protected:
    void start( const KUrl &url /*, const KParts::URLArgs &args = KParts::URLArgs()*/ );

    virtual void process( const QByteArray &data ) = 0;

  private slots:
    void ioResult( KJob *job );

    void ioData( KIO::Job *job, const QByteArray &data );

    void ioInfoMessage( KJob *job, const QString &text, const QString & );

    void ioInfoPercent( KJob *job, unsigned long percent );

  private:
    QByteArray m_data;
    BugServer *mServer;
};

#endif
/*
 * vim:sw=4:ts=4:et
 */
