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
#include "packageselectdialog.h"

#include <QTreeWidget>
#include <q3listview.h>
#include <QLayout>
#include <q3header.h>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QBoxLayout>
#include <QSplitter>

#include <kdebug.h>
#include <kcompletion.h>
#include <klineedit.h>

#include "bugsystem.h"
#include "kbbprefs.h"
#include "bugserver.h"

#include "packagelvi.h"
#include "packageselectdialog.moc"

PackageListView::PackageListView( QWidget *parent ) :
    Q3ListView( parent )
{
    setFocusPolicy( Qt::StrongFocus );
}

void PackageListView::resetTyped()
{
    mTyped = "";
}

void PackageListView::keyPressEvent( QKeyEvent *e )
{
    // Disable listview text completion for now
    Q3ListView::keyPressEvent( e );
    return;

    int k = e->key();
    if ( k == Qt::Key_Return || k == Qt::Key_Escape ) e->ignore();

    QString key = e->text();
    mTyped.append(key);
    emit typed( mTyped );
}

PackageSelectDialog::PackageSelectDialog(QWidget *parent) :
  KDialog( parent )
{
    setCaption( i18n("Select Product") );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );
    QWidget *topWidget = new QWidget( this );
    setMainWidget( topWidget );

    QBoxLayout *topLayout = new QVBoxLayout( topWidget );
    QSplitter *topSplitter = new QSplitter( Qt::Vertical, topWidget );
    topSplitter->setOpaqueResize( true );

    topLayout->addWidget( topSplitter );

    mRecentList = new QTreeWidget( topSplitter );
    QStringList lst;
    lst << i18n("Recent");
    mRecentList->setHeaderLabels(lst);
    mRecentList->resize( mRecentList->width(), mRecentList->fontMetrics().height() *
            KBBPrefs::instance()->mRecentPackagesCount );

    connect( mRecentList, SIGNAL(itemPressed ( QTreeWidgetItem *, int) ),
             SLOT( recentSelected(QTreeWidgetItem *, int) ) );
    connect( mRecentList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
             SLOT( slotOk() ) );

    mCompletion = new KCompletion;
    mCompletion->setCompletionMode( KGlobalSettings::CompletionAuto );

    mCompleteList = new PackageListView( topSplitter );
    mCompleteList->addColumn( i18n("Name") );
    mCompleteList->addColumn( i18n("Description") );
    mCompleteList->setRootIsDecorated(true);
    mCompleteList->setAllColumnsShowFocus( true );
    connect( mCompleteList, SIGNAL( typed( const QString & ) ),
             SLOT( completeTyped( const QString & ) ) );


    connect( mCompleteList, SIGNAL( mouseButtonPressed( int, Q3ListViewItem *, const QPoint &, int) ),
             SLOT( completeSelected( int, Q3ListViewItem * ) ) );
    connect( mCompleteList, SIGNAL( doubleClicked( Q3ListViewItem * ) ),
             SLOT( slotOk() ) );

    mPackageEdit = new KLineEdit( topWidget );
    mPackageEdit->setFocus();

    topLayout->addWidget( mPackageEdit );
    connect( mPackageEdit, SIGNAL( textChanged( const QString & ) ),
             SLOT( completeTyped( const QString & ) ) );
    enableButtonOk( !mPackageEdit->text().isEmpty() );
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
}

PackageSelectDialog::~PackageSelectDialog()
{
    delete mCompletion;
}

void PackageSelectDialog::setRecentPackages( const QStringList &recent )
{
    mRecentList->clear();
    QStringList::ConstIterator it;
    for( it = recent.begin(); it != recent.end(); ++it ) {
        new QTreeWidgetItem( mRecentList, QStringList()<<*it );
    }
}

void PackageSelectDialog::setPackages( const Package::List &pkgs )
{
    mCompleteList->clear();
    mCompletion->clear();
    mCompletionDict.clear();
    Package::List::ConstIterator it;
    for( it = pkgs.begin(); it != pkgs.end(); ++it ) {
        PackageLVI *item = new PackageLVI( mCompleteList, (*it), QString() );
        QStringList components = (*it).components();

        if (components.count() > 1) {
           for( QStringList::ConstIterator cit = components.constBegin(); cit != components.constEnd(); ++cit ) {
               PackageLVI *component = new PackageLVI( item, (*it), (*cit) );
               QString completionName = (*it).name() + '/' + (*cit);

               mCompletion->addItem( completionName );
               mCompletionDict.insert( completionName, component );
           }
        }

        mCompletion->addItem( (*it).name() );
        mCompletionDict.insert((*it).name(),item);
    }
}

void PackageSelectDialog::recentSelected(QTreeWidgetItem *item, int )
{
    kDebug() << "PackageSelectDialog::recentSelected()";
    if ( item ) {
        mCompleteList->clearSelection();
        // Why does a QLineEdit->setText() call emit the textChanged() signal?
        mPackageEdit->blockSignals( true );
        mPackageEdit->setText( item->text( 0 ) );
        enableButtonOk(true);
        mPackageEdit->blockSignals( false );
    }
}

void PackageSelectDialog::completeSelected( int, Q3ListViewItem *item )
{
    PackageLVI *lvi = dynamic_cast<PackageLVI*>(item);

    if ( lvi ) {
        mRecentList->clearSelection();
        if ( lvi->component().isEmpty() ) {
            mPackageEdit->setText( lvi->package().name() );
        }
        else {
            mPackageEdit->setText( lvi->package().name() + '/' + lvi->component() );
        }
    }
}

void PackageSelectDialog::slotOk()
{
    PackageLVI *item = (PackageLVI *)mCompleteList->selectedItem();
    if ( item ) {
        mSelectedPackage = item->package();
        mSelectedComponent = item->component();

        QString recent_key;
        if ( item->component().isEmpty() )
            recent_key = item->package().name();
        else
            recent_key = item->package().name() + '/' + item->component();

        BugServer *server = BugSystem::self()->server();
        QStringList recent = server->serverConfig().recentPackages();
        if( !recent.contains( recent_key ) ) {
            recent.prepend( recent_key );
            if ( int( recent.count() ) > KBBPrefs::instance()->mRecentPackagesCount ) {
                recent.removeAll( recent.last() );
            }
            kDebug() << "RECENT: " << recent.join(",");
            server->serverConfig().setRecentPackages( recent );
        }
    } else {
        QTreeWidgetItem *recentItem = mRecentList->currentItem();
        if ( recentItem ) {
            QStringList tokens = recentItem->text( 0 ).split( '/' );
            mSelectedPackage = BugSystem::self()->package( tokens[0] );
            if( tokens.count()>1)
               mSelectedComponent = tokens[1];
        }
    }
    mCompleteList->resetTyped();
    accept();
}

Package PackageSelectDialog::selectedPackage()
{
    return mSelectedPackage;
}

QString PackageSelectDialog::selectedComponent()
{
    return mSelectedComponent;
}

void PackageSelectDialog::completeTyped( const QString &typed )
{
    kDebug() << "completeTyped: " << typed;
    QString completed = mCompletion->makeCompletion( typed );
    kDebug() << "completed:     " << completed;
    if ( !completed.isEmpty() ) {
      mCompleteList->setSelected( mCompletionDict[ completed ], true );
      mCompleteList->ensureItemVisible( mCompletionDict[ completed ] );
    } else {
      mCompleteList->resetTyped();
    }
    enableButtonOk( !typed.isEmpty() );
}
