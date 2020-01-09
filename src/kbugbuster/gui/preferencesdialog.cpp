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
#include "preferencesdialog.h"

#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLayout>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QGroupBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <khbox.h>
#include <knuminput.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kicon.h>

#include "mailsender.h"
#include "kbbprefs.h"
#include "kbbmainwindow.h"
#include "serverconfigdialog.h"
#include "bugsystem.h"
#include "bugserver.h"
#include "bugserverconfig.h"

class ServerItem : public QTreeWidgetItem
{
  public:
    ServerItem( QTreeWidget *listView, const BugServerConfig &cfg )
      : QTreeWidgetItem( listView )
    {
      setServerConfig( cfg );
    }

    void setServerConfig( const BugServerConfig &cfg )
    {
      mServerConfig = cfg;
      setText( 0, cfg.name() );
      setText( 1, cfg.baseUrl().prettyUrl() );
      setText( 2, cfg.user() );
      setText( 3, cfg.bugzillaVersion() );
    }

    const BugServerConfig &serverConfig() const { return mServerConfig; }

  private:
    BugServerConfig mServerConfig;
};

class ServerListView : public QTreeWidget
{
  public:
    ServerListView( QWidget *parent ) : QTreeWidget( parent )
    {
      QTreeWidgetItem *header = new QTreeWidgetItem;
      header->setText( 0, i18n("Name") );
      header->setText( 1, i18n("Base URL") );
      header->setText( 2, i18n("User") );
      header->setText( 3, i18n("Version") );
      setHeaderItem( header );
    }
};

