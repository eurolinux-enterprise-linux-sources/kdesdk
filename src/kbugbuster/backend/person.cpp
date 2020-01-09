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
#include "person.h"

#include <kdebug.h>

Person::Person( const QString &fullName )
{
    int emailPos = fullName.indexOf( '<' );
    if ( emailPos < 0 ) {
        email = fullName;
    } else {
        email = fullName.mid( emailPos + 1, fullName.length() - 1 );
        name = fullName.left( emailPos - 1 );
    }
}

QString Person::fullName(bool html) const
{
    if( name.isEmpty() )
    {
        if( email.isEmpty() )
            return i18n( "Unknown" );
        else
            return email;
    }
    else
    {
        if( email.isEmpty() )
            return name;
        else
            if ( html ) {
                return name + " &lt;" + email + "&gt;";
            } else {
                return name + " <" + email + '>';
            }
    }
}

Person Person::parseFromString( const QString &_str )
{
    Person res;

    QString str = _str;

    int ltPos = str.indexOf( '<' );
    if ( ltPos != -1 )
    {
        int gtPos = str.indexOf( '>', ltPos );
        if ( gtPos != -1 )
        {
            res.name = str.left( ltPos - 1 );
            str = str.mid( ltPos + 1, gtPos - ( ltPos + 1 ) );
        }
    }

    int atPos = str.indexOf( '@' );
    int spacedAtPos = str.indexOf( QString::fromLatin1( " at " ) );
    if ( atPos == -1 && spacedAtPos != -1 )
        str.replace( spacedAtPos, 4, QString::fromLatin1( "@" ) );

    int spacePos = str.indexOf( ' ' );
    while ( spacePos != -1 )
    {
        str[ spacePos ] = '.';
        spacePos = str.indexOf( ' ', spacePos );
    }

    res.email = str;

    return res;
}

/**
 * vim:et:ts=4:sw=4
 */
