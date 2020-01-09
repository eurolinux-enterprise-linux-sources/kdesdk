/***************************************************************************
   loadallbugsdlg.cpp  -  progress dialog while loading all bugs for a package
                             -------------------
    copyright            : (C) 2002 by David Faure
    email                : david@mandrakesoft.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation version 2.                               *
 *                                                                         *
 ***************************************************************************/

#include "loadallbugsdlg.h"
#include "bugsystem.h"
#include "bugcache.h"
#include <kdebug.h>
#include <kprogressdialog.h>
#include <qtimer.h>

LoadAllBugsDlg::LoadAllBugsDlg(  const Package& pkg, const QString &component )
    : KDialog( 0L )
{
    setModal( true );
    m_bugLoadingProgress = new KProgressDialog( this );
    connect( m_bugLoadingProgress, SIGNAL( canceled() ),
             this, SLOT( slotStopped() ) );
    setWindowTitle( i18n( "Loading All Bugs for Product %1", pkg.name() ) );
    connect( BugSystem::self(),
             SIGNAL( bugDetailsAvailable( const Bug &, const BugDetails & ) ),
             SLOT( slotBugDetailsAvailable( const Bug &, const BugDetails & ) ) );
    connect( BugSystem::self(),
             SIGNAL( bugDetailsLoadingError() ),
             SLOT( slotBugDetailsLoadingError() ) );
    // The package (and its buglist) has to be in the cache already...
    m_bugs = BugSystem::self()->cache()->loadBugList( pkg, component, true );
    m_count = m_bugs.count();
    m_bugLoadingProgress->progressBar()->setMaximum( m_count );
    kDebug() << "LoadAllBugsDlg: " << m_count << " bugs to load";
    m_processed = 0;
    QTimer::singleShot( 0, this, SLOT( loadNextBug() ) );
}

void LoadAllBugsDlg::slotBugDetailsAvailable( const Bug &bug, const BugDetails & )
{
    kDebug() << "LoadAllBugsDlg::slotBugDetailsAvailable " << bug.number();
    m_bugLoadingProgress->setLabelText( i18n( "Bug %1 loaded" , bug.number()) );
    loadNextBug();
}

void LoadAllBugsDlg::slotBugDetailsLoadingError()
{
    // Abort at the first error. Otherwise we get spammed with "no host bugs.kde.org" msg boxes....
    reject();
}

void LoadAllBugsDlg::loadNextBug()
{
    kDebug() << "LoadAllBugsDlg::loadNextBug";
    if ( m_bugs.isEmpty() )
    {
        kDebug() << "LoadAllBugsDlg::loadNextBug DONE!";
        accept();
    } else {
        BugCache* cache = BugSystem::self()->cache();
        Bug bug;
        do {
            bug = m_bugs.front();
            m_bugs.pop_front();
            m_processed++;
            m_bugLoadingProgress->progressBar()->setValue( m_processed );
            kDebug() << "looking at bug " << bug.number() << " in cache:" << cache->hasBugDetails( bug );
        } while ( cache->hasBugDetails( bug ) && !m_bugs.isEmpty() );
        if ( !cache->hasBugDetails( bug ) ) {
            kDebug() << "LoadAllBugsDlg::loadNextBug loading bug " << bug.number();
            BugSystem::self()->retrieveBugDetails( bug );
        } else {
            kDebug() << "LoadAllBugsDlg::loadNextBug DONE!";
            accept();
        }
    }
}

void LoadAllBugsDlg::slotStopped()
{
    kDebug() << "LoadAllBugsDlg::slotStopped";
    // TODO abort job?
    reject();
}

#include "loadallbugsdlg.moc"
