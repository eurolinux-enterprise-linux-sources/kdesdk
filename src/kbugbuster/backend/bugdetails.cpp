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
#include "bugdetails.h"

#include <QLatin1String>
#include <QStringList>
#include <QRegExp>
#include <kdebug.h>
#include <kcodecs.h>
#include <kmessagebox.h>
#include "bugdetailsimpl.h"

BugDetails::BugDetails()
{
}

BugDetails::BugDetails( BugDetailsImpl *impl ) :
    m_impl( impl )
{
}

BugDetails::BugDetails( const BugDetails &other )
{
    (*this) = other;
}

BugDetails &BugDetails::operator=( const BugDetails &rhs )
{
    m_impl = rhs.m_impl;
    return *this;
}

BugDetails::~BugDetails()
{
}

QString BugDetails::version() const
{
    if ( !m_impl )
        return QString();

    return m_impl->version;
}

QString BugDetails::source() const
{
    if ( !m_impl )
        return QString();

    return m_impl->source;
}

QString BugDetails::compiler() const
{
    if ( !m_impl )
        return QString();

    return m_impl->compiler;
}

QString BugDetails::os() const
{
    if ( !m_impl )
        return QString();

    return m_impl->os;
}

QDateTime BugDetails::submissionDate() const
{
    if ( !m_impl ) return QDateTime();

    if ( m_impl->parts.count() > 0 ) {
        return m_impl->parts.last().date;
    }
    return QDateTime();
}

int BugDetails::age() const
{
   if ( !m_impl )
       return 0;

   return submissionDate().daysTo( QDateTime::currentDateTime() );
}

BugDetailsPart::List BugDetails::parts() const
{
    if ( !m_impl )
        return BugDetailsPart::List();

    return m_impl->parts;
}

QList<BugDetailsImpl::AttachmentDetails> BugDetails::attachmentDetails() const
{
    if ( m_impl )
        return m_impl->attachments;
    else
        return QList<BugDetailsImpl::AttachmentDetails>();
}

void BugDetails::addAttachmentDetails( const QList<BugDetailsImpl::AttachmentDetails>& attch )
{
    if ( m_impl )
        m_impl->attachments =  attch;
}

QList<BugDetails::Attachment> BugDetails::extractAttachments() const
{
    QList<BugDetails::Attachment> lst;
    if ( !m_impl )
        return lst;
    BugDetailsPart::List parts = m_impl->parts;
    for ( BugDetailsPart::List::ConstIterator it = parts.constBegin(); it != parts.constEnd(); ++it ) {
        lst += extractAttachments( (*it).text );
    }
    return lst;
}

//#define DEBUG_EXTRACT

