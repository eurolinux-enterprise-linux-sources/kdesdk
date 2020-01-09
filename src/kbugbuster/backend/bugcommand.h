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
#ifndef BUGCOMMAND_H
#define BUGCOMMAND_H

#include <QString>
#include <QStringList>

#include "bug.h"
#include "package.h"

class KConfig;
class KConfigGroup;

class BugCommand
{
  public:
    enum Mode  { Normal, Maintonly, Quiet };
    enum State { None, Queued, Sent };

    BugCommand( const Bug &bug ) : m_bug( bug ) {}
    BugCommand( const Bug &bug, const Package &pkg ) : m_bug( bug ), m_package( pkg ) {}
    virtual ~BugCommand() {}

    virtual QString controlString() const { return QString(); }

    virtual QString mailAddress() const { return QString(); }
    virtual QString mailText() const { return QString(); }

    Bug bug() const { return m_bug; }
    Package package() const { return m_package; }

    virtual QString name() const;
    virtual QString details() const;

    virtual QString type() const { return QString(); }

    virtual void save( KConfigGroup * ) = 0;
    static BugCommand *load( KConfigGroup *, const QString &type );

  protected:
    Bug m_bug;
    Package m_package;
};

class BugCommandClose : public BugCommand
{
  public:
    BugCommandClose( const Bug &bug, const QString &message, const Package &pkg )  :
        BugCommand( bug, pkg ), m_message( message ) {}

    QString controlString() const;
    QString mailAddress() const;
    QString mailText() const;

    QString name() const;
    QString details() const;

    QString type() const { return QString::fromLatin1("Close"); }

    void save( KConfigGroup * );

  private:
    QString m_message;
};

class BugCommandCloseSilently : public BugCommand 
{
  public:
    BugCommandCloseSilently( const Bug &bug, const Package &pkg )  :
        BugCommand( bug, pkg ) {}

    QString controlString() const;

    QString name() const;

    QString type() const { return QString::fromLatin1("CloseSilently"); }

    void save( KConfigGroup * );
};

class BugCommandReopen : public BugCommand
{
  public:
    BugCommandReopen( const Bug &bug, const Package &pkg )  :
        BugCommand( bug, pkg ) {}

    QString controlString() const;

    QString name() const;

    QString type() const { return QString::fromLatin1("Reopen"); }

    void save( KConfigGroup * );
};

class BugCommandRetitle : public BugCommand 
{
  public:
    BugCommandRetitle( const Bug &bug, const QString &title, const Package &pkg )  :
        BugCommand( bug, pkg ), m_title( title ) {}

    QString controlString() const;

    QString name() const;
    QString details() const;

    QString type() const { return QString::fromLatin1("Retitle"); }

    void save( KConfigGroup * );

  private:
    QString m_title;
};

class BugCommandMerge : public BugCommand 
{
  public:
    BugCommandMerge( const QStringList &bugNumbers, const Package &pkg )  :
        BugCommand( Bug(), pkg ), m_bugNumbers( bugNumbers ) {}

    QString controlString() const;

    QString name() const;
    QString details() const;

    QString type() const { return QString::fromLatin1("Merge"); }

    void save( KConfigGroup * );

  private:
    QStringList m_bugNumbers;
};

class BugCommandUnmerge : public BugCommand 
{
  public:
    BugCommandUnmerge( const Bug &bug, const Package &pkg )  :
        BugCommand( bug, pkg ) {}

    QString name() const;

    QString type() const { return QString::fromLatin1("Unmerge"); }

    void save( KConfigGroup * );

    QString controlString() const;
};

class BugCommandReply : public BugCommand 
{
  public:
    BugCommandReply( const Bug &bug, const QString &message, const int &recipient) :
        BugCommand( bug ), m_message( message ), m_recipient( recipient ) {}

    QString mailAddress() const;
    QString mailText() const;

    QString name() const;
    QString details() const;

    QString type() const { return QString::fromLatin1("Reply"); }

    void save( KConfigGroup * );

  private:
    QString m_message;
    int m_recipient;
};

class BugCommandReplyPrivate : public BugCommand 
{
  public:
    BugCommandReplyPrivate( const Bug &bug, const QString &address,
                            const QString &message ) :
         BugCommand( bug ), m_address( address ), m_message( message ) {}

    QString mailAddress() const;
    QString mailText() const;

    QString name() const;
    QString details() const;

    QString type() const { return QString::fromLatin1("ReplyPrivate"); }

    void save( KConfigGroup * );

  private:
    QString m_address;
    QString m_message;
};

class BugCommandSeverity : public BugCommand 
{
  public:
    BugCommandSeverity( const Bug &bug, const QString &severity, const Package &pkg ) :
        BugCommand( bug, pkg ), m_severity( severity ) {}

    QString name() const;
    QString details() const;

    QString type() const { return QString::fromLatin1("Severity"); }

    QString controlString() const;

    void save( KConfigGroup * );

  private:
    QString m_severity;
};

class BugCommandReassign : public BugCommand
{
  public:
    BugCommandReassign( const Bug &bug, const QString &package, const Package &pkg ) :
        BugCommand( bug, pkg ), m_package( package ) {}

    QString name() const;
    QString details() const;

    QString type() const { return QString::fromLatin1("Reassign"); }

    QString controlString() const;

    void save( KConfigGroup * );

  private:
    QString m_package;
};

#endif
