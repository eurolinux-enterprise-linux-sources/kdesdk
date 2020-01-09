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
#include "kcalresource.h"

#include <typeinfo>
#include <stdlib.h>

#include <QDateTime>
#include <QString>

#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <kcal/vcaldrag.h>
#include <kcal/vcalformat.h>
#include <kcal/icalformat.h>
#include <kcal/exceptions.h>
#include <kcal/incidence.h>
#include <kcal/event.h>
#include <kcal/todo.h>
#include <kcal/journal.h>
#include <kcal/filestorage.h>

#include <kabc/locknull.h>

#include <kresources/configwidget.h>

#include "bugsystem.h"
#include "bugserver.h"

#include "kcalresourceconfig.h"
#include "resourceprefs.h"

KCalResource::KCalResource()
 : ResourceCached(), mLock( 0 )
{
  mPrefs = new KBB::ResourcePrefs;

  KConfigSkeletonItem::List items = mPrefs->items();
  KConfigSkeletonItem::List::Iterator it;
  for( it = items.begin(); it != items.end(); ++it ) {
    (*it)->setGroup( identifier() );
  } 
  init();
}

KCalResource::KCalResource( const KConfigGroup& config )
  : ResourceCached( config ), mLock( 0 )
{
  mPrefs = new KBB::ResourcePrefs;

  KConfigSkeletonItem::List items = mPrefs->items();
  KConfigSkeletonItem::List::Iterator it;
  for( it = items.begin(); it != items.end(); ++it ) {
    (*it)->setGroup( identifier() );
  }

  readConfig( config );

  init();
}

KCalResource::~KCalResource()
{
  close();

  if ( mDownloadJob ) mDownloadJob->kill();
  if ( mUploadJob ) mUploadJob->kill();

  delete mLock;
}

void KCalResource::init()
{
  mDownloadJob = 0;
  mUploadJob = 0;

  setType( "remote" );

  mOpen = false;

  mLock = new KABC::LockNull( true );

  KConfig config( "kbugbusterrc" );

  BugSystem::self()->readConfig( &config );
}

KBB::ResourcePrefs *KCalResource::prefs()
{
  return mPrefs;
}

void KCalResource::readConfig( const KConfigGroup & )
{
  mPrefs->readConfig();
}

void KCalResource::writeConfig( KConfigGroup &config )
{
  kDebug() << "KCalResource::writeConfig()";

  ResourceCalendar::writeConfig( config );

  mPrefs->writeConfig();
}

QString KCalResource::cacheFile() const
{
  QString file = KStandardDirs::locateLocal( "cache", "kcal/kresources/" + identifier() );
  kDebug() << "KCalResource::cacheFile(): " << file;
  return file;
}

bool KCalResource::doOpen()
{
  kDebug(5800) << "KCalResource::doOpen()";

  mOpen = true;

  return true;
}

bool KCalResource::doLoad()
{
  kDebug() << "KCalResource::doLoad()";

  if ( !mOpen ) return true;

  if ( mDownloadJob ) {
    kWarning() << "KCalResource::doLoad(): download still in progress."
                << endl;
    return false;
  }
  if ( mUploadJob ) {
    kWarning() << "KCalResource::doLoad(): upload still in progress."
                << endl;
    return false;
  }

  calendar()->close();

  calendar()->load( cacheFile() );

  BugSystem *kbb = BugSystem::self();

  kDebug() << "KNOWN SERVERS:";
  QList<BugServer *> servers = kbb->serverList();
  QList<BugServer *>::ConstIterator it;
  for( it = servers.constBegin(); it != servers.constEnd(); ++it ) {
    kDebug() << "  " << (*it)->identifier();
  }

  kbb->setCurrentServer( mPrefs->server() );
  if ( !kbb->server() ) {
    kError() << "Server not found." << endl;
    return false;
  } else {
    kDebug() << "CURRENT SERVER: " << kbb->server()->identifier();
  }

  kbb->retrievePackageList();

  Package package = kbb->package( mPrefs->product() );

  connect( kbb, SIGNAL( bugListAvailable( const Package &, const QString &,
                                          const Bug::List & ) ),
           SLOT( slotBugListAvailable( const Package &, const QString &,
                                       const Bug::List & ) ) );

  kbb->retrieveBugList( package, mPrefs->component() );

  return true;
}