PreferencesDialog::PreferencesDialog( QWidget* parent,  const char* name )
    : KPageDialog ( parent )
{
    setCaption( i18n("Preferences") );
    setButtons( Ok|Apply|Cancel );
    setDefaultButton( Ok );
    setModal( false );
    showButtonSeparator( true );
    setFaceType( KPageDialog::List );
    setupServerPage();
    setupAdvancedPage();

    readConfig();
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(cancelClicked()),this,SLOT(slotCancel()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

PreferencesDialog::~PreferencesDialog()
{
}

void PreferencesDialog::setupServerPage()
{
    QFrame *topFrame = new QFrame();
    KPageWidgetItem *page = new KPageWidgetItem( topFrame, i18n("Servers"));
    page->setIcon( KIcon(DesktopIcon( "go-home", KIconLoader::SizeMedium )) );
    addPage( page );

  QBoxLayout *layout = new QVBoxLayout( topFrame );
  layout->setSpacing( spacingHint() );

  mServerList = new ServerListView( topFrame );
  layout->addWidget( mServerList );

  KHBox *buttonBox = new KHBox( topFrame );
  buttonBox->setSpacing( spacingHint() );
  layout->addWidget( buttonBox );

  QPushButton *addButton = new QPushButton( i18n("Add Server..."), buttonBox );
  connect( addButton, SIGNAL( clicked() ), SLOT( addServer() ) );

  QPushButton *editButton = new QPushButton( i18n("Edit Server..."), buttonBox );
  connect( editButton, SIGNAL( clicked() ), SLOT( editServer() ) );

  QPushButton *removeButton = new QPushButton( i18n("Delete Server"), buttonBox );
  connect( removeButton, SIGNAL( clicked() ), SLOT( removeServer() ) );

  QPushButton *button = new QPushButton( i18n("Select Server From List..."),
                                         topFrame );
  layout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( selectServer() ) );
  connect( mServerList, SIGNAL( itemDoubleClicked  ( QTreeWidgetItem *, int)), this, SLOT( editServer()));
}

void PreferencesDialog::setupAdvancedPage()
{
    QFrame *topFrame = new QFrame();
    KPageWidgetItem *page = new KPageWidgetItem( topFrame, i18n("Advanced"));
    page->setIcon( KIcon(DesktopIcon( "preferences-other", KIconLoader::SizeMedium )));
    addPage( page );

  QBoxLayout *layout = new QVBoxLayout( topFrame );
  layout->setSpacing( spacingHint() );

  QGroupBox *mailGroup = new QGroupBox( i18n( "Mail Client" ));
  QVBoxLayout *layoutH = new QVBoxLayout;
  layout->addWidget( mailGroup );
  mailGroup->setLayout(layoutH);

  mKMailButton = new QRadioButton( i18n( "&KMail" ) );
  mDirectButton = new QRadioButton( i18n( "D&irect" ) );
  mSendmailButton = new QRadioButton( i18n( "&Sendmail" ));
  layoutH->addWidget(mKMailButton);
  layoutH->addWidget(mDirectButton);
  layoutH->addWidget(mSendmailButton);

  mShowClosedCheckBox = new QCheckBox( i18n( "Show closed bugs" ), topFrame );
  layout->addWidget( mShowClosedCheckBox );

  mShowWishesCheckBox = new QCheckBox( i18n( "Show wishes" ), topFrame );
  layout->addWidget( mShowWishesCheckBox );

  mShowVotedCheckBox = new QCheckBox(  i18n( "Show bugs with number of votes greater than:" ), topFrame );
  layout->addWidget( mShowVotedCheckBox );

  mMinVotesInput = new KIntNumInput( topFrame );
  mMinVotesInput->setMinimum( 0 );
  connect( mShowVotedCheckBox, SIGNAL(toggled(bool)),
           mMinVotesInput, SLOT(setEnabled(bool)) );
  layout->addWidget( mMinVotesInput );

  mSendBccCheckBox = new QCheckBox( i18n( "Send BCC to myself" ), topFrame );
  layout->addWidget( mSendBccCheckBox );
}

void PreferencesDialog::setDefaults()
{
    KBBPrefs::instance()->setDefaults();
    readConfig();
}

void PreferencesDialog::slotApply()
{
  writeConfig();
}

void PreferencesDialog::slotOk()
{
    writeConfig();
    accept();
}

void PreferencesDialog::slotCancel()
{
    hide();
}

void PreferencesDialog::addServer()
{
  ServerConfigDialog *dlg = new ServerConfigDialog( this );
  int result = dlg->exec();
  if ( result == QDialog::Accepted ) {
    new ServerItem( mServerList, dlg->serverConfig() );
  }
}

void PreferencesDialog::editServer()
{
  ServerItem *item = static_cast<ServerItem *>( mServerList->currentItem() );
  if ( !item ) return;

  ServerConfigDialog *dlg = new ServerConfigDialog( this );
  dlg->setServerConfig( item->serverConfig() );

  int result = dlg->exec();
  if ( result == QDialog::Accepted ) {
    item->setServerConfig( dlg->serverConfig() );
  }
}

void PreferencesDialog::removeServer()
{
  QTreeWidgetItem *item = mServerList->currentItem();
  if ( !item ) return;

  delete item;
}

void PreferencesDialog::selectServer()
{
  SelectServerDlg *dlg =new SelectServerDlg( this );

  int result = dlg->exec();
  if ( result == QDialog::Accepted ) {
      ServerItem *item = dlg->serverSelected();
    if ( item ) {
      new ServerItem( mServerList, item->serverConfig() );
    }
  }
  delete dlg;
}

void PreferencesDialog::createServerItem( ServerListView *listView,
                                          const QString &name,
                                          const QString &url,
                                          const QString &version )
{
  BugServerConfig cfg( name, KUrl( url ) );
  cfg.setBugzillaVersion( version );
  new ServerItem( listView, cfg );
}

void PreferencesDialog::readConfig()
{
    int client = KBBPrefs::instance()->mMailClient;
    switch(client) {
      default:
      case MailSender::KMail:
        mKMailButton->setChecked(true);
        break;
      case MailSender::Sendmail:
        mSendmailButton->setChecked(true);
        break;
      case MailSender::Direct:
        mDirectButton->setChecked(true);
        break;
    }
    mShowClosedCheckBox->setChecked( KBBPrefs::instance()->mShowClosedBugs );
    mShowWishesCheckBox->setChecked( KBBPrefs::instance()->mShowWishes );
    mShowVotedCheckBox->setChecked( KBBPrefs::instance()->mShowVoted );
    mMinVotesInput->setValue( KBBPrefs::instance()->mMinVotes );
    mSendBccCheckBox->setChecked( KBBPrefs::instance()->mSendBCC );

    mServerList->clear();
    QList<BugServer *> servers = BugSystem::self()->serverList();
    QList<BugServer *>::ConstIterator it;
    for( it = servers.constBegin(); it != servers.constEnd(); ++it ) {
        new ServerItem( mServerList, (*it)->serverConfig() );
    }
}

void PreferencesDialog::writeConfig()
{
    MailSender::MailClient client = MailSender::KMail;

    if (mKMailButton->isChecked()) client = MailSender::KMail;
    if (mSendmailButton->isChecked()) client = MailSender::Sendmail;
    if (mDirectButton->isChecked()) client = MailSender::Direct;

    KBBPrefs::instance()->mMailClient = client;
    KBBPrefs::instance()->mShowClosedBugs = mShowClosedCheckBox->isChecked();
    KBBPrefs::instance()->mShowWishes = mShowWishesCheckBox->isChecked();
    KBBPrefs::instance()->mShowVoted = mShowVotedCheckBox->isChecked();
    KBBPrefs::instance()->mMinVotes = mMinVotesInput->value();
    KBBPrefs::instance()->mSendBCC = mSendBccCheckBox->isChecked();
    KBBPrefs::instance()->writeConfig();

    QList<BugServerConfig> servers;
    const int iCnt = mServerList->topLevelItemCount();
    for ( int i = 0; i < iCnt; ++i ) {
        ServerItem *item = static_cast<ServerItem *>(mServerList->topLevelItem( i ) );
        servers.append( item->serverConfig() );
    }

    BugSystem::self()->setServerList( servers );

    emit configChanged();
}

SelectServerDlg::SelectServerDlg(PreferencesDialog *parent )
    :KDialog(parent)
{
    setCaption( i18n("Select Server") );
    setButtons( KDialog::Ok | KDialog::Cancel);
    setDefaultButton( KDialog::Ok );
    setModal( true );
  list = new ServerListView(this );
  setMainWidget( list );

  parent->createServerItem( list, "KDE",       "http://bugs.kde.org",                       "KDE" );
  parent->createServerItem( list, "GNOME",     "http://bugzilla.gnome.org",                 "2.10" );
  parent->createServerItem( list, "Mozilla",   "http://bugzilla.mozilla.org",               "2.17.1" );
  parent->createServerItem( list, "Apache",    "http://nagoya.apache.org/bugzilla/",        "2.14.2" );
  parent->createServerItem( list, "XFree86",   "http://bugs.xfree86.org/cgi-bin/bugzilla/", "2.14.2" );
  parent->createServerItem( list, "Ximian",    "http://bugzilla.ximian.com",                "2.10" );
  parent->createServerItem( list, "RedHat",    "http://bugzilla.redhat.com/bugzilla/",      "2.17.1" );
  parent->createServerItem( list, "Mandriva",  "http://qa.mandriva.com/",                   "3.0.4" );
  connect( list, SIGNAL( itemDoubleClicked ( QTreeWidgetItem *, int)), this, SLOT( slotDoubleClicked( QTreeWidgetItem *, int)));
}


ServerItem *SelectServerDlg::serverSelected()
{
    return  static_cast<ServerItem *>( list->currentItem() );
}

void SelectServerDlg::slotDoubleClicked( QTreeWidgetItem *, int)
{
    accept();
}

#include "preferencesdialog.moc"
