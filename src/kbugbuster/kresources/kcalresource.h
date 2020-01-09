/*
    This file is part of KBugBuster.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KCALRESOURCE_H
#define KCALRESOURCE_H

#include <qstring.h>
#include <qdatetime.h>

#include <kurl.h>
#include <kconfig.h>
#include <kdirwatch.h>

#include <kcal/incidence.h>
#include <kcal/calendarlocal.h>
#include <kcal/icalformat.h>
#include <kcal/resourcecached.h>

#include <bugsystem.h>

namespace KIO {
class FileCopyJob;
class Job;
}

class KJob;

namespace KBB {
class ResourcePrefs;
}

/**
  This class provides a calendar stored as a remote file.
*/
class KCalResource : public KCal::ResourceCached
{
    Q_OBJECT

    friend class KCalResourceConfig;

  public:
    /**
      Reload policy.
      
      @see setReloadPolicy(), reloadPolicy()
    */
    enum { ReloadNever, ReloadOnStartup, ReloadOnceADay, ReloadAlways };
  
    /**
      Create resource from configuration information stored in KConfig object.
    */
    KCalResource();
    KCalResource( const KConfigGroup & );
    ~KCalResource();

    void readConfig( const KConfigGroup &config );
    void writeConfig( KConfigGroup &config );

    KBB::ResourcePrefs *prefs();

    /**
      Return name of file used as cache for remote file.
    */
    QString cacheFile() const;

    KABC::Lock *lock();

    bool isSaving();

    void dump() const;
    virtual bool doLoad( bool syncCache ){return false;}
	virtual bool doSave( bool syncCache ){return false;}

  protected slots:
    void slotBugListAvailable( const Package &, const QString &,
                               const Bug::List &bugs );

    void slotLoadJobResult( KIO::Job * );
    void slotSaveJobResult( KJob * );

  protected:
    bool doOpen();
    void doClose();
    bool doLoad();
    bool doSave();
 
  private:
    void init();

    KBB::ResourcePrefs *mPrefs;

    KUrl mDownloadUrl;
    KUrl mUploadUrl;

    int mReloadPolicy;

    KCal::ICalFormat mFormat;

    bool mOpen;

    KIO::FileCopyJob *mDownloadJob;
    KIO::FileCopyJob *mUploadJob;
    
    KABC::Lock *mLock;

    class Private;
    Private *d;
};

#endif