QList<BugDetails::Attachment> BugDetails::extractAttachments( const QString& text )
{
    QList<BugDetails::Attachment> lst;
    const QStringList lines = text.split( '\n' );
#ifdef DEBUG_EXTRACT
    kDebug() << lines.count() << " lines.";
#endif
    QString boundary;
    for ( QStringList::const_iterator it = lines.constBegin() ; it != lines.constEnd() ; ++it )
    {
#ifdef DEBUG_EXTRACT
        kDebug() << "Line: " << *it;
#endif
        if ( (*it).startsWith( QLatin1String( " Content-Type" ) ) ) // ## leading space comes from khtml
        {
#ifdef DEBUG_EXTRACT
            //kDebug() << "BugDetails::extractAttachments going back, looking for empty or boundary=" << boundary;
#endif
            // Rewind until last empty line
            QStringList::const_iterator rit = it;
            for ( ; rit != lines.constBegin() ; --rit ) {
                QString line = *rit;
                if ( line.endsWith( QLatin1String( "<br />" ) ) )
                    line = line.left( line.length() - 6 );
                while ( !line.isEmpty() && line[0] == ' ' )
                    line.remove( 0, 1 );
#ifdef DEBUG_EXTRACT
                kDebug() << "Back: '" << line << "'";
#endif
                if ( line.isEmpty() ) {
                    ++rit; // boundary is next line
                    boundary = *rit;
                    if ( boundary.endsWith( QLatin1String( "<br />" ) ) )
                        boundary = boundary.left( boundary.length() - 6 );
                    if ( boundary[0] == ' ' )
                        boundary.remove( 0, 1 );
                    kDebug() << "BugDetails::extractAttachments boundary=" << boundary;
                    break;
                }
                if ( line == boundary )
                    break;
            }
            // Forward until next empty line (end of headers) - and parse filename
            QString filename;
            QString encoding;
            rit = it;
            for ( ; rit != lines.end() ; ++rit ) {
                QString header = *rit;
                if ( header.endsWith( QLatin1String( "<br />" ) ) )
                    header = header.left( header.length() - 6 );
                if ( header[0] == ' ' )
                    header.remove( 0, 1 );
#ifdef DEBUG_EXTRACT
                kDebug() << "Header: '" << header << "'";
#endif
                if ( header.isEmpty() )
                    break;
                if ( header.startsWith( QLatin1String( "Content-Disposition:" ) ) )
                {
#ifdef DEBUG_EXTRACT
                    kDebug() << "BugDetails::extractAttachments found header " << *rit;
#endif
                    // Taken from libkdenetwork/kmime_headers.cpp
                    int pos=header.indexOf("filename=", 0, Qt::CaseInsensitive);
                    QString fn;
                    if(pos>-1) {
                        pos+=9;
                        fn=header.mid(pos, header.length()-pos);
                        if ( fn.startsWith( '\"' ) && fn.endsWith( '\"' ) )
                            fn = fn.mid( 1, fn.length() - 2 );
                        //filename=decodeRFC2047String(fn, &e_ncCS, defaultCS(), forceCS());
                        filename = fn; // hack
                        kDebug() << "BugDetails::extractAttachments filename=" << filename;
                    }

                }
                else if ( header.startsWith( QLatin1String( "Content-Transfer-Encoding:" ) ) )
                {
                    encoding = header.mid( 26 ).toLower();
                    while ( encoding[0] == ' ' )
                        encoding.remove( 0, 1 );
                    kDebug() << "BugDetails::extractAttachments encoding=" << encoding;
                }
            } //for
            if ( rit == lines.end() )
                break;

            it = rit;
            if ( rit != lines.end() && !filename.isEmpty() )
            {
                ++it;
                if ( it == lines.end() )
                    break;
                // Read encoded contents
                QString contents;
                for ( ; it != lines.end() ; ++it )
                {
                    QString line = *it;
                    if ( line.endsWith( QLatin1String( "</tt>" ) ) )
                        line = line.left( line.length() - 5 );
                    if ( line.endsWith( QLatin1String( "<br />" ) ) )// necessary for the boundary check
                        line = line.left( line.length() - 6 );
                    while ( !line.isEmpty() && line[0] == ' ' )
                        line.remove( 0, 1 );
                    if ( line.isEmpty() || line == boundary ) // end of attachment
                        break;
                    if ( line == boundary+"--" ) // end of last attachment
                    {
                        boundary.clear();
                        break;
                    }
                    contents += line; // no newline, because of linebreaking between <br and />
                }
                contents = contents.remove( QRegExp("<br */>") );
#ifdef DEBUG_EXTRACT
                kDebug() << "BugDetails::extractAttachments contents=***\n" << contents << "\n***";
#endif
                kDebug() << "BugDetails::extractAttachments creating attachment " << filename;
                Attachment a;
                if ( encoding == "base64" )
                    KCodecs::base64Decode( contents.toLocal8Bit(), a.contents /*out*/ );
                else
                    //KCodecs::uudecode( contents.local8Bit(), a.contents /*out*/ );
                    KMessageBox::information( 0, i18n("Attachment %1 could not be decoded.\nEncoding: %2", filename, encoding) );
#ifdef DEBUG_EXTRACT
                kDebug() << "Result: ***\n" << QByteArray( a.contents.data(), a.contents.size()+1 ) << "\n*+*";
#endif
                a.filename = filename;
                lst.append(a);
                if ( it == lines.end() )
                    break;
            }
        }
    }
#ifdef DEBUG_EXTRACT
    kDebug() << "BugDetails::extractAttachments returning " << lst.count() << " attachments for this part.";
#endif
    return lst;
}

bool BugDetails::operator==( const BugDetails &rhs )
{
    return m_impl == rhs.m_impl;
}

/**
 * vim:ts=4:sw=4:et
 */
