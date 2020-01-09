/***************************************************************************
                       kbbmainwindow.cpp  -  description
                             -------------------
    copyright            : (C) 2001 by Martijn Klingens
    email                : klingens@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kbbmainwindow.h"
#include <kselectaction.h>
#include <ktoggleaction.h>
#include <kactionmenu.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <QVBoxLayout>
#include <QBoxLayout>

#include <kaction.h>
#include <kactioncollection.h>
#include <kbookmarkmenu.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <kstandardaction.h>
#include <kstandardguiitem.h>
#include <kedittoolbar.h>
#include <ktextedit.h>
#include <QProgressBar>
#include <kicon.h>

#include "bugcommand.h"
#include "bugserver.h"
#include "bugserverconfig.h"
#include "bugsystem.h"
#include "centralwidget.h"
#include "cwbugdetails.h"
#include "kbbbookmarkmanager.h"
#include "kbbprefs.h"
#include "kfinddialog.h"
#include "packageselectdialog.h"
#include "preferencesdialog.h"

#define ID_STATUS_MSG 1

using namespace KBugBusterMainWindow;

class TextViewer : public KDialog
{
  public:
    TextViewer( const QString &title, QWidget *parent = 0 )
      : KDialog( parent)
    {
        setCaption( title );
        setButtons( Ok );
        setDefaultButton( Ok );
        setModal( false );
      QFrame *topFrame = new QFrame( this );
      setMainWidget( topFrame );

      QBoxLayout *topLayout = new QVBoxLayout( topFrame );

      mTextView = new KTextEdit( topFrame );
      mTextView->setReadOnly( true );
      topLayout->addWidget( mTextView );

      resize( 600, 400 );
    }

    void setText( const QString &text )
    {
      mTextView->setPlainText( text );
    }

  private:
    KTextEdit *mTextView;
};

KBookmarkManager* KBBBookmarkManager::s_bookmarkManager;

KBBMainWindow::KBBMainWindow( const QByteArray &initialPackage,
                              const QByteArray &initialComponent,
                              const QByteArray &initialBug,
                              QWidget * , const char * name )
  : KXmlGuiWindow( 0 ), mPreferencesDialog( 0 ), mResponseViewer( 0 ),
    mBugSourceViewer( 0 ), mPackageSelectDialog( 0 )
{
    setObjectName( name );
    BugSystem::self()->setCurrentServer( KBBPrefs::instance()->mCurrentServer );

    m_statusLabel = new QLabel( i18n( "Welcome to <b>KBugBuster</b>." ), statusBar() );
    m_statusLabel->setMaximumHeight( statusBar()->fontMetrics().height() + 6 );
    m_statusLabel->setIndent( KDialog::marginHint() / 2 );
    statusBar()->addWidget( m_statusLabel, 1 );

    m_mainWidget = new CentralWidget( initialPackage, initialComponent,
                                      initialBug, this );
    setCentralWidget( m_mainWidget );

    initActions();

    m_progressBar = new QProgressBar( statusBar() );
    m_progressBar->setMaximum(100);
    m_progressBar->setAlignment( Qt::AlignHCenter );
    m_progressBar->setMinimumWidth( 150 );
    m_progressBar->setMaximumHeight( statusBar()->fontMetrics().height() + 6 );
    statusBar()->addWidget( m_progressBar );
    connect( m_mainWidget, SIGNAL( resetProgressBar() ),
             m_progressBar, SLOT( reset() ) );
    connect( m_mainWidget, SIGNAL( searchPackage() ),
             this, SLOT( searchPackage() ) );
    connect( m_mainWidget, SIGNAL( searchBugNumber() ),
             this, SLOT( searchBugNumber() ) );

    connect( BugSystem::self(), SIGNAL( infoMessage( const QString & ) ),
             SLOT( slotStatusMsg( const QString & ) ) );

    connect( BugSystem::self(), SIGNAL( infoMessage( const QString & ) ),
             SLOT( slotStatusMsg( const QString & ) ) );
    connect( BugSystem::self(), SIGNAL( infoPercent( unsigned long ) ),
             SLOT( slotSetPercent( unsigned long ) ) );

    m_mainWidget->readConfig();
}

KBBMainWindow::~KBBMainWindow()
{
//    kDebug() << "KBBMainWindow::~KBBMainWindow()";
    delete m_pBookmarkMenu;

    m_mainWidget->writeConfig();

    KBBPrefs::instance()->writeConfig();
}

void KBBMainWindow::initActions()
{
    // Prepare and create XML GUI
    fileQuit          = KStandardAction::quit( this,
                        SLOT( close() ), actionCollection() );
    fileQuit->setToolTip( i18n( "Quit KBugBuster" ) );

    KAction *action = actionCollection()->addAction( "file_seechanges" );
    action->setIcon( KIcon("document-preview") );
    action->setText( i18n("See &Pending Changes") );
    connect(action, SIGNAL(triggered(bool)), SLOT( slotListChanges() ));
    action = actionCollection()->addAction( "file_submit" );
    action->setIcon( KIcon("mail-send") );
    action->setText( i18n("&Submit Changes") );
    connect(action, SIGNAL(triggered(bool)), SLOT( slotSubmit() ));

    reloadpacklist = actionCollection()->addAction( "reload_packagelist" );
    reloadpacklist->setIcon( KIcon("view-refresh") );
    reloadpacklist->setText( i18n("Reload &Product List") );
    connect(reloadpacklist, SIGNAL(triggered(bool)), m_mainWidget, SLOT( slotReloadPackageList() ));
    reloadpacklist->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_F5));
    reloadpack = actionCollection()->addAction( "reload_package" );
    reloadpack->setIcon( KIcon("view-refresh") );
    reloadpack->setText( i18n("Reload Bug &List (for current product)") );
    connect(reloadpack, SIGNAL(triggered(bool)), m_mainWidget, SLOT( slotReloadPackage() ));
    reloadpack->setShortcut(QKeySequence(Qt::Key_F5));
    reloadbug = actionCollection()->addAction( "reload_bug" );
    reloadbug->setIcon( KIcon("view-refresh") );
    reloadbug->setText( i18n("Reload Bug &Details (for current bug)") );
    connect(reloadbug, SIGNAL(triggered(bool)), m_mainWidget, SLOT( slotReloadBug() ));
    reloadbug->setShortcut(QKeySequence(Qt::SHIFT+Qt::Key_F5));
    loadMyBugs = actionCollection()->addAction( "load_my_bugs" );
    loadMyBugs->setText( i18n( "Load &My Bugs List" ) );
    connect(loadMyBugs, SIGNAL(triggered(bool) ), m_mainWidget, SLOT( slotLoadMyBugs() ));
    reloadall = actionCollection()->addAction( "load_allbugs" );
    reloadall->setText( i18n("Load All Bug Details (for current product)") );
    connect(reloadall, SIGNAL(triggered(bool) ), m_mainWidget, SLOT( slotRetrieveAllBugDetails() ));
    reloadall->setShortcut(QKeySequence(Qt::Key_F6));
    action = actionCollection()->addAction( "extract_attachments" );
    action->setIcon( KIcon("document-save") );
    action->setText( i18n("Extract &Attachments") );
    connect(action, SIGNAL(triggered(bool)), m_mainWidget, SLOT( slotExtractAttachments() ));
    action->setShortcut(QKeySequence(Qt::Key_F4));

    action = actionCollection()->addAction( "clear_cache" );
    action->setText( i18n("Clear Cache") );
    connect(action, SIGNAL(triggered(bool) ), SLOT( clearCache() ));

    action = actionCollection()->addAction( "search_package" );
    action->setIcon( KIcon("go-jump") );
    action->setText( i18n("&Search by Product...") );
    connect(action, SIGNAL(triggered(bool)), SLOT( searchPackage() ));
    action->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_P));
    action = actionCollection()->addAction( "search_bugnumber" );
    action->setIcon( KIcon("edit-find") );
    action->setText( i18n("Search by Bug &Number...") );
    connect(action, SIGNAL(triggered(bool)), SLOT( searchBugNumber() ));
    action->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_N));
    // For now "Description" searches by title. Maybe later we can have a
    // full-text search interfacing bugs.kde.org and rename the current one to "By Title".
    action = actionCollection()->addAction( "search_description" );
    action->setIcon( KIcon("edit-find") );
    action->setText( i18n("Search by &Description...") );
    connect(action, SIGNAL(triggered(bool)), SLOT( searchDescription() ));
    action->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_D));

//    new KAction( i18n("&Merge"), "view-close", CTRL+Qt::Key_M, m_mainWidget,
//                 SLOT( mergeBugs() ), actionCollection(), "cmd_merge" );
//    new KAction( i18n("&Unmerge"), "view-split-top-bottom", CTRL+SHIFT+Qt::Key_M, m_mainWidget,
//                 SLOT( unmergeBugs() ), actionCollection(), "cmd_unmerge" );
    action = actionCollection()->addAction( "cmd_close" );
    action->setIcon( KIcon("user-trash") );
    action->setText( i18n("C&lose...") );
    connect(action, SIGNAL(triggered(bool)), m_mainWidget, SLOT( closeBug() ));
    action->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_L));
//    new KAction( i18n("Clos&e Silently"), "user-trash", CTRL+Qt::Key_E, m_mainWidget,
//                 SLOT( closeBugSilently() ), actionCollection(), "cmd_close_silently" );
    action = actionCollection()->addAction( "cmd_reopen" );
    action->setIcon( KIcon("edit-redo") );
    action->setText( i18n("Re&open") );
    connect(action, SIGNAL(triggered(bool)), m_mainWidget, SLOT( reopenBug() ));
    action->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_O));
//    new KAction( i18n("Re&assign..."), "folder-new", CTRL+Qt::Key_A, m_mainWidget,
//                 SLOT( reassignBug() ), actionCollection(), "cmd_reassign" );
//    new KAction( i18n("Change &Title..."), "edit-rename", CTRL+Qt::Key_T, m_mainWidget,
//                 SLOT( titleBug() ), actionCollection(), "cmd_title" );
//    new KAction( i18n("Change &Severity..."), "document-properties", CTRL+Qt::Key_S, m_mainWidget,
//                 SLOT( severityBug() ), actionCollection(), "cmd_severity" );
    action = actionCollection()->addAction( "cmd_reply" );
    action->setIcon( KIcon("mail-reply-all") );
    action->setText( i18n("&Reply...") );
    connect(action, SIGNAL(triggered(bool)), m_mainWidget, SLOT( replyBug() ));
    action->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_R));
    action = actionCollection()->addAction( "cmd_replyprivate" );
    action->setIcon( KIcon("mail-reply-sender") );
    action->setText( i18n("Reply &Privately...") );
    connect(action, SIGNAL(triggered(bool)), m_mainWidget, SLOT( replyPrivateBug() ));
    action->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_I));

    KStandardAction::showMenubar(this, SLOT( slotToggleMenubar() ), actionCollection() );
    createStandardStatusBarAction();
    setStandardToolBarMenuEnabled(true);

    m_disconnectedAction = actionCollection()->add<KToggleAction>( "settings_disconnected" );
    m_disconnectedAction->setText( i18n("&Disconnected Mode") );
    connect(m_disconnectedAction, SIGNAL(triggered(bool)), SLOT( slotDisconnectedAction() ));
    m_disconnectedAction->setChecked( BugSystem::self()->disconnected() );

    // Bookmarks menu
    m_pamBookmarks = actionCollection()->add<KActionMenu>( "bookmarks" );
    m_pamBookmarks->setText( i18n( "&Bookmarks" ) );
    m_pBookmarkMenu = new KBookmarkMenu( KBBBookmarkManager::self(), this, m_pamBookmarks->menu(), actionCollection() );

    KStandardAction::preferences( this, SLOT(preferences()), actionCollection() );

    KToggleAction *toggleTmp = actionCollection()->add<KToggleAction>( "cmd_toggle_done" );
    toggleTmp->setIcon( KIcon("view-filter") );
    toggleTmp->setText( i18n("Show Closed Bugs") );
    connect(toggleTmp, SIGNAL(triggered(bool) ), SLOT( slotToggleDone() ));
    toggleTmp->setCheckedState(KGuiItem(i18n("Hide Closed Bugs")));
    toggleTmp->setChecked( KBBPrefs::instance()->mShowClosedBugs );

    toggleTmp = actionCollection()->add<KToggleAction>( "cmd_toggle_wishes" );
    toggleTmp->setIcon( KIcon("view-history") );
    toggleTmp->setText( i18n("Show Wishes") );
    connect(toggleTmp, SIGNAL(triggered(bool) ), SLOT( slotToggleWishes() ));
    toggleTmp->setCheckedState(KGuiItem(i18n("Hide Wishes")));
    toggleTmp->setChecked(KBBPrefs::instance()->mShowWishes);

    mSelectServerAction = actionCollection()->add<KSelectAction>( "select_server", this, SLOT( slotSelectServer() ) );
    mSelectServerAction->setText( i18n( "Select Server" ) );
    setupSelectServerAction();

    if ( KBBPrefs::instance()->mDebugMode ) {
      action = actionCollection()->addAction( "debug_lastresponse" );
      action->setText( i18n("Show Last Server Response...") );
      connect(action, SIGNAL(triggered(bool) ), SLOT( showLastResponse() ));
      action = actionCollection()->addAction( "debug_showbugsource" );
      action->setText( i18n("Show Bug HTML Source...") );
      connect(action, SIGNAL(triggered(bool) ), SLOT( showBugSource() ));
    }

    setupGUI( (ToolBar | Keys | StatusBar | Save | Create ), "kbugbusterui.rc" );
}

void KBBMainWindow::slotToggleMenubar()
{
    if ( !hasMenuBar() )
        return;

    if ( menuBar()->isVisible() )
        menuBar()->hide();
    else
        menuBar()->show();
}

void KBBMainWindow::setupSelectServerAction()
{
    QStringList servers;
    int current = -1;
    QList<BugServer *> serverList = BugSystem::self()->serverList();
    QList<BugServer *>::ConstIterator it;
    for ( it = serverList.constBegin(); it != serverList.constEnd(); ++it ) {
      QString name = (*it)->serverConfig().name();
      servers.append( name );
      if ( name == KBBPrefs::instance()->mCurrentServer ) {
        current = servers.count() - 1;
      }
    }
    mSelectServerAction->setItems( servers );
    if ( current >= 0 ) {
      mSelectServerAction->setCurrentItem( current );
    }
}

QString KBBMainWindow::currentUrl() const
{
  QString number=m_mainWidget->currentNumber();

  if (number.isEmpty())
    return "";
  else
    return "bug:"+number;
}

QString KBBMainWindow::currentTitle() const
{
  return '#'+m_mainWidget->currentNumber()+": "+m_mainWidget->currentTitle();
}

void KBBMainWindow::openBookmark(const KBookmark & bm, Qt::MouseButtons mb, Qt::KeyboardModifiers km)
{
    const QString & url = bm.url().url();
    if ( url.left(4)=="bug:" ) {
      QString bugnumber = url.mid(4);
      m_mainWidget->slotRetrieveBugDetails( Bug::fromNumber( bugnumber ) );
    }
}

// --- SLOT IMPLEMENTATIONS -------------------------------------------------

void KBBMainWindow::slotDisconnectedAction()
{
    BugSystem::self()->setDisconnected( m_disconnectedAction->isChecked() );

    bool enable = !m_disconnectedAction->isChecked();

    reloadpacklist->setEnabled( enable );
    reloadpacklist->setEnabled( enable );
    reloadpack->setEnabled( enable );
    reloadbug->setEnabled( enable );
    reloadall->setEnabled( enable );
    loadMyBugs->setEnabled( enable );
}

void KBBMainWindow::slotStatusMsg( const QString &text )
{
    // Change status message permanently
    m_statusLabel->setText( text );
}

void KBBMainWindow::slotSubmit()
{
    BugSystem::self()->sendCommands();
}

void KBBMainWindow::slotListChanges()
{
    QStringList list = BugSystem::self()->server()->listCommands();

    if (list.count() > 0)
    {
       int ret = KMessageBox::questionYesNoList( this, i18n("List of pending commands:"),
                                              list, QString(), KStandardGuiItem::clear(), KStandardGuiItem::close() );
       if ( ret == KMessageBox::Yes )
       {
           // Ask for confirmation, it's too easy to click the wrong button in the above dlg box
           if ( KMessageBox::warningContinueCancel( this, i18n("Do you really want to delete all commands?"),
                               i18n("Confirmation Required"), KGuiItem( i18n("&Delete"), "edit-delete"), KStandardGuiItem::cancel(), "clearcommands", KMessageBox::Notify)
                   == KMessageBox::Continue )
               BugSystem::self()->clearCommands();
       }
   }
   else
   {
       KMessageBox::information( this, i18n("There are no pending commands.") );
   }
}

void KBBMainWindow::slotSetPercent( unsigned long percent )
{
    // KIO::Job::percent() shouldn't return an unsigned long. - Frerich
    m_progressBar->setValue( percent );
}

void KBBMainWindow::searchPackage()
{
    if ( !mPackageSelectDialog ) {
        mPackageSelectDialog = new PackageSelectDialog( this );
    }
    mPackageSelectDialog->setPackages( BugSystem::self()->packageList() );
    BugServerConfig cfg = BugSystem::self()->server()->serverConfig();
    QStringList recent = cfg.recentPackages();
    kDebug() << "MainWindow RECENT: " << recent.join(",");
    mPackageSelectDialog->setRecentPackages( recent );

    mPackageSelectDialog->exec();
    Package package = mPackageSelectDialog->selectedPackage();

    if ( package.isNull() ) {
        return;
    }

    QString component = mPackageSelectDialog->selectedComponent();
    m_mainWidget->slotRetrieveBugList( package.name(), component );
}

void KBBMainWindow::searchBugNumber()
{
    bool ok = false;
    QString result = KInputDialog::getText( i18n("Search for Bug Number"),
                                            i18n("Please enter a bug number:"),
                                            QString(), &ok, this );
    if ( ok ) {
      //Strip whitespace and # if needed
      result = result.trimmed();
      if (result[0]=='#')
        result = result.mid(1);
    }

    if ( ok && !result.isEmpty()) {
        // ### bad way to instantiate a bug! doesn't get us the details!
        // Right - but do we need the details in this case ? There's no listview entry here... (DF)
        m_mainWidget->slotRetrieveBugDetails( Bug::fromNumber( result ) );
    }
}

void KBBMainWindow::searchDescription()
{
    kDebug() << "KBBMainWindow::searchDescription().";
    //KMessageBox::sorry(this,i18n("searchDescription(): to be implemented."),i18n("Not implemented"));
    KFindDialog dlg( this );
    if ( dlg.exec() == KDialog::Accepted )
        m_mainWidget->searchBugByTitle( dlg.options(), dlg.pattern() );
}

bool KBBMainWindow::queryClose()
{
    if ( ! BugSystem::self()->server()->commandsPending() ) return true;

    int result = KMessageBox::warningYesNoCancel(this,i18n("There are unsent bug commands."
                                                           " Do you want to send them now?"), QString(), KGuiItem(i18n("Send")), KGuiItem(i18n("Do Not Send")));
    if ( result == KMessageBox::Cancel ) return false;
    if ( result == KMessageBox::Yes ) {
        BugSystem::self()->sendCommands();
    }
    return true;
}

void KBBMainWindow::preferences()
{
    if (!mPreferencesDialog) {
      mPreferencesDialog  = new PreferencesDialog(this);
      connect( mPreferencesDialog, SIGNAL( configChanged() ),
               SLOT( setupSelectServerAction() ) );
      connect( mPreferencesDialog, SIGNAL( configChanged() ),
               m_mainWidget, SLOT( slotReloadPackage() ) );
    }
    mPreferencesDialog->show();
    mPreferencesDialog->raise();
}

void KBBMainWindow::updatePackage()
{
    m_mainWidget->updatePackage();
}

void KBBMainWindow::slotToggleDone()
{
  KBBPrefs::instance()->mShowClosedBugs=!(KBBPrefs::instance()->mShowClosedBugs);
  updatePackage();
}

void KBBMainWindow::slotToggleWishes()
{
  KBBPrefs::instance()->mShowWishes=!(KBBPrefs::instance()->mShowWishes);
  updatePackage();
}

void KBBMainWindow::slotSelectServer()
{
  m_mainWidget->writeConfig();

  QString currentServer = mSelectServerAction->currentText();

  BugSystem::self()->setCurrentServer( currentServer );

  m_mainWidget->initialize();
}

void KBBMainWindow::showLastResponse()
{
  if ( !mResponseViewer ) {
    mResponseViewer = new TextViewer( i18n("Last Server Response"), this );
  }

  mResponseViewer->setText( BugSystem::lastResponse() );

  mResponseViewer->show();
  mResponseViewer->raise();
}

void KBBMainWindow::showBugSource()
{
  if ( !mBugSourceViewer ) {
    mBugSourceViewer = new TextViewer( i18n("Bug HTML Source"), this );
  }

  mBugSourceViewer->setText( m_mainWidget->bugDetailsWidget()->source() );

  mBugSourceViewer->show();
  mBugSourceViewer->raise();
}

void KBBMainWindow::clearCache()
{
  BugSystem::self()->server()->cache()->clear();
}

#include "kbbmainwindow.moc"

/* vim: set et ts=4 sw=4 softtabstop=4: */

