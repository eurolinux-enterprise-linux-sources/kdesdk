// (C) 2001, 
/* This file is part of kdesdk / KBugBuster.

   Copyright 2001  Cornelius Schumacher <schumacher@kde.org>

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
#include "bugcache.h"

#include <QStringList>
#include <QFile>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kdebug.h>

#include "packageimpl.h"
#include "bugimpl.h"
#include "bugdetailsimpl.h"


BugCache::BugCache( const QString &id )
{
    mId = id;

    init();
}

BugCache::~BugCache()
{
    m_cachePackages->sync();
    m_cacheBugs->sync();

    delete m_cachePackages;
    delete m_cacheBugs;
}

void BugCache::init()
{
    mCachePackagesFileName = KStandardDirs::locateLocal( "appdata", mId + "-packages.cache" );
    mCacheBugsFileName = KStandardDirs::locateLocal( "appdata", mId + "-bugs.cache" );

    m_cachePackages = new KConfig( mCachePackagesFileName, KConfig::SimpleConfig);
    m_cacheBugs = new KConfig( mCacheBugsFileName, KConfig::SimpleConfig);
}

void BugCache::savePackageList( const Package::List &pkgs )
{
    Package::List::ConstIterator it;
    for (it = pkgs.constBegin(); it != pkgs.constEnd(); ++it) {
        KConfigGroup grp(m_cachePackages, (*it).name());
        grp.writeEntry("description",(*it).description());
        grp.writeEntry("numberOfBugs",(*it).numberOfBugs());
        grp.writeEntry("components",(*it).components());
        writePerson(&grp, "Maintainer",(*it).maintainer());
    }
}

Package::List BugCache::loadPackageList()
{
    Package::List pkgs;

    QStringList packages = m_cachePackages->groupList();
    QStringList::ConstIterator it;
    for( it = packages.constBegin(); it != packages.constEnd(); ++it ) {
        if ((*it) == "<default>") continue;
        if ((*it).contains("/")) continue;

        KConfigGroup grp(m_cachePackages, *it);

        QString description = grp.readEntry("description");
        int numberOfBugs = grp.readEntry("numberOfBugs",0);
        Person maintainer = readPerson(&grp, "Maintainer");
        QStringList components = grp.readEntry("components", QStringList() );
 
        pkgs.append( Package( new PackageImpl( (*it), description, numberOfBugs,
                                                maintainer, components ) ) );
    }

    return pkgs;
}

void BugCache::invalidatePackageList()
{
    // Completely wipe out packages.cache
    QStringList packages = m_cachePackages->groupList();
    QStringList::ConstIterator it;
    for( it = packages.constBegin(); it != packages.constEnd(); ++it ) {
        if ((*it) == "<default>") continue;
        m_cachePackages->deleteGroup(*it);
    }
}

void BugCache::saveBugList( const Package &pkg, const QString &component, const Bug::List &bugs )
{
    QStringList bugList;

    Bug::List::ConstIterator it;
    for( it = bugs.constBegin(); it != bugs.constEnd(); ++it ) {
        QString number = (*it).number();
        bugList.append( number );
        KConfigGroup grp( m_cacheBugs, number );
        grp.writeEntry( "Title", (*it).title() );
        grp.writeEntry( "Severity", Bug::severityToString((*it).severity()) );
        grp.writeEntry( "Status", Bug::statusToString((*it).status()) );
        grp.writeEntry( "MergedWith" , (*it).mergedWith() );
        grp.writeEntry( "Age", ( *it ).age() );
        writePerson( &grp, "Submitter", (*it).submitter() );
        writePerson( &grp, "TODO", (*it).developerTODO() );
    }

    KConfigGroup grp(m_cachePackages, component.isEmpty() ? pkg.name() : (pkg.name() + '/' + component));
    grp.writeEntry( "bugList", bugList );
}

Bug::List BugCache::loadBugList( const Package &pkg, const QString &component, bool disconnected )
{
//    kDebug() << "Loading bug list for " << pkg.name();

    Bug::List bugList;

    KConfigGroup grp(m_cachePackages, component.isEmpty() ? pkg.name() : (pkg.name() + '/' + component));
    QStringList bugs = grp.readEntry( "bugList" , QStringList() );

//    kDebug() << "  Bugs: " << (bugs.join(","));

    QStringList::ConstIterator it;
    for( it = bugs.constBegin(); it != bugs.constEnd(); ++it ) {
        if ( m_cacheBugs->hasGroup(*it) )
        {
	    KConfigGroup grp(m_cacheBugs, *it);
            QString title = grp.readEntry("Title");
            if ( !title.isEmpty() ) // dunno how I ended up with an all empty bug in the cache
            {
                Person submitter = readPerson( &grp, "Submitter" );
                Bug::Status status = Bug::stringToStatus( grp.readEntry("Status") );
                Bug::Severity severity = Bug::stringToSeverity( grp.readEntry("Severity") );
                Person developerTODO = readPerson( &grp, "TODO" );
                Bug::BugMergeList mergedWith = grp.readEntry( "MergedWith",QList<int>() );
                uint age = grp.readEntry( "Age", 0xFFFFFFFF );
                bugList.append( Bug( new BugImpl( title, submitter, ( *it ), age,
                                                  severity, developerTODO,
                                                  status, mergedWith ) ) );
            }
        } else {
            // This bug is in the package cache's buglist but not in the bugs cache
            // Probably a new bug, we need to fetch it - if we're not in disconnected mode
            kWarning() << "Bug " << *it << " not in bug cache";
            if ( !disconnected )
                return Bug::List(); // returning an empty list will trigger a reload of the buglist
        }
    }

    return bugList;
}

void BugCache::invalidateBugList( const Package& pkg, const QString &component )
{
    kDebug() << "BugCache::invalidateBugList " << pkg.name()
              << " (" << component << ")" << endl;

    // Erase bug list for this package
    KConfigGroup grp(m_cachePackages, component.isEmpty() ? pkg.name() : (pkg.name() + '/' + component));
    grp.writeEntry("bugList","");
}

void BugCache::saveBugDetails( const Bug &bug, const BugDetails &details )
{
    KConfigGroup grp  = m_cacheBugs->group(bug.number());

    grp.writeEntry( "Version", details.version() );
    grp.writeEntry( "Source", details.source() );
    grp.writeEntry( "Compiler", details.compiler() );
    grp.writeEntry( "OS", details.os() );

    QStringList senders;
    QStringList texts;
    QStringList dates;

    BugDetailsPart::List parts = details.parts();
    BugDetailsPart::List::ConstIterator it;
    for ( it = parts.constBegin(); it != parts.constEnd(); ++it ) {
        senders.append( (*it).sender.fullName() );
        texts.append( (*it).text );
        dates.append( (*it).date.toString( Qt::ISODate ) );
    }

    grp.writeEntry( "Details", texts );
    grp.writeEntry( "Senders", senders );
    grp.writeEntry( "Dates", dates );
}

bool BugCache::hasBugDetails( const Bug& bug ) const
{
    if ( !m_cacheBugs->hasGroup( bug.number() ) )
        return false;

    KConfigGroup grp = m_cacheBugs->group(bug.number());
    return grp.hasKey( "Details" );
}

BugDetails BugCache::loadBugDetails( const Bug &bug )
{
    if ( !m_cacheBugs->hasGroup( bug.number() ) ) {
        return BugDetails();
    }

    KConfigGroup grp = m_cacheBugs->group(bug.number());

    BugDetailsPart::List parts;

    QStringList texts = grp.readEntry( "Details" , QStringList() );
    QStringList senders = grp.readEntry( "Senders" , QStringList() );
    QStringList dates = grp.readEntry( "Dates" , QStringList() );

    QStringList::ConstIterator itTexts = texts.constBegin();
    QStringList::ConstIterator itSenders = senders.constBegin();
    QStringList::ConstIterator itDates = dates.constBegin();
    while( itTexts != texts.constEnd() ) {
        QDateTime date = QDateTime::fromString( *itDates, Qt::ISODate );
        parts.append( BugDetailsPart( Person(*itSenders), date, *itTexts ) );

        ++itTexts;
        ++itSenders;
        ++itDates;
    }

    if ( parts.count() == 0 ) {
        return BugDetails();
    }

    QString version = grp.readEntry( "Version" );
    QString source = grp.readEntry( "Source" );
    QString compiler = grp.readEntry( "Compiler" );
    QString os = grp.readEntry( "OS" );

    return BugDetails( new BugDetailsImpl( version, source, compiler, os,
                                           parts ) );
}

void BugCache::invalidateBugDetails( const Bug& bug )
{
    m_cacheBugs->deleteGroup( bug.number() );
}

void BugCache::clear()
{
    delete m_cachePackages;
    delete m_cacheBugs;

    QFile f1( mCachePackagesFileName );
    f1.remove();

    QFile f2( mCacheBugsFileName );
    f2.remove();

    init();    
}
void BugCache::writePerson( KConfigGroup *grp, const QString &key,
                            const Person &p )
{
    QStringList values;
    values.append(p.name);
    values.append(p.email);
    grp->writeEntry( key, values );
}

struct Person BugCache::readPerson( KConfigGroup *grp, const QString &key )
{
    struct Person p;
    QStringList values = grp->readEntry(key, QStringList() );
    if ( values.count() > 0 )
        p.name = values[0];
    if ( values.count() > 1 )
        p.email = values[1];
    return p;
}