void KCalResource::slotBugListAvailable( const Package &, const QString &,
                                         const Bug::List &bugs )
{
  kDebug() << "KCalResource::slotBugListAvailable()";

  if ( bugs.isEmpty() ) return;

  QString masterUid = "kbb_" + BugSystem::self()->server()->identifier();
  KCal::Todo *masterTodo = calendar()->todo( masterUid );
  if ( !masterTodo ) {
    masterTodo = new KCal::Todo;
    masterTodo->setUid( masterUid );
    masterTodo->setSummary( resourceName() );
    calendar()->addTodo( masterTodo );
  }

  Bug::List::ConstIterator it;
  for( it = bugs.begin(); it != bugs.end(); ++it ) {
    Bug bug = *it;
    kDebug() << "  Bug " << bug.number() << ": " << bug.title();
    QString uid = "KBugBuster_" + bug.number();
    KCal::Todo *newTodo = 0;
    KCal::Todo *todo = calendar()->todo( uid );
    if ( !todo ) {
      newTodo = new KCal::Todo;
      newTodo->setUid( uid );
      QString uri = "http://bugs.kde.org/show_bug.cgi?id=%1";
      newTodo->addAttachment( new KCal::Attachment( uri.arg( bug.number() ) ) );
      todo = newTodo;
    }

    todo->setSummary( bug.number() + ": " + bug.title() );
    todo->setRelatedTo( masterTodo );

    if ( newTodo ) calendar()->addTodo( newTodo );
  }

  emit resourceChanged( this );
}

void KCalResource::slotLoadJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    job->ui()->showErrorMessage();
  } else {
    kDebug() << "KCalResource::slotLoadJobResult() success";

    calendar()->close();
    calendar()->load( cacheFile() );

    emit resourceChanged( this );
  }

  mDownloadJob = 0;

  emit resourceLoaded( this );
}

bool KCalResource::doSave()
{
  kDebug() << "KCalResource::doSave()";

  if ( !mOpen ) return true;

  if ( readOnly() ) {
    emit resourceSaved( this );
    return true;
  }

  if ( mDownloadJob ) {
    kWarning() << "KCalResource::save(): download still in progress."
                << endl;
    return false;
  }
  if ( mUploadJob ) {
    kWarning() << "KCalResource::save(): upload still in progress."
                << endl;
    return false;
  }

  calendar()->save( cacheFile() );

  mUploadJob = KIO::file_copy( KUrl( cacheFile() ), mUploadUrl, -1, KIO::Overwrite );
  connect( mUploadJob, SIGNAL( result( KJob * ) ),
           SLOT( slotSaveJobResult( KJob * ) ) );

  return true;
}

bool KCalResource::isSaving()
{
  return mUploadJob;
}

void KCalResource::slotSaveJobResult( KJob *job )
{
  if ( job->error() ) {
    static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
  } else {
    kDebug() << "KCalResource::slotSaveJobResult() success";
  }

  mUploadJob = 0;

  emit resourceSaved( this );
}

void KCalResource::doClose()
{
  if ( !mOpen ) return;

  calendar()->close();
  mOpen = false;
}

KABC::Lock *KCalResource::lock()
{
  return mLock;
}

void KCalResource::dump() const
{
  ResourceCalendar::dump();
  kDebug(5800) << "  DownloadUrl: " << mDownloadUrl.url();
  kDebug(5800) << "  UploadUrl: " << mUploadUrl.url();
  kDebug(5800) << "  ReloadPolicy: " << mReloadPolicy;
}

#include "kcalresource.moc"
