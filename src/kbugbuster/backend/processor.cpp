/*
    This file is part of KBugBuster.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#include "processor.h"

#include <kdebug.h>
#include <kcodecs.h>

#include "bugserver.h"
#include "packageimpl.h"
#include "bugimpl.h"
#include "bugdetailsimpl.h"
#include "kbbprefs.h"

Processor::Processor( BugServer *server )
  : mServer( server )
{
}

Processor::~Processor()
{
}

void Processor::setPackageListQuery( KUrl &url )
{
  url.setFileName( "xml.cgi" );
  url.setQuery( "?data=versiontable" );
}

void Processor::setBugListQuery( KUrl &url, const Package &product, const QString &component )
{
  if ( mServer->serverConfig().bugzillaVersion() == "Bugworld" ) {
    url.setFileName( "bugworld.cgi" );
  } else {
    url.setFileName( "xmlquery.cgi" );
  }

  QString user = mServer->serverConfig().user();

  if ( component.isEmpty() )
      url.setQuery( "?user=" + user + "&product=" + product.name() );
  else
      url.setQuery( "?user=" + user + "&product=" + product.name() + "&component=" + component );
}

void Processor::setBugDetailsQuery( KUrl &url, const Bug &bug )
{
  url.setFileName( "xml.cgi" );
  url.setQuery( "?id=" + bug.number() );
}


/*
 * vim:sw=4:ts=4:et
 */
