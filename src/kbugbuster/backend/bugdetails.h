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
#ifndef BUGDETAILS_H
#define BUGDETAILS_H

#include "person.h"
#include "bugdetailspart.h"
#include "bugdetailsimpl.h"

#include <ksharedptr.h>

class BugDetailsImpl;

class BugDetails
{
public:
    typedef QList<BugDetails> List;
    struct Attachment {
        QByteArray contents;
        QString filename;
    };

    BugDetails();
    BugDetails( BugDetailsImpl *impl );
    BugDetails( const BugDetails &other );
    BugDetails &operator=( const BugDetails &rhs );
    ~BugDetails();

    QString version() const;
    QString source() const;
    QString compiler() const;
    QString os() const;
    BugDetailsPart::List parts() const;
    void addAttachmentDetails( const QList<BugDetailsImpl::AttachmentDetails>& attch );
    QList<BugDetailsImpl::AttachmentDetails> attachmentDetails() const;
    QList<BugDetails::Attachment> extractAttachments() const;
    static QList<BugDetails::Attachment> extractAttachments( const QString& text );

    QDateTime submissionDate() const;
    int age() const;

    bool operator==( const BugDetails &rhs );

    bool isNull() const { return m_impl.isNull(); }

private:
    BugDetailsImpl *impl() { return m_impl.data(); }

    KSharedPtr<BugDetailsImpl> m_impl;
};

#endif

/* vim: set sw=4 ts=4 et softtabstop=4: */

