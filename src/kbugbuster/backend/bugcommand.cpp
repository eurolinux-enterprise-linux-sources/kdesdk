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
#include "bugcommand.h"

#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocale.h>

QString BugCommand::name() const
{
    return i18n("Unknown");
}

QString BugCommand::details() const
{
    return QString();
}

BugCommand *BugCommand::load( KConfigGroup *grp, const QString &type )
{
    QString bugNumber = grp->name();
    // ### this sucks. we better let Bug implement proper persistance,
    // because this way of instantiating a bug object doesn't bring back
    // properties like title, package, etc. (Simon)
    Bug bug = Bug::fromNumber( bugNumber );
    Package pkg = Package(); // hack

    if ( type == "Close" ) {
        return new BugCommandClose( bug, grp->readEntry( type,QString() ), pkg );
    } else if ( type == "Reopen" ) {
        return new BugCommandReopen( bug, pkg );
    } else if ( type == "Merge" ) {
        return new BugCommandMerge( grp->readEntry( type , QStringList() ), pkg );
    } else if ( type == "Unmerge" ) {
        return new BugCommandUnmerge( bug, pkg );
    } else if ( type == "Reassign" ) {
        return new BugCommandReassign( bug, grp->readEntry( type,QString() ), pkg );
    } else if ( type == "Retitle" ) {
        return new BugCommandRetitle( bug, grp->readEntry( type, QString() ), pkg );
    } else if ( type == "Severity" ) {
        return new BugCommandSeverity( bug, grp->readEntry( type,QString() ), pkg );
    } else if ( type == "Reply" ) {
        return new BugCommandReply( bug, grp->readEntry( type,QString() ), grp->readEntry("Recipient",int(Normal)) );
    } else if ( type == "ReplyPrivate" ) {
        QStringList args = grp->readEntry( type , QStringList() );
        if ( args.count() != 2 ) return 0;
        return new BugCommandReplyPrivate( bug, (args.at(0)), (args.at(1)) );
    } else {
      kDebug() << "Warning! Unknown bug command '" << type << "'";
      return 0;
    }
}

///////////////////// Close /////////////////////

QString BugCommandClose::controlString() const
{
    if (m_message.isEmpty()) {
        return "close " + m_bug.number();
    } else {
        return QString();
    }
}

QString BugCommandClose::mailAddress() const
{
    kDebug() << "BugCommandClose::mailAddress(): number: " << m_bug.number();

    if (m_message.isEmpty()) {
        return QString();
    } else {
        return m_bug.number() + "-done@bugs.kde.org";
    }
}

QString BugCommandClose::mailText() const
{
    if (m_message.isEmpty()) {
        return QString();
    } else {
        return m_message;
    }
}

QString BugCommandClose::name() const
{
    return i18n("Close");
}

QString BugCommandClose::details() const
{
    return m_message;
}

void BugCommandClose::save( KConfigGroup *grp )
{
    grp->writeEntry( "Close",m_message );
}

///////////////////// Close Silently /////////////////////

QString BugCommandCloseSilently::controlString() const
{
    return "done " + m_bug.number();
}

QString BugCommandCloseSilently::name() const
{
    return i18n("Close Silently");
}

void BugCommandCloseSilently::save( KConfigGroup *grp )
{
    grp->writeEntry( "CloseSilently", true );
}

///////////////////// Reopen /////////////////////

QString BugCommandReopen::controlString() const
{
    return "reopen " + m_bug.number();
}

QString BugCommandReopen::name() const
{
    return i18n("Reopen");
}

void BugCommandReopen::save( KConfigGroup *grp )
{
    grp->writeEntry( "Reopen", true );
}

///////////////////// Retitle /////////////////////

QString BugCommandRetitle::controlString() const
{
    return "retitle " + m_bug.number() + ' ' + m_title;
}

QString BugCommandRetitle::name() const
{
    return i18n("Retitle");
}

QString BugCommandRetitle::details() const
{
    return m_title;
}

void BugCommandRetitle::save( KConfigGroup *grp )
{
    grp->writeEntry( "Retitle", m_title );
}

///////////////////// Merge /////////////////////

QString BugCommandMerge::controlString() const
{
    return "merge " + m_bugNumbers.join(" ");
}

QString BugCommandMerge::name() const
{
    return i18n("Merge");
}

QString BugCommandMerge::details() const
{
    return m_bugNumbers.join(", ");
}

void BugCommandMerge::save( KConfigGroup *grp )
{
    grp->writeEntry( "Merge", m_bugNumbers );
}

///////////////////// Unmerge /////////////////////

QString BugCommandUnmerge::controlString() const
{
    return "unmerge " + m_bug.number();
}

QString BugCommandUnmerge::name() const
{
    return i18n("Unmerge");
}

void BugCommandUnmerge::save( KConfigGroup *grp )
{
    grp->writeEntry( "Unmerge", true );
}

///////////////////// Reply /////////////////////

QString BugCommandReply::mailAddress() const
{
    return m_bug.number() + "@bugs.kde.org";
#if 0
    switch ( m_recipient ) {
      case Normal:
        return m_bug.number() + "@bugs.kde.org";
      case Maintonly:
        return m_bug.number() + "-maintonly@bugs.kde.org";
      case Quiet:
        return m_bug.number() + "-quiet@bugs.kde.org";
    }
    return QString();
#endif
}

QString BugCommandReply::mailText() const
{
    return m_message;
}

QString BugCommandReply::name() const
{
    return i18n("Reply");
#if 0
    switch ( m_recipient ) {
      case Normal:
        return i18n("Reply");
      case Maintonly:
        return i18n("Reply (Maintonly)");
      case Quiet:
        return i18n("Reply (Quiet)");
    }
    return QString();
#endif
}

QString BugCommandReply::details() const
{
    return m_message;
}

void BugCommandReply::save( KConfigGroup *grp )
{
    grp->writeEntry( "Reply", m_message );
#if 0
    grp->writeEntry( "Recipient", m_recipient );
#endif
}

///////////////////// Reply Private /////////////////////

QString BugCommandReplyPrivate::mailAddress() const
{
    return m_address;
}

QString BugCommandReplyPrivate::mailText() const
{
    return m_message;
}

QString BugCommandReplyPrivate::name() const
{
    return i18n("Private Reply");
}

QString BugCommandReplyPrivate::details() const
{
    return m_message;
}

void BugCommandReplyPrivate::save( KConfigGroup *grp )
{
    QStringList args;
    args << m_address;
    args << m_message;
    grp->writeEntry( "ReplyPrivate", args );
}

///////////////////// Severity /////////////////////

QString BugCommandSeverity::controlString() const
{
    return "severity " + m_bug.number() + ' ' + m_severity.toLower();
}

QString BugCommandSeverity::name() const
{
    return i18n("Severity");
}

QString BugCommandSeverity::details() const
{
    return m_severity;
}

void BugCommandSeverity::save( KConfigGroup *grp )
{
    grp->writeEntry( "Severity", m_severity );
}

///////////////////// Reassign /////////////////////

QString BugCommandReassign::controlString() const
{
    return "reassign " + m_bug.number() + ' ' + m_package;
}

QString BugCommandReassign::name() const
{
    return i18n("Reassign");
}

QString BugCommandReassign::details() const
{
    return m_package;
}

void BugCommandReassign::save( KConfigGroup *grp )
{
    grp->writeEntry( "Reassign", m_package );
}
