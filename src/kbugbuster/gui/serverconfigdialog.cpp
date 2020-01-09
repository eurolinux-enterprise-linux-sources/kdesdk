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
#include "serverconfigdialog.h"

#include <QLayout>
#include <QLabel>
#include <QGridLayout>

#include <kcombobox.h>
#include <klineedit.h>
#include <kdebug.h>
#include <klocale.h>

#include "bugserverconfig.h"


ServerConfigDialog::ServerConfigDialog( QWidget *parent) :
  KDialog( parent)
{
    setCaption( i18n("Edit Bugzilla Server") );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );

  QWidget *topFrame = new QWidget( this );
  setMainWidget( topFrame );

  QGridLayout *topLayout = new QGridLayout( topFrame );
  topLayout->setSpacing( spacingHint() );

  QLabel *label;

  mServerName = new KLineEdit( topFrame );
  label = new QLabel( i18n("Name:"), topFrame );
  label->setBuddy( mServerName );
  topLayout->addWidget( label, 0, 0 );
  topLayout->addWidget( mServerName, 0, 1 );
  mServerName->setFocus();

  mServerUrl = new KLineEdit( topFrame );
  label = new QLabel( i18n("URL:"), topFrame );
  label->setBuddy( mServerUrl );
  topLayout->addWidget( label, 1, 0 );
  topLayout->addWidget( mServerUrl, 1, 1 );

  mUser = new KLineEdit( topFrame );
  label = new QLabel( i18n("User:"), topFrame );
  label->setBuddy( mUser );
  topLayout->addWidget( label, 2, 0 );
  topLayout->addWidget( mUser, 2, 1 );

  mPassword = new KLineEdit( topFrame );
  mPassword->setPasswordMode(true);
  label = new QLabel( i18n("Password:"), topFrame );
  label->setBuddy( mPassword );
  topLayout->addWidget( label, 3, 0 );
  topLayout->addWidget( mPassword, 3, 1 );

  mVersion = new KComboBox( topFrame );
  label = new QLabel( i18n("Bugzilla version:"), topFrame );
  label->setBuddy( mVersion );
  topLayout->addWidget( label, 4, 0 );
  topLayout->addWidget( mVersion, 4, 1 );
  mVersion->addItems( BugServerConfig::bugzillaVersions() );
}

void ServerConfigDialog::setServerConfig( const BugServerConfig &cfg )
{
  mServerName->setText( cfg.name() );
  mServerUrl->setText( cfg.baseUrl().url() );
  mUser->setText( cfg.user() );
  mPassword->setText( cfg.password() );

  for( int i = 0; i < mVersion->count(); ++i ) {
    if ( mVersion->itemText( i ) == cfg.bugzillaVersion() ) {
      mVersion->setCurrentIndex( i );
      break;
    }
  }
}

BugServerConfig ServerConfigDialog::serverConfig()
{
  BugServerConfig cfg;

  cfg.setName( mServerName->text() );
  cfg.setBaseUrl( KUrl( mServerUrl->text() ) );
  cfg.setUser( mUser->text() );
  cfg.setPassword( mPassword->text() );
  cfg.setBugzillaVersion( mVersion->currentText() );

  return cfg;
}

#include "serverconfigdialog.moc"
